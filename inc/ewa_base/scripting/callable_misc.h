#ifndef __H_EW_SCRIPTING_CALLABLE_MISC__
#define __H_EW_SCRIPTING_CALLABLE_MISC__


#include "ewa_base/scripting/callable_table.h"
#include "ewa_base/scripting/callable_array.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE CallableFunctionArrayLength : public CallableFunction
{
public:

	CallableFunctionArrayLength(const String& name="length",int v=0);

	virtual int __fun_call(Executor& ewsl, int pm);
	static int do_apply(Executor& ewsl, Variant& v);

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionArrayLength, ObjectInfo);
};


class DLLIMPEXP_EWA_BASE CallableFunctionReverse : public CallableFunction
{
public:
	CallableFunctionReverse(const String& name="reverse");

	static int do_apply(Executor& ewsl,Variant& var);

	virtual int __fun_call(Executor& ewsl,int pm);

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionReverse, ObjectInfo);
};



class DLLIMPEXP_EWA_BASE CallableFunctionPack : public CallableFunction
{
public:
	CallableFunctionPack();
	virtual int __fun_call(Executor& ewsl,int pm);
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionPack, ObjectInfo);
};


class DLLIMPEXP_EWA_BASE CallableFunctionUnpack : public CallableFunction
{
public:
	CallableFunctionUnpack(const String& name="unpack");

	static int do_apply(Executor& ewsl,Variant& var);
	virtual int __fun_call(Executor& ewsl,int pm);

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionUnpack, ObjectInfo);
};


EW_LEAVE
#endif