#ifndef __H_EW_UTIL_SYMM__
#define __H_EW_UTIL_SYMM__

#include "ewa_base/basic.h"
#include "ewa_base/collection.h"
#include "ewa_base/scripting.h"
#include "ewa_base/math/math_def.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE TableSerializer;
class DLLIMPEXP_EWA_BASE DState;


class DLLIMPEXP_EWA_BASE CallableSymbol : public CallableData
{
public:
	CallableSymbol(const String& s=""):m_sId(s){}

	enum
	{
		FLAG_MIN		=1<<0,
		FLAG_UPDATED	=FLAG_MIN<<0,
		FLAG_ERROR		=FLAG_MIN<<1,
		FLAG_MAX		=FLAG_MIN<<2,
	};

	String m_sId;
	BitFlags flags;

	virtual bool DoUpdateValue(DState&){return true;}
	virtual bool DoTransferData(TableSerializer&){return true;}

};

class DLLIMPEXP_EWA_BASE SymbolItem
{
public:
	SymbolItem(const String& n = "");

	String name;
	arr_1t<Variant> prop;

	void Serialize(Serializer& ar);
};


template<>
class hash_t<SymbolItem>
{
public:
	uint32_t operator()(const SymbolItem& item)
	{
		return -1;
	}
};

DEFINE_OBJECT_NAME(SymbolItem,"SymbolItem")

inline bool operator==(const SymbolItem& lhs,const SymbolItem& rhs)
{
	return lhs.name==rhs.name && lhs.prop==rhs.prop;
}
inline bool operator!=(const SymbolItem& lhs,const SymbolItem& rhs)
{
	return !(lhs==rhs);
}

template<>
class DLLIMPEXP_EWA_BASE CallableWrapT<arr_1t<SymbolItem> > : public CallableDataBaseT<arr_1t<SymbolItem> >
{
public:
	typedef arr_1t<SymbolItem> type;
	typedef CallableDataBaseT<type> basetype;

	CallableWrapT():basetype(){}
	CallableWrapT(const type& v):basetype(v){}
	virtual CallableData* DoClone(ObjectCloneState&){return new CallableWrapT(value);}
};

class DLLIMPEXP_EWA_BASE SymbolManager
{
protected:

	class ObjState
	{
	public:
		ObjState(const String& n="",const String& t=""):name(n),type(t){}

		String name;
		String type;
		DataPtrT<CallableSymbol> symptr;
		VariantTable value;
	};


	indexer_map<String,DataPtrT<CallableSymbol> > m_aSymbol;
	arr_1t<ObjState> m_aStack;

public:

	void swap(SymbolManager& o)
	{
		m_aSymbol.swap(o.m_aSymbol);
	}

	size_t depth(){return m_aStack.size();}

	
	size_t size(){return m_aSymbol.size();}

	CallableSymbol* get_item(size_t i)
	{
		if(i>=m_aSymbol.size()) return NULL;
		return m_aSymbol.get(i).second.get();
	}

	ObjectCreator factory;

	SymbolManager(){}

	bool UpdateValue();
	bool DoUpdateValue(DState& ds);

	void gp_add(const String& s)
	{
		m_aStack.back().value["#children"].ref<arr_1t<SymbolItem> >().push_back(s);
	}


	void gp_add(const String& s,double f)
	{
		SymbolItem item(s);
		item.prop.resize(1);
		item.prop[0].reset(f);

		m_aStack.back().value["#children"].ref<arr_1t<SymbolItem> >().push_back(item);
	}

	template<typename T>
	void gp_add_t(const String& s,const T& item)
	{
		m_aStack.back().value[s].ref<arr_1t<T> >().push_back(item);
	}

	void gp_set(const String& s,const String& v)
	{
		m_aStack.back().value[s].reset(v);
	}

	void gp_set(const String& s,const Variant& v)
	{
		m_aStack.back().value[s]=v;
	}

	void append(CallableSymbol* p)
	{
		if(!p) return;
		String s=p->m_sId;
		m_aSymbol[s].reset(p);
	}

	void gp_add(CallableSymbol* p)
	{
		if(!p) return;
		append(p);
		gp_add(p->m_sId);
	}

	void gp_beg(const String& s,const String& t="");
	void gp_end();

