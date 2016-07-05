#include "ewa_base/scripting.h"
#include "ewa_base/util/regex.h"
#include <limits>

EW_ENTER

template<>
class CallableWrapT<Regex> : public CallableObject
{
public:

	int __getindex(Executor&,const String&);

	Regex value;
	String sexpr;

	virtual bool ToValue(String& s,int n=0) const
	{
		s="regex("+sexpr+")";
		return true;
	}

	DECLARE_OBJECT_INFO(CallableWrapT<Regex>,ObjectInfo)
};

IMPLEMENT_OBJECT_INFO(CallableWrapT<Regex>,ObjectInfo)


template<>
class CallableWrapT<RegexEx> : public CallableObject
{
public:

	int __getindex(Executor&,const String&);

	RegexEx value;
	String sexpr;

	virtual bool ToValue(String& s,int n=0) const
	{
		s="regex_ex("+sexpr+")";
		return true;
	}

	DECLARE_OBJECT_INFO(CallableWrapT<RegexEx>,ObjectInfo)
};

IMPLEMENT_OBJECT_INFO(CallableWrapT<RegexEx>,ObjectInfo)

template<>
class CallableWrapT<Match> : public CallableObject
{
public:

	int __getarray(Executor& ewsl,int n)
	{
		ewsl.check_pmc(this,n,1,2);
		if(n==1)
		{
			unsigned nd=variant_cast<unsigned>(ewsl.ci0.nbx[1]);
			if(nd>=value.size())
			{
				ewsl.kerror("invalid result");
			}
			ewsl.ci0.nbx[1].reset(String(value[nd]));		
		}
		else if(n==2)
		{
			unsigned n1=variant_cast<unsigned>(ewsl.ci0.nbx[1]);
			unsigned n2=variant_cast<unsigned>(ewsl.ci0.nbx[2]);
			if(n1>=value.size()||n2>=value[n1].size())
			{
				ewsl.kerror("invalid result");
			}
			ewsl.ci0.nbx[1].reset(String(value[n1][n2]));			
		
		}

		return 1;
	}

	virtual bool ToValue(String& s,int n=0) const
	{
		s="match("+String::Format("size:%d",(int)value.size())+")";
		return true;
	}

	DECLARE_OBJECT_INFO(CallableWrapT<Match>,ObjectInfo)

	Match value;
};

IMPLEMENT_OBJECT_INFO(CallableWrapT<Match>,ObjectInfo)

class CallableRegexFunction : public CallableFunction
{
public:

	Regex& get_regex(Executor& ewsl)
	{
		CallableWrapT<Regex>* p=dynamic_cast<CallableWrapT<Regex>*>(ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr());
		if(!p)
		{
			return  get_regex_ex(ewsl);
		}
		return p->value;
	}

	RegexEx& get_regex_ex(Executor& ewsl)
	{
		CallableWrapT<RegexEx>* p=dynamic_cast<CallableWrapT<RegexEx>*>(ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr());
		if(!p) ewsl.kerror("invalid regex");
		return p->value;
	}
};



class CallableRegexMatch : public CallableRegexFunction
{
public:

	int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		String str=variant_cast<String>(ewsl.ci0.nbx[1]);
		Regex& re=get_regex(ewsl);
		DataPtrT<CallableWrapT<Match> > pres(new CallableWrapT<Match>);
		bool flag=re.match(str,pres->value);
		if(flag)
		{
			ewsl.ci0.nbx[1].reset(flag);
			ewsl.ci0.nbx[2].reset(pres);
			return 2;
		}
		else
		{
			ewsl.ci0.nbx[1].reset(flag);
			return 1;
		}
	}

	DECLARE_OBJECT_CACHED_INFO(CallableRegexMatch,ObjectInfo)
};

IMPLEMENT_OBJECT_INFO(CallableRegexMatch,ObjectInfo)

class CallableRegexSearch : public CallableRegexFunction
{
public:

	int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		String str=variant_cast<String>(ewsl.ci0.nbx[1]);
		Regex& re=get_regex(ewsl);
		DataPtrT<CallableWrapT<Match> > pres(new CallableWrapT<Match> );
		bool flag=re.search(str,pres->value);
		if(flag)
		{
			ewsl.ci0.nbx[1].reset(flag);
			ewsl.ci0.nbx[2].reset(pres);
			return 2;
		}
		else
		{
			ewsl.ci0.nbx[1].reset(flag);
			return 1;
		}
	}
	
	DECLARE_OBJECT_CACHED_INFO(CallableRegexSearch,ObjectInfo)
};

IMPLEMENT_OBJECT_INFO(CallableRegexSearch,ObjectInfo)

class CallableRegexReplace : public CallableRegexFunction
{
public:

	int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,2);
		String str1=variant_cast<String>(ewsl.ci0.nbx[1]);
		String str2=variant_cast<String>(ewsl.ci0.nbx[2]);
		Match res;
		Regex& re=get_regex(ewsl);
		re.search(str1,res);

		ewsl.ci0.nbx[1].reset(res.replace(str2));

		return 1;
	}	

	DECLARE_OBJECT_CACHED_INFO(CallableRegexReplace,ObjectInfo)
};

IMPLEMENT_OBJECT_INFO(CallableRegexReplace,ObjectInfo)


