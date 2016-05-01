#include "ewa_base/scripting.h"

EW_ENTER


template<int N>
class CallableFunctionGetIteratorT : public CallableFunction
{
public:
	CallableFunctionGetIteratorT():CallableFunction(String::Format("#get_iterator%d",N)){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		CallableData* p=ewsl.ci1.nsp[0].kptr();
		if(!p)
		{
			ewsl.kerror("invalid get_iterator");
		}
		else
		{
			p->__get_iterator(ewsl,N);
		}
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionGetIteratorT, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO_N1(CallableFunctionGetIteratorT, ObjectInfo);


class CallableFunctionColon : public CallableFunction
{
public:
	CallableFunctionColon():CallableFunction("colon"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		double s1,s2,ds;
		if(pm==3)
		{
			s1=pl_cast<double>::g(ewsl.ci1.nsp[-2]);
			ds=pl_cast<double>::g(ewsl.ci1.nsp[-1]);
			s2=pl_cast<double>::g(ewsl.ci1.nsp[-0]);
		}
		else if(pm==2)
		{
			s1=pl_cast<double>::g(ewsl.ci1.nsp[-1]);
			s2=pl_cast<double>::g(ewsl.ci1.nsp[-0]);
			ds=s2>s1?1.0:-1.0;
		}
		else
		{
			ewsl.kerror("invalid pm_count");
		}


		size_t sz=1+::floor((s2-s1)/ds);
		arr_xt<double>& xt(ewsl.ci0.nbx[1].ref_unique<arr_xt<double> >());
		xt.resize(1,sz);

		for(size_t i=0;i<sz;i++)
		{
			xt[i]=s1+ds*i;
		}

		return 1;
	}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionColon, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionColon, ObjectInfo);

class CallableFunctionColon_data : public CallableFunction
{
public:
	CallableFunctionColon_data():CallableFunction("#colon_data"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ColonData d;
		if(pm==3)
		{
			if(!ewsl.ci0.nbx[1].is_nil())
			{
				d.set_beg(pl_cast<double>::g(ewsl.ci0.nbx[1]));
			}
			if(!ewsl.ci0.nbx[2].is_nil())
			{
				d.set_mid(pl_cast<double>::g(ewsl.ci0.nbx[2]));
			}
			if(!ewsl.ci0.nbx[3].is_nil())
			{
				d.set_end(pl_cast<double>::g(ewsl.ci0.nbx[3]));
			}
		}
		else if(pm==2)
		{
			if(!ewsl.ci0.nbx[1].is_nil())
			{
				d.set_beg(pl_cast<double>::g(ewsl.ci0.nbx[1]));
			}
			if(!ewsl.ci0.nbx[2].is_nil())
			{
				d.set_end(pl_cast<double>::g(ewsl.ci0.nbx[2]));
			}
		}
		else
		{
			ewsl.kerror("invalid pm_count");
		}

		ewsl.ci0.nbx[1].reset(d);
		return 1;
	}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionColon_data, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionColon_data, ObjectInfo);

class CallableFunctionLinspace : public CallableFunction
{
public:
	CallableFunctionLinspace():CallableFunction("linspace"){}

	int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,3);

		double v1=pl_cast<double>::g(ewsl.ci1.nsp[-2]);
		double v2=pl_cast<double>::g(ewsl.ci1.nsp[-1]);
		intptr_t sz=pl_cast<intptr_t>::g(ewsl.ci1.nsp[0]);

		if(sz<1)
		{
			ewsl.kerror("invalid param");
		}

		arr_xt<double>& val(ewsl.ci0.nbx[1].ref_unique<arr_xt<double> >());
		val.resize(1,
			sz);

		if(sz>1)
		{
			intptr_t n=sz-1;
			double _dMax=double(n);
			for(intptr_t i=0;i<=n;i++)
			{
				double p1=double(i)/_dMax;
				double p2=double(n-i)/_dMax;
				val(i)=v1*p2+v2*p1;
			}
		}
		else
		{
			val(0)=0.5*(v1+v2);
		}

		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionLinspace, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionLinspace, ObjectInfo);

template<typename T>
class CallableFunctionArrayValue : public CallableFunction
{
public:

	T nValue;
	typedef arr_xt<T> arr_type;

	CallableFunctionArrayValue(const String& s,const T& v):CallableFunction(s),nValue(v){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this, pm, 1, 6);

		Variant* nbp=ewsl.ci1.nsp-pm+1;

		arr_xt_dims xtd;
		for(int i=0;i<pm;i++)
		{
			xtd[i]=pl_cast<size_t>::g(nbp[i]);
		}

		arr_type& val(nbp[0].ref_unique<arr_type>());
		
		val.resize(xtd);
		if(nValue!=T()){for(size_t i=0;i<val.size();i++) val(i)=nValue;}

