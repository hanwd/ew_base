#ifndef __H_EW_SCRIPTING_VARIANT_TABLE__
#define __H_EW_SCRIPTING_VARIANT_TABLE__


#include "ewa_base/scripting/variant.h"
#include "ewa_base/scripting/callable_wrap.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE VariantTable : public indexer_map<String,Variant>
{
public:

	bool operator==(const VariantTable& v2) const;
	bool operator!=(const VariantTable& v2) const;

	void Serialize(Serializer& ar);

};
DEFINE_OBJECT_NAME(VariantTable, "table");

class DLLIMPEXP_EWA_BASE CallableTableOperators : public CallableObject
{
public:
	static int __do_setindex(Executor& ewsl,VariantTable& tb,const String& si);
	static int __do_getindex(Executor& ewsl,VariantTable& tb,const String& si);
	static int __do_getarray(Executor& ewsl,VariantTable& tb,int pm);
	static int __do_setarray(Executor& ewsl,VariantTable& tb,int pm);

	static void __do_get_iterator(Executor& ewsl,VariantTable& tb,int nd);
};

class DLLIMPEXP_EWA_BASE CallableTableProxy : public CallableObject
{
public:

	static const int FLAG_READONLY=1<<0;
	static const int FLAG_SET_THIS=1<<1;

	VariantTable& value;
	DataPtrT<ObjectData> parent;

	BitFlags flags;

	CallableTableProxy(VariantTable& v,ObjectData* p=NULL):value(v),parent(p){}

	int __getindex(Executor& ewsl,const String& si);

	int __setindex(Executor& ewsl,const String& si);

	int __getarray(Executor& ewsl,int pm);

	int __setarray(Executor& ewsl,int pm);

	void __get_iterator(Executor& ewsl,int nd);

};

template<>
class DLLIMPEXP_EWA_BASE CallableWrapT<VariantTable> : public CallableDataBaseT<VariantTable>
{
public:
	typedef CallableDataBaseT<VariantTable> basetype;
	CallableWrapT():basetype(){}
	CallableWrapT(const VariantTable& v):basetype(v){}

	virtual CallableWrapT<VariantTable>* ToTable() {return this;}

	virtual CallableData* DoClone(ObjectCloneState& cs)
	{
		if(cs.type==0) return this;
		return new CallableWrapT<VariantTable>(*this);
	}

	int __setindex(Executor& ewsl,const String& si){return CallableTableOperators::__do_setindex(ewsl,value,si);}
	int __getindex(Executor& ewsl,const String& si){return CallableTableOperators::__do_getindex(ewsl,value,si);}
	int __getarray(Executor& ewsl,int pm){return CallableTableOperators::__do_getarray(ewsl,value,pm);}
	int __setarray(Executor& ewsl,int pm){return CallableTableOperators::__do_setarray(ewsl,value,pm);}

	void __get_iterator(Executor& ewsl,int nd){CallableTableOperators::__do_get_iterator(ewsl,value,nd);}

	DECLARE_OBJECT_INFO(CallableWrapT,ObjectInfo);

};


class DLLIMPEXP_EWA_BASE CallableTableEx : public CallableWrapT<VariantTable>
{
public:
	
	virtual CallableData* DoClone(ObjectCloneState& cs)
	{
		if(cs.type==0) return this;
		return new CallableTableEx(*this);
	}

	DECLARE_OBJECT_INFO(CallableTableEx,ObjectInfo);
	
};

class DLLIMPEXP_EWA_BASE CallableTableRo : public CallableTableEx
{
public:

	int __setindex(Executor& ewsl,const String&);
	int __setarray(Executor& ewsl,int pm);

	int __getindex(Executor& ewsl,const String& si);
	int __getarray(Executor& ewsl,int pm);


	virtual CallableData* DoClone(ObjectCloneState&){return this;}

	DECLARE_OBJECT_INFO(CallableTableRo, ObjectInfo);
};


template<>
class hash_t<VariantTable>
{
public:
	inline uint32_t operator()(const VariantTable& o)
	{
		return hash_array<VariantTable::value_type>::hash(o.begin(),o.end());
	}
};

EW_LEAVE

#endif