	void append(const String& s,const String& t)
	{
		gp_beg(s,t);
		gp_end();
	}

	CallableData* get_item(const String& s)
	{
		int id=m_aSymbol.find1(s);
		if(id<0) return NULL;
		return m_aSymbol.get(id).second.get();
	}

	template<typename T>
	T* get_item_t(const String& s)
	{
		return dynamic_cast<T*>(get_item(s));
	}
	template<typename T>
	T* get_item_t(size_t i)
	{
		return dynamic_cast<T*>(get_item(i));
	}

};

template<typename T>
class NamedReferenceT : public DataPtrT<T>
{
public:
	String name;
};

class DLLIMPEXP_EWA_BASE TableSerializer : public Object
{
public:

	enum
	{
		READER,
		WRITER,
	};


	bool is_reader(){return type==READER;}
	bool is_writer(){return type==WRITER;}

	SymbolManager& smap;
	VariantTable& value;

	const int type;

	TableSerializer(int t,SymbolManager& m,VariantTable& v);

	void link(const String& s,double& v);
	void link(const String& s,int& v);
	void link(const String& s,BitFlags& v,int m);

	void link(const String& s,String& v);

	template<typename T>
	void link_t(const String& s,arr_1t<T>& v)
	{
		arr_1t<T> &a(value[s].ref<arr_1t<T> >());
		if(is_reader())
		{
			v=a;
		}
		else
		{
			a=v;
		}
	}

};

class DLLIMPEXP_EWA_BASE TableSerializerReader : public TableSerializer
{
public:
	TableSerializerReader(SymbolManager& m,VariantTable& v):TableSerializer(READER,m,v){}
};

class DLLIMPEXP_EWA_BASE TableSerializerWriter : public TableSerializer
{
public:
	TableSerializerWriter(SymbolManager& m,VariantTable& v):TableSerializer(WRITER,m,v){}
};

class DLLIMPEXP_EWA_BASE DMatrixBox
{
public:

	DMatrixBox()
	{
		m4.LoadIdentity();
		b3.load_min();
	}

	DMatrixBox(const mat4d& m):m4(m)
	{
		b3.load_min();
	}


	mat4d m4;
	box3d b3;
};

class DLLIMPEXP_EWA_BASE DState : public Object
{
public:

	DState(Executor& k,SymbolManager* s=NULL);


	LitePtrT<SymbolManager> psmap;
	Executor& lexer;
	BitFlags flags;

	arr_1t<SymbolManager*> asmap;

	enum
	{
		FLAG_POST	=1<<0,
	};

	int phase;

	indexer_map<void*,int> tested;

	bool link(const String& s,int &v);
	bool link(const String& s,double &v);

	bool link(const vec3s& s,vec3i& v);
	bool link(const vec2s& s,vec2d& v);
	bool link(const vec3s& s,vec3d& v);
	bool link(const box3s& s,box3d& v);

	bool link(const String& s,DataPtrT<CallableSymbol>& v);

	template<typename T>
	bool link_t(const String& s,DataPtrT<T>& v)
	{
		DataPtrT<CallableSymbol> h;
		if(!link(s,h)) return false;
		v.reset(dynamic_cast<T*>(h.get()));
		return v;
	}

	template<typename T>
	bool link_t(NamedReferenceT<T>& p)
	{
		return link_t<T>(p.name,p);
	}

	template<typename T>
	bool DoUpdateValue(NamedReferenceT<T>& p)
	{
		if((phase==1 && !link_t<T>(p.name,p))||!p)
		{
			return false;
		}

		if(test(p.get()) && !p->DoUpdateValue(*this))
		{
			return false;
		}

		return true;
	}

	bool test(ObjectData* p)
	{
		int &v(tested[p]);
		if(v<phase)
		{
			v=phase;
			return true;
		}
		else
		{
			return false;
		}
	}

	class LockerSM
	{
	public:
		DState& ds;
		LockerSM(DState& d_,SymbolManager& s_):ds(d_)
		{
			ds.asmap.push_back(&s_);
			ds.psmap=ds.asmap.back();
		}
		~LockerSM()
		{
			ds.asmap.pop_back();
			ds.psmap=ds.asmap.empty()?NULL:ds.asmap.back();
		}
	};
};



EW_LEAVE
#endif
