
#ifndef __H_EW_DOMDATA_DSTATE__
#define __H_EW_DOMDATA_DSTATE__


#include "ewa_base/basic.h"
#include "ewa_base/domdata/dobject.h"


EW_ENTER


class DLLIMPEXP_EWA_BASE SymbolManager;

enum
{
	DPHASE_NIL,
	DPHASE_VAR,
	DPHASE_SHP,
	DPHASE_ACT,
	DPHASE_VAL,

	DPHASE_MIN=DPHASE_VAR,
	DPHASE_MAX=DPHASE_VAL,
};

class DLLIMPEXP_EWA_BASE DState : public Object
{
public:

	DState(Executor& k, SymbolManager* s = NULL);

	LitePtrT<SymbolManager> psmap;
	arr_1t<SymbolManager*> asmap;

	Executor& lexer;
	BitFlags flags;

	enum
	{
		FLAG_POST = 1 << 0,
	};

	int phase;

	indexer_map<void*, int> tested;

	bool link(const String& s, int &v);
	bool link(const String& s, double &v);

	bool link(const vec3s& s, vec3i& v);
	bool link(const vec2s& s, vec2d& v);
	bool link(const vec3s& s, vec3d& v);
	bool link(const box3s& s, box3d& v);
	bool link(const box2s& s, box2d& v);

	bool link(const String& s, DataPtrT<DObject>& v);

	template<typename T>
	bool link_t(const String& s, DataPtrT<T>& v)
	{
		DataPtrT<DObject> h;
		if (!link(s, h)) return false;
		v.reset(dynamic_cast<T*>(h.get()));
		return v;
	}

	template<typename T>
	bool link_t(NamedReferenceT<T>& p)
	{
		return link_t<T>(p.name, p);
	}

	template<typename T1, typename T2>
	bool link_t(const arr_1t<T1>& s, arr_1t<T2>& v)
	{
		bool flag = true;
		for (int i = 0; i < int(s.size()); i++)
			flag && link(s[i], v[i]);
		return flag;
	}

	template<typename T>
	bool DoUpdateValue(NamedReferenceT<T>& p)
	{
		if(phase==DPHASE_VAR)
		{
			if(!link_t<T>(p.name,p))
			{
				return false;
			}
			if(!p)
			{
				return false;
			}
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
