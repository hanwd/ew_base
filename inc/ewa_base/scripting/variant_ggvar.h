#ifndef __H_EW_SCRIPTING_VARIANT_GGVAR__
#define __H_EW_SCRIPTING_VARIANT_GGVAR__

#include "ewa_base/scripting/callable_table.h"


EW_ENTER

class DLLIMPEXP_EWA_BASE CG_GGVar : public VariantTable
{
public:

	void add(CallableData* p,const String& s,int f=0);
	void add(CallableFunction* p);
	void add(CallableMetatable* p);

	CG_Variable* find_var(const String& s);

	void import(const String& lib,bool reload=false);
	void expand(const String& lib);

	void import(CallableMetatable* q);
	void unload(const String& lib);

	template<typename T>
	void add_inner();

	template<typename T>
	void add_inner(const String& s,int f=0);

	static CG_GGVar& current();

private:

	CG_GGVar();
	CG_GGVar(const CG_GGVar&);
	~CG_GGVar();

	indexer_map<String, CG_Variable*> sm_vmap;

	bool _bInited;

	void _init();
};


template<typename T>
inline void CG_GGVar::add_inner()
{
	add(T::sm_info.GetCachedInstance());
}

template<typename T>
inline void CG_GGVar::add_inner(const String& s,int f)
{
	add(T::sm_info.GetCachedInstance(), s, f);
}

EW_LEAVE
#endif
