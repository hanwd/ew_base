#ifndef __H_EW_SCRIPTING_VARIANT_OTHER__
#define __H_EW_SCRIPTING_VARIANT_OTHER__

#include "ewa_base/scripting/callable_table.h"
#include "ewa_base/scripting/helpdata.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE CallableMetatable : public CallableTableEx
{
public:

	VariantTable table_self;
	VariantTable& table_meta;

	String m_sClassName;

	CallableMetatable(const String& name="");

	void Serialize(Serializer& ar);

	bool ToValue(String& v,int) const;

	virtual CallableData* DoClone(ObjectCloneState& cs)
	{
		if(cs.type==0) return this;
		return new CallableMetatable(*this);
	}

	int __setindex(Executor& ewsl,const String&);
	int __setarray(Executor& ewsl,int pm);

	virtual int __fun_call(Executor&,int);

	static int __metatable_call1(Executor&,const String&);
	static int __metatable_call2(Executor&,const String&);


	//virtual bool __add(Variant&, Variant&, Variant&){ return false; }
	//virtual bool __sub(Variant&, Variant&, Variant&){ return false; }
	//virtual bool __mul(Variant&, Variant&, Variant&){ return false; }
	//virtual bool __div(Variant&, Variant&, Variant&){ return false; }

	//virtual bool __dot_mul(Variant&, Variant&, Variant&){ return false; }
	//virtual bool __dot_div(Variant&, Variant&, Variant&){ return false; }

	//virtual bool __gt(Variant&, Variant&, Variant&){ return false; }
	//virtual bool __lt(Variant&, Variant&, Variant&){ return false; }
	//virtual bool __ge(Variant&, Variant&, Variant&){ return false; }
	//virtual bool __le(Variant&, Variant&, Variant&){ return false; }
	//virtual bool __ne(Variant&, Variant&, Variant&){ return false; }
	//virtual bool __eq(Variant&, Variant&, Variant&){ return false; }

	//virtual bool __shl(Variant&, Variant&, Variant&){ return false; }
	//virtual bool __shr(Variant&, Variant&, Variant&){ return false; }

	virtual CallableMetatable* ToMetatable(){return this;}

	virtual HelpData* __get_helpdata();
	void __set_helpdata(const String& s){ m_pHelp.reset(new HelpData(m_sClassName,s)); }

protected:
	DataPtrT<HelpData> m_pHelp;

	DECLARE_OBJECT_INFO(CallableMetatable, ObjectInfo);
};



class DLLIMPEXP_EWA_BASE CallableModule : public CallableMetatable
{
public:

	CallableModule(const String name=""):CallableMetatable(name){}

	virtual CallableData* DoClone(ObjectCloneState&)
	{
		return this;
	}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		return CallableData::__fun_call(ewsl,pm);
	}

	void Serialize(Serializer& ar)
	{
		ar & m_sClassName;
	}

	bool ToValue(String& v,int) const;

	virtual CallableModule* ToModule(){return this;}

	DECLARE_OBJECT_INFO(CallableModule, ObjectInfo);
};

template<typename T>
class CallableMetatableT : public CallableMetatable
{
public:
	typedef T type;

	CallableMetatableT();
	virtual int __fun_call(Executor& ewsl, int pm);

	DECLARE_OBJECT_CACHED_INFO(CallableMetatableT, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO_T1(CallableMetatableT, ObjectInfo);

class DLLIMPEXP_EWA_BASE CallableClass : public CallableObject
{
public:
	DataPtrT<CallableMetatable> metax;

	CallableClass(CallableMetatable* p=NULL){reset(p);}

	void reset(CallableMetatable* p = NULL);

	arr_1t<Variant> value;

	virtual CallableClass* ToClass() {return this;}
	virtual CallableMetatable* GetMetaTable(){return metax.get();}

	virtual CallableData* DoClone(ObjectCloneState& cs)
	{
		if(cs.type==0) return this;
		return new CallableClass(*this);
	}

	virtual int __getindex(Executor&,const String&); // top.s
	virtual int __setindex(Executor&,const String&); // top.s=val

	bool ToValue(String& v,int) const;

	void Serialize(Serializer& ar);

	virtual void __get_iterator(Executor&,int);

	DECLARE_OBJECT_INFO(CallableClass,ObjectInfo);
};



EW_LEAVE

#endif
