#ifndef __H_EW_SCRIPTING_VARIANT_BYTECODE__
#define __H_EW_SCRIPTING_VARIANT_BYTECODE__

#include "ewa_base/scripting/variant.h"
#include "ewa_base/scripting/bytecode_inst.h"
#include "ewa_base/scripting/callable_function.h"
#include "ewa_base/scripting/scanner.h"


EW_ENTER


class DLLIMPEXP_EWA_BASE CallableSource : public CallableData
{
public:
	CallableSource();
	CallableSource(const String& s);

	void Serialize(Serializer& ar);

	CallableData* DoClone(ObjectCloneState& cs)
	{
		return cs.type==0?this:new CallableSource(*this); 
	}

	arr_1t<String> aLines;

	DECLARE_OBJECT_INFO(CallableSource, ObjectInfo);
};

class DLLIMPEXP_EWA_BASE CallableCode : public CallableFunction
{
public:

	enum
	{
		STATE_SYSTEM_VARIABLES_LOADED=1<<0,
	};

	CallableCode();
	~CallableCode();

	arr_1t<XopInst> aInsts;
	arr_1t<Variant> aLLVar;

	size_t nLocal;	// stackframe size
	size_t nParam;	// function param count
	size_t nShift;	// captured target shift


	bst_set<String> aDepends;
	arr_1t<tokInfo> aInfos;

	tokInfo tok_beg,tok_end;

	DataPtrT<CallableSource> pSource;

	virtual CallableCode* ToCode(){return this;}

	int __fun_call(Executor&,int);

	void SetName(const String& s) { m_sName=s;}

	void Serialize(Serializer& ar);

	void __set_helpdata(const String& s);

	int __getindex(Executor& ewsl,const String& index);

	bool ToValue(String& v,int) const;

	DECLARE_OBJECT_INFO(CallableCode,ObjectInfo);

protected:

	BitFlags state;

};

class DLLIMPEXP_EWA_BASE CallableClosure : public CallableObject
{
public:

	CallableClosure(){}

	DataPtrT<CallableCode> pInst;
	arr_1t<Variant> aCapture;

	int __fun_call(Executor& ewsl,int pm);

	virtual CallableClosure* ToClosure(){return this;}

	void Serialize(Serializer& ar);

	DECLARE_OBJECT_INFO(CallableClosure,ObjectInfo);
};

EW_LEAVE



#endif