		return 1;
	}
};

template<typename T>
class CallableFunctionZeros : public CallableFunctionArrayValue<T>
{
public:
	CallableFunctionZeros():CallableFunctionArrayValue<T>("zeros",T(0)){}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionZeros, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO_T1(CallableFunctionZeros, ObjectInfo);


template<typename T>
class CallableFunctionOnes : public CallableFunctionArrayValue<T>
{
public:
	CallableFunctionOnes():CallableFunctionArrayValue<T>("ones",T(1)){}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionOnes, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO_T1(CallableFunctionOnes, ObjectInfo);


class CallableFunctionArrayDiag : public CallableFunction
{
public:

	CallableFunctionArrayDiag():CallableFunction("array.diag",1){}

	template<typename T>
	int _diag(Variant& v)
	{
		typedef arr_xt<T> arr_type;

		arr_type& rhs(v.ref<arr_type>());
		arr_type lhs;

		size_t n=rhs.size();
		lhs.resize(n,n);
		for(size_t i=0;i<n;i++) lhs(i,i)=rhs(i);

		v.ref_unique<arr_type>().swap(lhs);

		return 1;
	}

	template<typename T,typename T2>
	int _diag2(Variant& v)
	{
		typedef arr_xt<T2> arr_type;

		arr_type& rhs(v.ref<arr_type>());
		arr_xt<T> lhs;

		size_t n=rhs.size();
		lhs.resize(n,n);
		for(size_t i=0;i<n;i++) lhs(i,i)=pl_cast<T>::g(rhs(i));

		v.ref_unique<arr_xt<T> >().swap(lhs);

		return 1;
	}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);

		Variant& v(ewsl.ci1.nsp[0]);

		int t=v.type();
		if(t==type_flag<arr_xt<double> >::value)
		{
			return _diag<double>(v);;
		}
		if(t==type_flag<arr_xt<int64_t> >::value)
		{
			return _diag<int64_t>(v);;
		}
		if(t==type_flag<arr_xt<dcomplex> >::value)
		{
			return _diag<dcomplex>(v);;
		}

		if(t==type_flag<arr_xt<Variant> >::value)
		{
			arr_xt<Variant>& a(v.ref<arr_xt<Variant> >());
			int k=0;
			for(size_t i=0;i<a.size();i++)
			{
				if(v.type()>k) k=v.type();
			}
			if(k<=type_flag<int64_t>::value)
			{
				return _diag2<int64_t,Variant>(v);
			}
			if(k<=type_flag<double>::value)
			{
				return _diag2<double,Variant>(v);
			}
			if(k<=type_flag<dcomplex>::value)
			{
				return _diag2<dcomplex,Variant>(v);
			}
			return _diag2<double,Variant>(v);
		}

		ewsl.kerror("invalid param");
		return 0;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionArrayDiag, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionArrayDiag, ObjectInfo);

class CallableFunctionGetArrayIndex : public CallableFunction
{
public:
	typedef arr_xt<double> arr_type;

	CallableFunctionGetArrayIndex():CallableFunction("#arr_getindex"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,2);

		CallableData* _arr=ewsl.ci1.nsp[-1].kptr();;
		int idx=ewsl.ci1.nsp[0].ref<int64_t>();

		ewsl.ci1.nsp-=2;

		if(_arr->__getarray_index_range(ewsl,idx)!=2)
		{
			ewsl.kerror("invalid array");
		}

		return 2;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionGetArrayIndex, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionGetArrayIndex, ObjectInfo);


class CallableFunctionPack : public CallableFunction
{
public:
	CallableFunctionPack():CallableFunction("pack"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		DataPtrT<CallableWrapT<arr_xt<Variant> > > p(new CallableWrapT<arr_xt<Variant> >);
		Variant* _bp=ewsl.ci0.nbx+1;
		p->value.assign(_bp,pm);
		p->value.reshape(1,pm);

		_bp[0].kptr(p.get());
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionPack, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionPack, ObjectInfo);

class CallableFunctionDot3Adjust : public CallableFunction
{
public:
	CallableFunctionDot3Adjust():CallableFunction("#dot3_adjust"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		for(int i=pm+1;i<=ewsl.ci0.kep;i++)
		{
			ewsl.ci0.nbx[i].clear();
		}

		DataPtrT<CallableWrapT<arr_xt<Variant> > > p(new CallableWrapT<arr_xt<Variant> >);
		Variant* _bp=ewsl.ci0.nbx+ewsl.ci0.kep;
		int _n=pm-ewsl.ci0.kep+1;
		if(_n>0)
		{
			p->value.assign(_bp,_n);
		}

		_bp[0].kptr(p.get());
		return ewsl.ci0.kep;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionDot3Adjust, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionDot3Adjust, ObjectInfo);

class CallableFunctionKepnAdjust : public CallableFunction
{
public:
	CallableFunctionKepnAdjust():CallableFunction("#kepn_adjust"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		for(int i=pm+1;i<=ewsl.ci0.kep;i++)
		{
			ewsl.ci0.nbx[i].clear();
		}
		return ewsl.ci0.kep;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionKepnAdjust, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionKepnAdjust, ObjectInfo);

template<unsigned N>
class fk_unpack
{
public:

