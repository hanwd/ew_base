#include "ewa_base/scripting/callable_misc.h"
#include "ewa_base/scripting/callable_array.h"
#include "ewa_base/scripting/callable_iterator.h"
#include "ewa_base/util/strlib.h"

EW_ENTER

CallableFunctionArrayLength::CallableFunctionArrayLength(const String& name,int v) :CallableFunction(name, v){}

int CallableFunctionArrayLength::__fun_call(Executor& ewsl, int pm)
{
	ewsl.check_pmc(this, pm, 1);
	return do_apply(ewsl, ewsl.ci0.nbx[1]);
}

int CallableFunctionArrayLength::do_apply(Executor& ewsl, Variant& v)
{
	arr_xt_dims dims;
	if (!v.kptr() || !v.kptr()->__test_dims(dims, 2))
	{
		ewsl.kerror("invalid param");
	}	
	ewsl.ci0.nbx[1].reset(dims[0]);
	return 1;
}

IMPLEMENT_OBJECT_INFO(CallableFunctionArrayLength, ObjectInfo);

CallableFunctionReverse::CallableFunctionReverse(const String& name):CallableFunction(name){}


template<unsigned N>
class fk_reverse
{
public:
	template<typename T>
	static int g(Executor&,const T&)
	{
		return 1;
	}
	static int g(Executor& ewsl,const String& v)
	{
		StringBuffer<wchar_t> sb(v);
		std::reverse(sb.begin(),sb.end());
		ewsl.ci0.nbx[1].reset(String(sb));
		return 1;
	}
	template<typename T>
	static int g(Executor& ewsl,const arr_xt<T>& v)
	{
		arr_xt<T> x(v);
		std::reverse(x.begin(),x.end());
		ewsl.ci0.nbx[1].reset(x);
		return 1;
	}
	static int value(Executor& ewsl,const Variant& v)
	{
		typedef typename flag_type<N>::type type;
		return g(ewsl,variant_handler<type>::raw(v));
	}
};


int CallableFunctionReverse::do_apply(Executor& ewsl,Variant& var)
{
	typedef int (*fn)(Executor&,const Variant&);
	typedef lookup_table_4bit<fk_reverse,fn> lk;
	return lk::test(var.type())(ewsl,var);
}

int CallableFunctionReverse::__fun_call(Executor& ewsl,int pm)
{
	ewsl.check_pmc(this,pm,1);
	return do_apply(ewsl,*ewsl.ci1.nsp);
}

IMPLEMENT_OBJECT_INFO(CallableFunctionReverse, ObjectInfo);



CallableFunctionPack::CallableFunctionPack():CallableFunction("pack"){}

int CallableFunctionPack::__fun_call(Executor& ewsl,int pm)
{
	DataPtrT<CallableWrapT<arr_xt<Variant> > > p(new CallableWrapT<arr_xt<Variant> >);
	p->value.assign(ewsl.ci0.nbx+1,pm);
	p->value.reshape(1,pm);
	ewsl.ci0.nbx[1].kptr(p.get());
	return 1;
}

IMPLEMENT_OBJECT_INFO(CallableFunctionPack, ObjectInfo);

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

CallableFunctionUnpack::CallableFunctionUnpack(const String& name):CallableFunction(name){}


int CallableFunctionUnpack::do_apply(Executor& ewsl,Variant& var)
{
	typedef int (*fn)(Executor&,const Variant&);
	typedef lookup_table_4bit<fk_unpack,fn> lk;
	return lk::test(var.type())(ewsl,var);
}

int CallableFunctionUnpack::__fun_call(Executor& ewsl,int pm)
{
	ewsl.check_pmc(this,pm,1);
	return do_apply(ewsl,*ewsl.ci1.nsp);
}

IMPLEMENT_OBJECT_INFO(CallableFunctionUnpack, ObjectInfo);




template<unsigned N>
class fk_array_size
{
public:

	template<typename T>
	static void g(Executor&,const T&,const arr_xt_dims&)
	{
		Exception::XError("not array");
	}

	template<typename T>
	static void g(Executor& ewsl,arr_xt<T>& v,const arr_xt_dims& dims)
	{
		v.resize(dims);
	}

	template<typename T>
	static void k(Executor&,const T&,const arr_xt_dims&)
	{
		Exception::XError("not array");
	}

	template<typename T>
	static void k(Executor& ewsl,arr_xt<T>& v,const arr_xt_dims& dims)
	{
		v.reshape(dims);
	}

	static int value(Executor& ewsl,Variant& v,const arr_xt_dims& dims,bool re)
	{
		typedef typename flag_type<N>::type type;
		if(re)
		{
			g(ewsl,variant_handler<type>::raw(v),dims);
		}
		else
		{
			k(ewsl,variant_handler<type>::raw(v),dims);
		}
		ewsl.ci0.nbx[1]=v;
		return 1;
	}

};




void CallableFunctionResize::update_dims(arr_xt_dims& dims,Variant* nbp,int pm)
{
	if(pm==1)
	{
		arr_xt<int64_t> d=variant_cast<arr_xt<int64_t> >(nbp[0]);
		if(d.size()==1)
		{
			dims[0]=1;
			dims[1]=d[0];
		}
		else if(d.size()<6)
		{
			for(size_t i=0;i<d.size();i++)
			{
				dims[i]=d[i];
			}
		}
		else
		{
			Exception::XError("too many dims");
		}
	}
	else
	{
		for(int i=0;i<pm;i++)
		{
			dims[i]=variant_cast<int64_t>(nbp[i]);
		}
	}
}

CallableFunctionResize::CallableFunctionResize(const String& name):CallableFunction(name){}

int CallableFunctionResize::do_apply(Executor& ewsl,Variant& var,Variant* nbp,int pm)
{

	arr_xt_dims _array_dims;
	CallableFunctionResize::update_dims(_array_dims,nbp,pm);

	typedef int (*fn)(Executor&,Variant&,const arr_xt_dims&,bool);
	typedef lookup_table_4bit<fk_array_size,fn> lk;
	return lk::test(var.type())(ewsl,var,_array_dims,true);
	
}

int CallableFunctionResize::__fun_call(Executor& ewsl,int pm)
{
	ewsl.check_pmc(this,pm,2,7);
	return do_apply(ewsl,ewsl.ci0.nbx[1],ewsl.ci0.nbx+2,pm-1);
}

IMPLEMENT_OBJECT_INFO(CallableFunctionResize, ObjectInfo);

CallableFunctionReshape::CallableFunctionReshape(const String& name):CallableFunction(name){}

int CallableFunctionReshape::do_apply(Executor& ewsl,Variant& var,Variant* nbp,int pm)
{

	arr_xt_dims _array_dims;
	CallableFunctionResize::update_dims(_array_dims,nbp,pm);

	typedef int (*fn)(Executor&,Variant&,const arr_xt_dims&,bool);
	typedef lookup_table_4bit<fk_array_size,fn> lk;
	return lk::test(var.type())(ewsl,var,_array_dims,false);
	
}

int CallableFunctionReshape::__fun_call(Executor& ewsl,int pm)
{
	ewsl.check_pmc(this,pm,2,7);
	return do_apply(ewsl,ewsl.ci0.nbx[1],ewsl.ci0.nbx+2,pm-1);
}

IMPLEMENT_OBJECT_INFO(CallableFunctionReshape, ObjectInfo);
EW_LEAVE
