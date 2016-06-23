
#ifndef __H_EW_SCRIPTING_PL_DISPATCH__
#define __H_EW_SCRIPTING_PL_DISPATCH__

#include "ewa_base/scripting/variant.h"
#include "ewa_base/scripting/variant_op.h"
#include "ewa_base/scripting/executor.h"


EW_ENTER

template<typename P,unsigned N,unsigned X>
class pl1_dispatch : public pl1_dispatch_base<P>{};

template<typename P,unsigned N>
class pl1_dispatch<P,N,1> : public pl1_dispatch_base<P>
{
public:

	template<typename T>
	static typename tl::enable_if<type_flag<T>::is_scr>::type g(Executor& ewsl,const T& v)
	{
		typedef typename P::template rebind<T>::type retx;
		retx y=P::g(v);
		ewsl.ci1.nsp[0].reset(y);
	}

	template<typename T>
	static typename tl::enable_if<type_flag<T>::is_arr>::type g(Executor& ewsl,const T& v)
	{
		typedef typename type_flag<T>::scalar type;
		typedef typename P::template rebind<type>::type retx;

		type_flag<T>::size_chk(v);

		arr_xt<retx> y;
		y.resize(type_flag<T>::size_ptr(v));
		size_t n=y.size();

		for(size_t i=0;i<n;i++)
		{
			y(i)=P::g(v(i));
		}

		ewsl.ci1.nsp[0].reset(y);
	}

	static int value(Executor& ewsl,Variant& v)
	{
		typedef typename flag_type<N>::type type;
		g(ewsl,variant_handler<type>::raw(v));
		return 1;
	}
};


template<typename P,unsigned N>
class pl1_dispatch<P,N,2> : public pl1_dispatch_base<P>
{
public:

	template<typename T>
	static void g(Executor& ewsl,const T& v)
	{
		typedef typename P::template rebind<T>::type retx;
		retx y=P::g(v);
		ewsl.ci1.nsp[0].reset(y);
	}

	static int value(Executor& ewsl,Variant& v)
	{
		typedef typename flag_type<N>::type type;
		g(ewsl,variant_handler<type>::g(v));
		return 1;
	}
};

template<typename P>
class pl1_call
{
public:
	template<unsigned N> class fk : public pl1_dispatch<P,N,flag_info1<P,N>::value>{};
	typedef int (*fn)(Executor&,Variant&);
	typedef lookup_table_4bit<fk,fn> lk;

	static int g(Executor& ewsl)
	{
		return lk::test(ewsl.ci1.nsp[0].type())(ewsl,ewsl.ci1.nsp[0]);
	}
};



template<typename P,unsigned N>
class flag_info2
{
public:

	static const unsigned N1=N>>4;
	static const unsigned N2=N&0xF;

	typedef typename flag_type<N1>::scalar scalar1;
	typedef typename flag_type<N1>::type type1;
	typedef typename flag_type<N2>::scalar scalar2;
	typedef typename flag_type<N2>::type type2;


	static const bool num_enable=P::template rebind<scalar1,scalar2>::value;
	static const bool arr_enable=P::template rebind<type1,type2>::value;

	static const int value=(P::dispatch_type==pl_base2::DISPATCH_NUM)?(num_enable?1:0):(arr_enable?2:0);

};

template<typename P>
class pl2_dispatch_base
{
public:
	static int value(Executor& ewsl,Variant&,Variant&)
	{
		return CallableMetatable::__metatable_call2(ewsl,P::info().op_name);
	}
};


template<typename P,unsigned N,unsigned X>
class pl2_dispatch : public pl2_dispatch_base<P>
{
public:


};

template<typename P,unsigned N>
class pl2_dispatch<P,N,1>: public pl2_dispatch_base<P>
{
public:

	static const unsigned N1=N>>4;
	static const unsigned N2=N&0xF;

	template<typename T1,typename T2>
	static inline typename tl::enable_if<type_flag<T1>::is_scr&&type_flag<T2>::is_scr>::type g(Executor& ewsl,const T1& v1,const T2& v2)
	{
		typedef typename P::template rebind<T1,T2>::type retx;
		retx y=P::g(v1,v2);
		(*--ewsl.ci1.nsp).reset(y);
	}