	template<typename T>
	static int g(Executor&,const T&)
	{
		return 1;
	}

	template<typename T>
	static int g(Executor& ewsl,const arr_xt<T>& v)
	{
		DataPtrT<ObjectData> tmp(ewsl.ci1.nsp[0].kptr());
		size_t n = v.size();
		ewsl.check_stk(n);
		for (size_t i = 0; i < n; i++)
		{
			ewsl.ci0.nbx[1 + i].reset(v[i]);
		}
		return n;
	}

	static int value(Executor& ewsl,const Variant& v)
	{
		typedef typename flag_type<N>::type type;
		return g(ewsl,variant_handler<type>::raw(v));
	}
};

class CallableFunctionUnpack : public CallableFunction
{
public:

	CallableFunctionUnpack():CallableFunction("unpack"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		typedef int (*fn)(Executor&,const Variant&);
		typedef lookup_table_4bit<fk_unpack,fn> lk;
		Variant& v(*ewsl.ci1.nsp);
		return lk::test(v.type())(ewsl,v);
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionUnpack, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionUnpack, ObjectInfo);

template<unsigned N>
class fk_trans
{
public:

	template<typename T>
	static int g(Executor& ewsl,const T&)
	{
		ewsl.ci0.nbx[1].unique();
		return 1;
	}

	template<typename T>
	static int g(Executor& ewsl,const arr_xt<T>& v1)
	{
		arr_xt_dims dims=v1.size_ptr();
		std::swap(dims[0],dims[1]);

		arr_xt<T> v2;
		v2.resize(dims);

		size_t n3=dims[2]*dims[3]*dims[4]*dims[5];
		for(size_t k=0;k<n3;k++)
		{
			for(size_t i=0;i<dims[0];i++)
			{
				for(size_t j=0;j<dims[1];j++)
				{
					v2(i,j,k)=v1(j,i,k);
				}
			}
		}

		ewsl.ci0.nbx[1].ref_unique<arr_xt<T> >().swap(v2);
		return 1;
	}

	static int value(Executor& ewsl,const Variant& v)
	{
		typedef typename flag_type<N>::type type;
		return g(ewsl,variant_handler<type>::raw(v));
	}
};


class CallableFunctionArrayTrans : public CallableFunction
{
public:

	CallableFunctionArrayTrans():CallableFunction("array.trans",1){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if (pm == 1)
		{
			return do_apply(ewsl, ewsl.ci0.nbx[1]);
		}
		else if (pm == 0)
		{
			return do_apply(ewsl, ewsl.ci1.nbp[StackState1::SBASE_THIS]);
		}
		else
		{
			ewsl.check_pmc(this, pm,1);
			return INVALID_CALL;
		}
	}

	static int do_apply(Executor& ewsl,Variant& v)
	{
		typedef int (*fn)(Executor&,const Variant&);
		typedef lookup_table_4bit<fk_trans,fn> lk;
		lk::test(v.type())(ewsl, v);
		return 1;
	}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionArrayTrans, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionArrayTrans, ObjectInfo);


class CallableFunctionArraySize : public CallableFunction
{
public:

	CallableFunctionArraySize() :CallableFunction("array.size",1){}

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		if (pm == 1)
		{
			return do_apply(ewsl, ewsl.ci0.nbx[1]);
		}
		else if (pm == 0)
		{
			return do_apply(ewsl, ewsl.ci1.nbp[StackState1::SBASE_THIS]);
		}
		else
		{
			ewsl.check_pmc(this, pm, 1);
			return INVALID_CALL;
		}
	}

	static int do_apply(Executor& ewsl, Variant& v)
	{
		arr_xt_dims dims;
		if (!v.kptr() || !v.kptr()->__test_dims(dims, 0))
		{
			ewsl.kerror("invalid param");
		}

		arr_xt<int64_t> d;d.assign(dims.ptr(), 6);
		ewsl.ci0.nbx[1].reset(d);

		return 1;
	}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionArraySize, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionArraySize, ObjectInfo);


class CallableFunctionArrayLength : public CallableFunction
{
public:

