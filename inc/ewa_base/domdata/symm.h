#ifndef __H_EW_DOMDATA_SYMM__
#define __H_EW_DOMDATA_SYMM__

#include "ewa_base/basic.h"
#include "ewa_base/collection.h"
#include "ewa_base/scripting.h"

#include "ewa_base/domdata/dobject.h"


EW_ENTER


class DLLIMPEXP_EWA_BASE SymbolManager
{
protected:

	class ObjState
	{
	public:
		ObjState(const String& n="",const String& t=""):name(n),type(t){}

		String name;
		String type;
		DataPtrT<DObject> symptr;
		VariantTable value;
	};


	indexer_map<String,DataPtrT<DObject> > m_aSymbol;
	arr_1t<ObjState> m_aStack;

public:

	void swap(SymbolManager& o)
	{
		m_aSymbol.swap(o.m_aSymbol);
	}

	size_t depth(){return m_aStack.size();}


	size_t size(){return m_aSymbol.size();}

	DObject* get_item(size_t i)
	{
		if(i>=m_aSymbol.size()) return NULL;
		return m_aSymbol.get(i).second.get();
	}

	ObjectCreator factory;

	SymbolManager(){}

	bool UpdateValue();
	bool DoUpdateValue(DState& dp);
	bool DoUpdateValue(DState& dp,const String& name);
	

	void gp_add(const String& s)
	{
		m_aStack.back().value["#children"].ref<arr_1t<SymbolItem> >().push_back(s);
	}

/*
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
*/
	void gp_set(const String& s,const String& v)
	{
		m_aStack.back().value[s].reset(v);
	}

	void gp_set(const String& s,const Variant& v)
	{
		m_aStack.back().value[s]=v;
	}

	Variant& gp_get(const String& s)
	{
		return m_aStack.back().value[s];
	}

	void append(DObject* p)
	{
		if(!p) return;
		String s=p->m_sId;
		m_aSymbol[s].reset(p);
	}

	void gp_add(DObject* p)
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



EW_LEAVE
#endif
