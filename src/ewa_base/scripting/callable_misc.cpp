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

EW_LEAVE
