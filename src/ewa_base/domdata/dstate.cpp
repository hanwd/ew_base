
#include "ewa_base/domdata/symm.h"
#include "ewa_base/domdata/dstate.h"
#include "ewa_base/scripting/executor.h"

EW_ENTER


DState::DState(Executor& k,SymbolManager* s)
	:lexer(k)
	,phase(1)
{
	if(s) asmap.push_back(s);
}

bool DState::link(const String& s,int &v)
{
	return lexer.eval(s,v);
}

bool DState::link(const String& s,double &v)
{
	return lexer.eval(s,v);
}

bool DState::link(const vec3s& s,vec3d& v)
{
	return link(s[0],v[0]) && link(s[1],v[1]) && link(s[2],v[2]);
}

bool DState::link(const vec3s& s,vec3i& v)
{
	return link(s[0],v[0]) && link(s[1],v[1]) && link(s[2],v[2]);
}

bool DState::link(const vec2s& s, vec2d& v)
{
	return link(s[0],v[0]) && link(s[1],v[1]);
}

bool DState::link(const vec2s& s, vec2i& v)
{
	return link(s[0],v[0]) && link(s[1],v[1]);
}

bool DState::link(const box3s& s, box3d& v)
{
	return link(s.lo,v.lo) && link(s.hi,v.hi);
}

bool DState::link(const box3s& s, box3i& v)
{
	return link(s.lo, v.lo) && link(s.hi, v.hi);
}


bool DState::link(const box2s& s, box2d& v)
{
	return link(s.lo, v.lo) && link(s.hi, v.hi);
}

bool DState::link(const box2s& s, box2i& v)
{
	return link(s.lo, v.lo) && link(s.hi, v.hi);
}

bool DState::link(const String& s, arr_1t<double>& v)
{
	return lexer.eval(s, v);
}

bool DState::link(const String& s, arr_xt<double>& v)
{
	return lexer.eval(s, v);
}

bool DState::link(const String& s,DataPtrT<DObject>& v)
{
	String h(s);
	if(s.c_str()[0]=='@')
	{
		if(!lexer.eval(s.c_str()+1,h))
		{
			v.reset(NULL);
			return false;
		}
	}

	for(arr_1t<SymbolManager*>::reverse_iterator it=asmap.rbegin();it!=asmap.rend();++it)
	{
		v.reset((*it)->get_item_t<DObject>(h));
		if(v) return true;
	}

	return v;
}


EW_LEAVE
