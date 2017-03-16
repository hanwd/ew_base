#ifndef __H_EW_SCRIPTING_VARIANT_FUNCTION__
#define __H_EW_SCRIPTING_VARIANT_FUNCTION__

#include "ewa_base/scripting/callable_data.h"
#include "ewa_base/scripting/helpdata.h"
#include "ewa_base/basic/bitflags.h"
#include "ewa_base/basic/pointer.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE CallableFunction : public CallableObject
{
public:

	CallableFunction();
	CallableFunction(const String& s, int f = 0);

	virtual void Serialize(SerializerHelper sh);

	const String& GetName() const{return m_sName;}
	virtual CallableFunction* ToFunction(){return this;}

	BitFlags flags;

	virtual HelpData* __get_helpdata();
	void __set_helpdata(const String& s);

	virtual int __getindex(Executor&,const String&); // top.s

	bool ToValue(String& v,int) const;

	DECLARE_OBJECT_INFO(CallableFunction,ObjectInfo);

protected:
	String m_sName;
	DataPtrT<HelpData> m_pHelp;

};


EW_LEAVE
#endif