	template<typename T1,typename T2>
	static typename tl::enable_if<type_flag<T1>::is_arr&&type_flag<T2>::is_scr>::type g(Executor& ewsl,const T1& v1,const T2& v2)
	{
		typedef typename P::template rebind<typename type_flag<T1>::scalar,T2>::type retx;

		arr_xt<retx> y;
		y.resize(type_flag<T1>::size_ptr(v1));

		size_t n=y.size();
		for(size_t i=0;i<n;i++)
		{
			y(i)=P::g(v1(i),v2);
		}

		(*--ewsl.ci1.nsp).reset(y);
	}
	template<typename T1,typename T2>
	static typename tl::enable_if<type_flag<T1>::is_scr&&type_flag<T2>::is_arr>::type g(Executor& ewsl,const T1& v1,const T2& v2)
	{

		typedef typename P::template rebind<T1,typename type_flag<T2>::scalar>::type retx;
		arr_xt<retx> y;
		y.resize(type_flag<T2>::size_ptr(v2));

		size_t n=y.size();
		for(size_t i=0;i<n;i++)
		{
			y(i)=P::g(v1,v2(i));
		}

		(*--ewsl.ci1.nsp).reset(y);
	}

	template<typename T1,typename T2>
	static typename tl::enable_if<type_flag<T1>::is_arr&&type_flag<T2>::is_arr>::type g(Executor& ewsl,const T1& v1,const T2& v2)
	{
		typedef typename P::template rebind<typename type_flag<T1>::scalar,typename type_flag<T2>::scalar>::type retx;

		arr_xt<retx> y;
		y.resize(type_flag<T1>::size_ptr(v1));

		size_t n=y.size();
		if(n!=type_flag<T2>::size_ptr(v2).size())
		{
			Exception::XError("array_size error");
		}

		for(size_t i=0;i<n;i++)
		{
			y(i)=P::g(v1(i),v2(i));
		}

		(*--ewsl.ci1.nsp).reset(y);
	}


	static inline int value(Executor& ewsl,Variant& v1,Variant& v2)
	{
		typedef typename flag_type<N1>::type type1;
		typedef typename flag_type<N2>::type type2;

		g(ewsl,variant_handler<type1>::raw(v1),variant_handler<type2>::raw(v2));

		return 1;
	}
};



template<typename P,unsigned N>
class pl2_dispatch<P,N,2>: public pl2_dispatch_base<P>
{
public:

	template<typename T>
	static inline void s(Variant& r,const T& v)
	{
		r.reset(v);
	}

	template<typename T>
	static inline void s(Variant& r,const arr_xt<T>& v)
	{
		if(v.size()==1)
		{
			r.reset(v[0]);
		}
		else
		{
			r.reset(v);
		}
	}

	template<typename T1,typename T2>
	static inline void g(Executor& ewsl,const T1& v1,const T2& v2)
	{
		typedef typename P::template rebind<T1,T2>::type retx;
		retx y=P::g(v1,v2);
		s((*--ewsl.ci1.nsp),y);
	}

	static inline int value(Executor& ewsl,Variant& v1,Variant& v2)
	{
		static const unsigned N1=N>>4;
		static const unsigned N2=N&0xF;

		typedef typename flag_type<N1>::type type1;
		typedef typename flag_type<N2>::type type2;

		g(ewsl,variant_handler<type1>::raw(v1),variant_handler<type2>::raw(v2));

		return 1;
	}
};


template<typename P>
class pl2_call
{
public:

	template<unsigned N> class fk : public pl2_dispatch<P,N,flag_info2<P,N>::value>{};

	typedef int (*fn)(Executor&,Variant&,Variant&);
	typedef lookup_table_8bit<fk,fn> lk;

	static EW_FORCEINLINE int g(Executor& ewsl)
	{
		Variant& v1(ewsl.ci1.nsp[-1]);
		Variant& v2(ewsl.ci1.nsp[-0]);
		unsigned n=(v1.type()<<4)|v2.type();
		return lk::test(n)(ewsl,v1,v2);
	}

	static EW_FORCEINLINE int k(Executor& ewsl)
	{
		typedef int64_t fast_type;
		static const unsigned fast_flag=(type_flag<fast_type>::value<<4)|type_flag<fast_type>::value;

		Variant& v1(ewsl.ci1.nsp[-1]);
		Variant& v2(ewsl.ci1.nsp[-0]);
		unsigned n=(v1.type()<<4)|v2.type();
		if(n==fast_flag)
		{
			(*--ewsl.ci1.nsp).reset(P::g(variant_handler<fast_type>::raw(v1),variant_handler<fast_type>::raw(v2)));
			return 1;
		}
		return lk::test(n)(ewsl,v1,v2);
	}

};


template<typename P>
void Executor::call1()
{
    pl1_call<P>::g(*this);
}

template<typename P>
void Executor::call2()
{
    pl2_call<P>::g(*this);
}


EW_LEAVE
#endif