	CallableFunctionArrayLength() :CallableFunction("array.length", 1){}

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		if (pm == 1)
		{
			return do_apply(ewsl, ewsl.ci0.nbx[1]);
		}
		else if (pm == 0)
		{
			return do_apply(ewsl, ewsl.ci1.nbp[StackState1::SBASE_THIS]);
		}
		else
		{
			ewsl.check_pmc(this, pm, 1);
			return INVALID_CALL;
		}
	}

	static int do_apply(Executor& ewsl, Variant& v)
	{
		arr_xt_dims dims;
		if (!v.kptr() || !v.kptr()->__test_dims(dims, 2))
		{
			ewsl.kerror("invalid param");
		}	
		ewsl.ci0.nbx[1].reset(dims[0]);
		return 1;
	}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionArrayLength, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionArrayLength, ObjectInfo);


template<typename T>
class CallableMetatableT<arr_xt<T> > : public CallableMetatable
{
public:

	typedef arr_xt<T> type;
	typedef T scalar_type;

	CallableMetatableT();
	virtual int __fun_call(Executor& ewsl, int pm);

	DECLARE_OBJECT_CACHED_INFO(CallableMetatableT, ObjectInfo);
};

template<typename T>
struct callable_metatable_array_name
{
	static String name();
};

template<> 
String callable_metatable_array_name<double>::name()
{
	return "array_double";
};
template<> 
String callable_metatable_array_name<int64_t>::name()
{
	return "array_integer";
};
template<> 
String callable_metatable_array_name<dcomplex>::name()
{
	return "array_complex";
};
template<> 
String callable_metatable_array_name<Variant>::name()
{
	return "array_variant";
};



template<typename T>
CallableMetatableT<arr_xt<T> >::CallableMetatableT()
:CallableMetatable(callable_metatable_array_name<T>::name())
{
	table_meta["zeros"].kptr(CallableFunctionZeros<scalar_type>::sm_info.CreateObject());
	table_meta["ones"].kptr(CallableFunctionOnes<scalar_type>::sm_info.CreateObject());
	table_meta["trans"].kptr(CallableFunctionArrayTrans::sm_info.CreateObject());
	table_meta["size"].kptr(CallableFunctionArraySize::sm_info.CreateObject());
	table_meta["length"].kptr(CallableFunctionArrayLength::sm_info.CreateObject());

	CG_GGVar::current().sm_meta[type_flag<type>::value].reset(this);
}

template<typename T>
int CallableMetatableT<arr_xt<T> >::__fun_call(Executor& ewsl, int pm)
{

	ewsl.check_pmc(this, pm, 1, 6);

	arr_xt_dims dims;
	Variant* _bp = ewsl.ci0.nbx + 1;
	for (int i = 0; i<pm; i++)
	{
		dims[i] = pl_cast<size_t>::g(_bp[i]);
	}
	type arr;
	arr.resize(dims);

	ewsl.ci0.nbx[1].reset(arr);
	return 1;

}

template<typename T> 
typename CallableMetatableT<arr_xt<T> >::infotype  
CallableMetatableT<arr_xt<T> >::sm_info(ObjectNameT<T>::MakeName("CallableMetatableT"));

//template class CallableMetatableT<arr_xt<bool> >;
template class CallableMetatableT<arr_xt<int64_t> >;
template class CallableMetatableT<arr_xt<double> >;
template class CallableMetatableT<arr_xt<dcomplex> >;
template class CallableMetatableT<arr_xt<Variant> >;

void init_module_array()
{
	CG_GGVar& gi(CG_GGVar::current());

	gi.add_inner<CallableMetatableT<arr_xt<int64_t> > >();
	gi.add_inner<CallableMetatableT<arr_xt<double> > >();
	gi.add_inner<CallableMetatableT<arr_xt<dcomplex> > >();
	gi.add_inner<CallableMetatableT<arr_xt<Variant> > >();
	
	gi.add_inner<CallableFunctionArraySize>();
	gi.add_inner<CallableFunctionArrayLength>();
	gi.add_inner<CallableFunctionArrayTrans>();
	gi.add_inner<CallableFunctionArrayDiag>();

	gi.add_inner<CallableFunctionZeros<double> >();
	gi.add_inner<CallableFunctionOnes<double>>();

	gi.add_inner<CallableFunctionColon>();
	gi.add_inner<CallableFunctionArrayTrans>();

	gi.add_inner<CallableFunctionColon_data>();
	gi.add_inner<CallableFunctionLinspace>();
	gi.add_inner<CallableFunctionPack>();
	gi.add_inner<CallableFunctionUnpack>();
	gi.add_inner<CallableFunctionDot3Adjust>();
	gi.add_inner<CallableFunctionKepnAdjust>();

	gi.add_inner<CallableFunctionGetIteratorT<1> >();
	gi.add_inner<CallableFunctionGetIteratorT<2> >();
	gi.add_inner<CallableFunctionGetArrayIndex>();

}


EW_LEAVE
