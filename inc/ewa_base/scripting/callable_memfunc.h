#ifndef __H_EW_SCRIPTING_VARIANT_MEMFUNC__
#define __H_EW_SCRIPTING_VARIANT_MEMFUNC__

#include "ewa_base/scripting/scanner.h"
#include "ewa_base/scripting/callable_table.h"


EW_ENTER


template<typename T>
class FunctionParamT;

template<>
class FunctionParamT<int>
{
public:
	typedef int const_reference;
	int64_t value;
	bool update(Variant& v)
	{
		value=pl_cast<int64_t>::g(v);
		return true;
	}
};

template<>
class FunctionParamT<double>
{
public:
	typedef double const_reference;
	double value;

	bool update(Variant& v)
	{
		value=pl_cast<double>::g(v);
		return true;
	}
};

template<> 
class FunctionParamT<String>
{
public:
	typedef const String& const_reference;
	String value;
	bool update(Variant& v)
	{
		return v.get(value);
	}
};

template<> 
class FunctionParamT<Variant>
{
public:
	typedef const Variant& const_reference;
	Variant value;
	bool update(Variant& v)
	{
		value=v;
		return true;
	}
};

template<typename B>
class CallableFunctionTargetFunc0 : public B
{
public:

	typedef typename B::target_type target_type;
	typedef typename B::return_type return_type;

	typedef return_type (target_type::*func_type)();
	func_type func;

	CallableFunctionTargetFunc0(const String& n,func_type fn)
		:B(n)
		,func(fn)
	{

	}

	int __fun_call(Executor& ewsl,int pm)
	{
		target_type& Target(this->get_target(ewsl,ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr()));

		if(pm<0) ewsl.kerror("invalid param");

		(Target.*func)();

		ewsl.ci0.nbx[1]=ewsl.ci1.nbp[StackState1::SBASE_THIS];
		return 1;
	}
};


template<typename B,typename T1>
class CallableFunctionTargetFunc1 : public B
{
public:

	typedef typename B::target_type target_type;
	typedef typename B::return_type return_type;

	typedef return_type (target_type::*func_type)(typename FunctionParamT<T1>::const_reference);
	func_type func;

	CallableFunctionTargetFunc1(const String& n,func_type fn)
		:B(n)
		,func(fn)
	{

	}

	int __fun_call(Executor& ewsl,int pm)
	{
		target_type& Target(this->get_target(ewsl,ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr()));

		if(pm<1) ewsl.kerror("invalid param");

		typename FunctionParamT<T1> val1;
		if(val1.update(ewsl.ci0.nbx[1]))
		{
			(Target.*func)(val1.value);
		}
		else
		{
			ewsl.kerror("invalid param");
		}

		ewsl.ci0.nbx[1]=ewsl.ci1.nbp[StackState1::SBASE_THIS];
		return 1;
	}
};


template<typename B,typename T1,typename T2>
class CallableFunctionTargetFunc2 : public B
{
public:

	typedef typename B::target_type target_type;
	typedef typename B::return_type return_type;

	typedef return_type (target_type::*func_type)(
		typename FunctionParamT<T1>::const_reference,
		typename FunctionParamT<T2>::const_reference
		);

	func_type func;

	CallableFunctionTargetFunc2(const String& n,func_type fn)
		:B(n)
		,func(fn)
	{

	}

	int __fun_call(Executor& ewsl,int pm)
	{
		target_type& Target(this->get_target(ewsl,ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr()));

		if(pm<1) ewsl.kerror("invalid param");

		typename FunctionParamT<T1> val1;
		typename FunctionParamT<T2> val2;

		if(val1.update(ewsl.ci0.nbx[1]) && val2.update(ewsl.ci0.nbx[2]))
		{
			(Target.*func)(val1.value,val2.value);
		}
		else
		{
			ewsl.kerror("invalid param");
		}

		ewsl.ci0.nbx[1]=ewsl.ci1.nbp[StackState1::SBASE_THIS];
		return 1;
	}

};


template<typename B,typename T1,typename T2,typename T3>
class CallableFunctionTargetFunc3 : public B
{
public:

	typedef typename B::target_type target_type;
	typedef typename B::return_type return_type;

	typedef return_type (target_type::*func_type)(
		typename FunctionParamT<T1>::const_reference,
		typename FunctionParamT<T2>::const_reference,
		typename FunctionParamT<T3>::const_reference
		);

	func_type func;

	CallableFunctionTargetFunc3(const String& n,func_type fn)
		:B(n)
		,func(fn)
	{

	}

	int __fun_call(Executor& ewsl,int pm)
	{
		target_type& Target(this->get_target(ewsl,ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr()));

		if(pm<1) ewsl.kerror("invalid param");

		typename FunctionParamT<T1> val1;
		typename FunctionParamT<T2> val2;
		typename FunctionParamT<T3> val3;

		if(val1.update(ewsl.ci0.nbx[1]) && val2.update(ewsl.ci0.nbx[2]) && val3.update(ewsl.ci0.nbx[3]))
		{
			(Target.*func)(val1.value,val2.value,val3.value);
		}
		else
		{
			ewsl.kerror("invalid param");
		}

		ewsl.ci0.nbx[1]=ewsl.ci1.nbp[StackState1::SBASE_THIS];
		return 1;
	}

};

EW_LEAVE
#endif
