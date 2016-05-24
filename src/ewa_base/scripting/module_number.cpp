#include "ewa_base/scripting.h"
#include <limits>

EW_ENTER

template<>
CallableMetatableT<dcomplex>::CallableMetatableT()
:CallableMetatable("complex")
{
	CG_GGVar::current().sm_meta[type_flag<dcomplex>::value].reset(this);
	table_meta["i_unit"].reset<type>(type(0, 1));
}

template<>
int CallableMetatableT<dcomplex>::__fun_call(Executor& ewsl, int pm)
{
	if(pm==0)
	{
		ewsl.ci0.nbx[1].reset<dcomplex>();
		return 1;
	}
	else if(pm==1)
	{
		Variant z;z.reset(0.0);
		pl2_call<pl_cpx>::g(ewsl.ci0.nbx[1],ewsl.ci0.nbx[1],z);
		return 1;
	}
	else if(pm==2)
	{
		pl2_call<pl_cpx>::g(ewsl.ci0.nbx[1],ewsl.ci0.nbx[1],ewsl.ci0.nbx[2]);
		return 1;
	}

	return CallableData::__fun_call(ewsl,pm);
}

template class CallableMetatableT<dcomplex>;

template<>
CallableMetatableT<int64_t>::CallableMetatableT()
:CallableMetatable("integer")
{
	CG_GGVar::current().sm_meta[type_flag<type>::value].reset(this);
	table_meta["MAX_VALUE"].reset<type>(std::numeric_limits<type>::max());
	table_meta["MIN_VALUE"].reset<type>(std::numeric_limits<type>::min());
}

template<>
int CallableMetatableT<int64_t>::__fun_call(Executor& ewsl, int pm)
{
	if(pm==0)
	{
		ewsl.ci0.nbx[1].reset<type>();
		return 1;
	}
	else if(pm==1)
	{
		ewsl.ci0.nbx[1].reset<type>(variant_cast<type>(ewsl.ci0.nbx[1]));
		return 1;
	}

	return CallableData::__fun_call(ewsl,pm);

}
template class CallableMetatableT<int64_t>;



template<>
CallableMetatableT<double>::CallableMetatableT()
:CallableMetatable("double")
{
	CG_GGVar::current().sm_meta[type_flag<type>::value].reset(this);
	table_meta["MAX_VALUE"].reset<type>(std::numeric_limits<type>::max());
	table_meta["MIN_VALUE"].reset<type>(std::numeric_limits<type>::min());
}

template<>
int CallableMetatableT<double>::__fun_call(Executor& ewsl, int pm)
{
	if(pm==0)
	{
		ewsl.ci0.nbx[1].reset<type>();
		return 1;
	}
	else if(pm==1)
	{
		ewsl.ci0.nbx[1].reset<type>(variant_cast<type>(ewsl.ci0.nbx[1]));
		return 1;
	}

	return CallableData::__fun_call(ewsl,pm);

}
template class CallableMetatableT<double>;



template<>
CallableMetatableT<bool>::CallableMetatableT()
:CallableMetatable("boolean")
{
	CG_GGVar::current().sm_meta[type_flag<type>::value].reset(this);
	table_meta["TRUE"].reset<type>(true);
	table_meta["FALSE"].reset<type>(false);
}

template<>
int CallableMetatableT<bool>::__fun_call(Executor& ewsl, int pm)
{
	if (pm == 0)
	{
		ewsl.ci0.nbx[1].reset<type>();
		return 1;
	}
	else if (pm == 1)
	{
		ewsl.ci0.nbx[1].reset<type>(variant_cast<type>(ewsl.ci0.nbx[1]));
		return 1;
	}

	return CallableData::__fun_call(ewsl, pm);

}
template class CallableMetatableT<bool>;

class CallableFunctionBooleanAnd : public CallableFunction
{
public:
	CallableFunctionBooleanAnd() :CallableFunction("boolean.and"){}
	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this, pm, 2);
		bool f1 = variant_cast<bool>(ewsl.ci0.nbx[1]);
		bool f2 = variant_cast<bool>(ewsl.ci0.nbx[2]);
		ewsl.ci0.nbx[1].reset(f1 && f2);
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionBooleanAnd, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionBooleanAnd, ObjectInfo);


class CallableFunctionBooleanOr : public CallableFunction
{
public:
	CallableFunctionBooleanOr() :CallableFunction("boolean.or"){}
	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this, pm, 2);
		bool f1 = variant_cast<bool>(ewsl.ci0.nbx[1]);
		bool f2 = variant_cast<bool>(ewsl.ci0.nbx[2]);
		ewsl.ci0.nbx[1].reset(f1 || f2);
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionBooleanOr, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionBooleanOr, ObjectInfo);


class CallableFunctionBooleanXor : public CallableFunction
{
public:
	CallableFunctionBooleanXor() :CallableFunction("boolean.xor"){}
	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this, pm, 2);
		bool f1 = variant_cast<bool>(ewsl.ci0.nbx[1]);
		bool f2 = variant_cast<bool>(ewsl.ci0.nbx[2]);
		ewsl.ci0.nbx[1].reset(f1 != f2);
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionBooleanXor, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionBooleanXor, ObjectInfo);


class CallableFunctionBooleanNot : public CallableFunction
{
public:
	CallableFunctionBooleanNot() :CallableFunction("boolean.not"){}
	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this, pm, 1);
		bool f1 = variant_cast<bool>(ewsl.ci0.nbx[1]);
		ewsl.ci0.nbx[1].reset(!f1);
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionBooleanNot, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionBooleanNot, ObjectInfo);



void init_module_number()
{
	CG_GGVar& gi(CG_GGVar::current());

	gi.add_inner<CallableMetatableT<bool> >();

	gi.add_inner<CallableFunctionBooleanAnd>();
	gi.add_inner<CallableFunctionBooleanOr>();
	gi.add_inner<CallableFunctionBooleanXor>();
	gi.add_inner<CallableFunctionBooleanNot>();
	gi.add_inner<CallableMetatableT<int64_t> >();
	gi.add_inner<CallableMetatableT<double> >();
	gi.add_inner<CallableMetatableT<dcomplex> >();

	
}

EW_LEAVE