class CallableRegexPrepare : public CallableRegexFunction
{
public:

	int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,2);
		String str1=variant_cast<String>(ewsl.ci0.nbx[1]);
		String str2=variant_cast<String>(ewsl.ci0.nbx[2]);

		RegexEx& re=get_regex_ex(ewsl);
		bool flag=re.prepare(str1,str2);

		ewsl.ci0.nbx[1].reset(flag);

		return 1;
	}	

	DECLARE_OBJECT_CACHED_INFO(CallableRegexPrepare,ObjectInfo)
};

IMPLEMENT_OBJECT_INFO(CallableRegexPrepare,ObjectInfo)


int CallableWrapT<Regex>::__getindex(Executor& ewsl,const String& id)
{
	if(id=="match")
	{
		(*ewsl.ci1.nsp).reset(CallableRegexMatch::sm_info.GetCachedInstance());
	}
	else if(id=="search")
	{
		(*ewsl.ci1.nsp).reset(CallableRegexSearch::sm_info.GetCachedInstance());	
	}
	else if(id=="replace")
	{
		(*ewsl.ci1.nsp).reset(CallableRegexReplace::sm_info.GetCachedInstance());		
	}
	else
	{
		ewsl.kerror("invalid index");
	}
	
	return STACK_BALANCED;
}


class CallableRegexCompile2 : public CallableRegexFunction
{
public:

	int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1,2);
		Regex& re=get_regex(ewsl);
		String sexpr=variant_cast<String>(ewsl.ci0.nbx[1]);
		int flag=0;
		if(pm==2) flag=variant_cast<int>(ewsl.ci0.nbx[2]);
		bool bflag=re.assign(sexpr,flag);
		ewsl.ci0.nbx[1].reset(bflag);
		return 1;
	}	

	DECLARE_OBJECT_CACHED_INFO(CallableRegexCompile2,ObjectInfo)
};

IMPLEMENT_OBJECT_INFO(CallableRegexCompile2,ObjectInfo)

int CallableWrapT<RegexEx>::__getindex(Executor& ewsl,const String& id)
{
	if(id=="match")
	{
		(*ewsl.ci1.nsp).reset(CallableRegexMatch::sm_info.GetCachedInstance());
	}
	else if(id=="prepare")
	{
		(*ewsl.ci1.nsp).reset(CallableRegexPrepare::sm_info.GetCachedInstance());
	}
	else if(id=="search")
	{
		(*ewsl.ci1.nsp).reset(CallableRegexSearch::sm_info.GetCachedInstance());	
	}
	else if(id=="replace")
	{
		(*ewsl.ci1.nsp).reset(CallableRegexReplace::sm_info.GetCachedInstance());		
	}
	else if(id=="compile")
	{
		(*ewsl.ci1.nsp).reset(CallableRegexCompile2::sm_info.GetCachedInstance());		
	}
	else
	{
		ewsl.kerror("invalid index");
	}
	
	return STACK_BALANCED;
}


class CallableRegexCompile : public CallableFunction
{
public:

	int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1,2);

		DataPtrT<CallableWrapT<Regex> > preg(new CallableWrapT<Regex> );
		preg->sexpr=variant_cast<String>(ewsl.ci0.nbx[1]);

		int flag=0;
		if(pm==2) flag=variant_cast<int>(ewsl.ci0.nbx[2]);

		preg->value.assign(preg->sexpr,flag);
		ewsl.ci0.nbx[1].reset(preg);
		return 1;
	}	

	DECLARE_OBJECT_CACHED_INFO(CallableRegexCompile,ObjectInfo)
};

IMPLEMENT_OBJECT_INFO(CallableRegexCompile,ObjectInfo)


class CallableRegexCreate : public CallableFunction
{
public:

	int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,0);
		DataPtrT<CallableWrapT<RegexEx> > preg(new CallableWrapT<RegexEx> );
		ewsl.ci0.nbx[1].reset(preg);
		return 1;
	}	

	DECLARE_OBJECT_CACHED_INFO(CallableRegexCreate,ObjectInfo)
};

IMPLEMENT_OBJECT_INFO(CallableRegexCreate,ObjectInfo)


class CallableMetatableRegex : public CallableMetatable
{
public:

	CallableMetatableRegex()
	{
		value["compile"].reset(CallableRegexCompile::sm_info.GetCachedInstance());
		value["create"].reset(CallableRegexCreate::sm_info.GetCachedInstance());

		value["FLAG_RE_IGNORECASE"].reset(Regex::FLAG_RE_IGNORECASE);
		value["FLAG_RE_UNICODE"].reset(Regex::FLAG_RE_UNICODE);
		value["FLAG_RE_MULTILINE"].reset(Regex::FLAG_RE_MULTILINE);
		value["FLAG_RE_DOTALL"].reset(Regex::FLAG_RE_DOTALL);
		value["FLAG_RE_PARTITIAL"].reset(Regex::FLAG_RE_PARTITIAL);
	}

	DECLARE_OBJECT_CACHED_INFO(CallableMetatableRegex,ObjectInfo)
};

IMPLEMENT_OBJECT_INFO(CallableMetatableRegex,ObjectInfo)

void init_module_regex()
{
	CG_GGVar& gi(CG_GGVar::current());
	gi.add(new CallableMetatableRegex,"regex");


}

EW_LEAVE
