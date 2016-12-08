
#ifndef __H_EW_SCRIPTING_VARIANT_OP__
#define __H_EW_SCRIPTING_VARIANT_OP__

#include "ewa_base/scripting/variant.h"
#include "ewa_base/scripting/callable_table.h"
#include "ewa_base/scripting/callable_class.h"
#include "ewa_base/math/math_op.h"


EW_ENTER




template<typename T>
class pl_cast_base;

template<>
class pl_cast_base<bool>
{
public:
	typedef bool type;

	template<typename O>
	static type g(const O&){Exception::XBadCast();return type();}

	static type g(const VariantTable&){return true;}

	static type g(const type& v){return v;}
	static type g(int32_t v){return v!=0;}
	static type g(int64_t v){return v!=0;}
	static type g(float v){return v!=0;}
	static type g(double v){return v!=0;}
	static type g(const String& v){return !v.empty();}
	static type g(CallableData* d){return d!=NULL;}

	template<typename T>
	static type g(const std::complex<T>& o)
	{
		return g(o.real())||g(o.imag());
	}

	template<typename O>
	static type g(const arr_xt<O>& v)
	{
		if(v.size()==1) return g(v(0));
		Exception::XBadCast();
		return type();
	}

};


template<>
class pl_cast_base<VariantTable>
{
public:
	typedef VariantTable type;

	static const type& g(const type& v){return v;}
	static type& g(type& v){return v;}

	template<typename O>
	static type g(const O&){Exception::XBadCast();return type();}

};


template<>
class pl_cast_base<DataPtrT<CallableData> >
{
public:
	typedef DataPtrT<CallableData> type;

	template<typename O>
	static type g(const O&){Exception::XBadCast();return type();}

};



template<typename T>
class kany_cast_number
{
public:
	typedef T type;

	template<typename O>
	static type g(const O&){Exception::XBadCast();return type();}

	static type g(bool v){return (type)v;}
	static type g(int32_t v){return (type)v;}
	static type g(int64_t v){return (type)v;}
	static type g(uint32_t v){return (type)v;}
	static type g(uint64_t v){return (type)v;}

	static type g(float v){return (type)v;}
	static type g(double v){return (type)v;}

	static type g(CallableData* v)
	{
		type n(0);
		if(!v)
		{
			Exception::XError("nil cannot be cast to number");
		}

		if(!v->ToValue(n))
		{
			Exception::XBadCast();
		}
		return n;
	}

	static type g(const String& v)
	{
		if(tl::is_same_type<type,double>::value||tl::is_same_type<type,float>::value)
		{
			double k;if(v.ToNumber(&k)) return (type)k;
			Exception::XBadCast();return type();
		}
		else
		{
			int64_t k;
			if(v.ToNumber(&k))
			{
				return g(k);
			}
			Exception::XBadCast();return type();
		}
	}

	static type g(const char* v)
	{
		return g(String(v));
	}

	template<typename O>
	static type g(const arr_xt<O>& v)
	{
		if(v.size()==1) return g(v(0));
		Exception::XBadCast();
		return type();
	}

	template<typename O>
	static type g(const std::complex<O>& v)
	{
		if(v.imag()==type(0)) return g(v.real());
		Exception::XBadCast();
		return type();
	}

};

template<> class pl_cast_base<uint32_t> : public kany_cast_number<uint32_t>{};
template<> class pl_cast_base<uint64_t> : public kany_cast_number<uint64_t>{};
template<> class pl_cast_base<int32_t> : public kany_cast_number<int32_t>{};
template<> class pl_cast_base<int64_t> : public kany_cast_number<int64_t>{};
template<> class pl_cast_base<float> : public kany_cast_number<float>{};
template<> class pl_cast_base<double> : public kany_cast_number<double>{};

template<typename T>
class pl_cast_base<tiny_cpx<T> >
{
public:
	typedef tiny_cpx<T> type;
	template<typename O>
	static type g(const O&){Exception::XBadCast();return type();}

	static type g(bool v){return type(v,0);}
	static type g(int32_t v){return type(v,0);}
	static type g(int64_t v){return type(v,0);}
	static type g(float v){return type(v,0);}
	static type g(double v){return type(v,0);}
	static const type& g(const type& v){return v;}

	template<typename O>
	static type g(const arr_xt<O>& v)
	{
		if(v.size()==1) return g(v(0));
		Exception::XBadCast();
		return type();
	}
};

template<typename T>
class pl_cast_base<std::complex<T> >
{
public:
	typedef std::complex<T> type;
	template<typename O>
	static type g(const O&){Exception::XBadCast();return type();}

	static type g(bool v){return type(v,0);}
	static type g(int32_t v){return type(v,0);}
	static type g(int64_t v){return type(v,0);}
	static type g(float v){return type(v,0);}
	static type g(double v){return type(v,0);}
	static const type& g(const type& v){return v;}

	template<typename Y>
	static const type g(const tiny_cpx<Y>& v)
	{
		return v;
	}

	template<typename O>
	static type g(const arr_xt<O>& v)
	{
		if(v.size()==1) return g(v(0));
		Exception::XBadCast();
		return type();
	}
};

template<>
class pl_cast_base<String>
{
public:
	typedef String type;
	template<typename O>
	static type g(const O&){Exception::XBadCast();return type();}

	static const type& g(const type& v){return v;}
	static type g(bool v){return v?"true":"false";}
	static type g(int32_t v){type t;t<<v;return t;}
	static type g(int64_t v){type t;t<<v;return t;}
	static type g(float v){type t;t<<v;return t;}
	static type g(double v){type t;t<<v;return t;}

	static type g(const VariantTable& o)
	{
		return String::Format("table:0x%p",&o);
	}

	template<typename O>
	static type g(const tiny_cpx<O>& v){type t;t<<"complex("<<v.real()<<","<<v.imag()<<")";return t;}

	template<typename O>
	static type g(const std::complex<O>& v){type t;t<<"complex("<<v.real()<<","<<v.imag()<<")";return t;}

	static type g(CallableData* v)
	{
		type s;if(!v) return "nil";
		v->ToValue(s);
		return s;
	}

	template<typename O>
	static type g(const arr_xt<O>& v)
	{
		return String::Format("[array:0x%p]",&v);
	}

};

template<>
class pl_cast_base<Variant>
{
public:
	typedef Variant type;

	template<typename O>
	static type g(const O& o)
	{
		return Variant(o);
	}


};


template<typename T,typename P>
class vv_cast : public P
{
public:

	using P::g;

	template<unsigned N>
	struct fk
	{
		static T value(const Variant& v)
		{
			EW_ASSERT(N==v.type());
			return P::g(variant_handler<typename flag_type<N>::type>::raw(v));
		}
	};

	typedef T (*fn)(const Variant&);

	static inline T g(const Variant& v)
	{
		return lookup_table_4bit<fk,fn>::test(v.type())(v);
	}

};

template<typename T>
class pl_cast : public vv_cast<T,pl_cast_base<T> >
{
public:

	typedef vv_cast<T,pl_cast_base<T> > basetype;

	using basetype::g;

	static inline T g(const char* v)
	{
		return basetype::g(v);
	}

	template<typename X>
	static inline T g(const X& v,const T& d)
	{
		try
		{
			return g(v);
		}
		catch(...)
		{
			return d;
		}
	}

	static EW_FORCEINLINE T k(const Variant& v)
	{
		if(v.type()==type_flag<T>::value)
		{
			return variant_handler<T>::raw(v);
		}
		else
		{
			return basetype::g(v);
		}
	}

};

template<>
class pl_cast<Variant>
{
public:

	template<typename T>
	static inline Variant g(const T& v)
	{
		Variant t;
		t.reset(v);
		return t;
	}

	static inline const Variant& g(const Variant& v)
	{
		return v;
	}
};


template<typename T>
class pl_cast_base<arr_xt<T> >
{
public:
	typedef arr_xt<T> type;

	template<typename O>
	static type g(const O&){Exception::XBadCast();return type();}

	static type g(bool v){type t;t.resize(1);t(0)=pl_cast_base<T>::g(v);return t;}
	static type g(int32_t v){type t;t.resize(1);t(0)=pl_cast_base<T>::g(v);return t;}
	static type g(int64_t v){type t;t.resize(1);t(0)=pl_cast_base<T>::g(v);return t;}
	static type g(float v){type t;t.resize(1);t(0)=pl_cast_base<T>::g(v);return t;}
	static type g(double v){type t;t.resize(1);t(0)=pl_cast_base<T>::g(v);return t;}
	static type g(const String& v){type t;t.resize(1);t(0)=pl_cast_base<T>::g(v);return t;}

	static const type& g(const type& v){return v;}

	template<typename O>
	static type g(const arr_1t<O>& v)
	{
		type t;
		t.resize(v.size());
		for(size_t i=0;i<v.size();i++) t[i]=vv_cast<T,pl_cast_base<T> >::g(v[i]);
		return t;
	}

	template<typename O>
	static type g(const arr_xt<O>& v)
	{
		type t;
		t.resize(v.size_ptr());

		for(size_t k5=0;k5<v.size(5);k5++)
		for(size_t k4=0;k4<v.size(4);k4++)
		for(size_t k3=0;k3<v.size(3);k3++)
		for(size_t k2=0;k2<v.size(2);k2++)
		for(size_t k1=0;k1<v.size(1);k1++)
		for(size_t k0=0;k0<v.size(0);k0++)
			t(k0,k1,k2,k3,k4,k5)=vv_cast<T,pl_cast_base<T> >::g(v(k0,k1,k2,k3,k4,k5));

		return t;
	}

};


template<typename T,typename T2>
T variant_cast(const T2& v){return pl_cast<T>::g(v);}

template<typename T,typename T2>
T variant_cast(const T2& v,const T& d){return pl_cast<T>::g(v,d);}



template<typename P,unsigned N>
class flag_info1
{
public:

	typedef typename flag_type<N>::scalar scalar;
	typedef typename flag_type<N>::type type;

	static const bool num_enable=P::template rebind<scalar>::value;
	static const bool arr_enable=P::template rebind<type>::value;

	static const int value=(P::dispatch_type==pl_base1::DISPATCH_NUM)?(num_enable?1:0):(arr_enable?2:0);
};


template<typename P>
class pl1_dispatch_base
{
public:
	static int value(Executor& ewsl,Variant&)
	{
		return CallableMetatable::__metatable_call1(ewsl,P::info().op_name);
	}
};


EW_LEAVE

#endif
