#include "ewa_base/scripting.h"
#include "ewa_base/util/strlib.h"
EW_ENTER



template<>
CallableMetatableT<String>::CallableMetatableT()
:CallableMetatable("string")
{
	CallableData::sm_meta[type_flag<type>::value]=this;
}

template<>
int CallableMetatableT<String>::__fun_call(Executor& ewsl, int pm)
{
	typedef String type;

	if (pm == 0)
	{
		ewsl.ci0.nbx[1].reset<type>();
		return 1;
	}
	else if (pm == 1)
	{
		ewsl.ci0.nbx[1].reset<type>(variant_cast<type>(ewsl.ci0.nbx[1]));
		return 1;
	}

	return CallableData::__fun_call(ewsl, pm);

}
template class CallableMetatableT<String>;

struct pl_string_length
{
	static inline void g(Variant& r, const String& v)
	{
		StringBuffer<wchar_t> rhs(v);
		r.reset(rhs.size());
	}
};

struct pl_string_lower
{
	static inline void g(Variant& r, const String& v)
	{
		r.reset(ew::string_to_lower(v));
	}
};

struct pl_string_upper
{
	static inline void g(Variant& r, const String& v)
	{
		r.reset(ew::string_to_upper(v));
	}
};

struct pl_string_reverse
{
	static inline void g(Variant& r, const String& v)
	{
		StringBuffer<wchar_t> rhs(v);
		std::reverse(rhs.begin(), rhs.end());
		r.reset(String(rhs));
	}
};

template<typename P>
class CallableFunctionStringBaseT : public CallableFunction
{
public:
	CallableFunctionStringBaseT(const String &n) :CallableFunction(n){}
	static int do_apply(Executor& ewsl, Variant& v)
	{
		String s0=variant_cast<String>(v);
		P::g(ewsl.ci0.nbx[1], s0);
		return 1;
	}
};

template<typename P>
class CallableFunctionStringT : public CallableFunctionStringBaseT<P>
{
public:
	typedef CallableFunctionStringBaseT<P> basetype;
	CallableFunctionStringT(const String &n) :basetype(n){}
	int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this, pm, 1);
		return do_apply(ewsl, ewsl.ci0.nbx[1]);
	}
};


template<typename P>
class CallableFunctionStringThisT : public CallableFunctionStringBaseT<P>
{
public:
	typedef CallableFunctionStringBaseT<P> basetype;

	CallableFunctionStringThisT(const String &n) :basetype(n){}
	int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this, pm, 0);
		return basetype::do_apply(ewsl, ewsl.ci1.nbp[StackState1::SBASE_THIS]);
	}
};


class CallableFunctionStringLength : public CallableFunctionStringThisT<pl_string_length>
{
public:
	CallableFunctionStringLength() :CallableFunctionStringThisT<pl_string_length>("string.length"){}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStringLength, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStringLength, ObjectInfo);

class CallableFunctionStringFind : public CallableFunction
{
public:
	CallableFunctionStringFind() :CallableFunction("string.find"){}

	int __fun_call(Executor& ewsl, int pm)
	{
		
		String* pstr=ewsl.ci1.nbp[StackState1::SBASE_THIS].ptr<String>();
		if(pm==0||!pstr)
		{
			ewsl.ci0.nbx[1].clear();
			return 1;
		}

		String what=variant_cast<String>(ewsl.ci0.nbx[1]);
		int pos=0;
		if(pm>1)
		{
			pos=variant_cast<int>(ewsl.ci0.nbx[2]);
		}
		ewsl.ci0.nbx[1].reset(pstr->find(what,pos));
		return 1;
	}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStringFind, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStringFind, ObjectInfo);

class CallableFunctionStringLower : public CallableFunctionStringThisT<pl_string_lower>
{
public:
	CallableFunctionStringLower() :CallableFunctionStringThisT<pl_string_lower>("string.lower"){}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStringLower, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStringLower, ObjectInfo);

class CallableFunctionLower : public CallableFunctionStringT<pl_string_lower>
{
public:
	CallableFunctionLower() :CallableFunctionStringT<pl_string_lower>("lower"){}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionLower, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionLower, ObjectInfo);

class CallableFunctionStringUpper : public CallableFunctionStringThisT<pl_string_upper>
{
public:
	CallableFunctionStringUpper() :CallableFunctionStringThisT<pl_string_upper>("string.upper"){}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStringUpper, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStringUpper, ObjectInfo);

class CallableFunctionUpper : public CallableFunctionStringT<pl_string_upper>
{
public:
	CallableFunctionUpper() :CallableFunctionStringT<pl_string_upper>("upper"){}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionUpper, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionUpper, ObjectInfo);

class CallableFunctionStringReverse : public CallableFunctionStringThisT<pl_string_reverse>
{
public:
	CallableFunctionStringReverse() :CallableFunctionStringThisT<pl_string_reverse>("string.reverse"){}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStringReverse, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStringReverse, ObjectInfo);

class CallableFunctionStringSubstr : public CallableFunction
{
public:

	CallableFunctionStringSubstr(const String& name="string.substr"):CallableFunction(name){}

	int do_apply(Executor& ewsl,const String& v,size_t p1,size_t p2)
	{
		String s=variant_cast<String>(v);
		StringBuffer<wchar_t> rhs(s);

		if(p2>rhs.size()) p2=rhs.size();
		if(p1>=p2)
		{
			ewsl.ci0.nbx[1].reset("");
			return 1;
		}
		else
		{
			ewsl.ci0.nbx[1].reset(String(rhs.data()+p1,rhs.data()+p2));
			return 1;
		}
	}

	int __fun_call(Executor& ewsl,int pm)
	{
		String s0=variant_cast<String>(ewsl.ci1.nbp[StackState1::SBASE_THIS]);
		if (pm == 0)
		{
			ewsl.push(s0);
			return 0;
		}
		size_t p1=variant_cast<size_t>(ewsl.ci0.nbx[1]);
		size_t p2=pm>1?variant_cast<size_t>(ewsl.ci0.nbx[2]):-1;
		return do_apply(ewsl,s0,p1,p2);

	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStringSubstr, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStringSubstr, ObjectInfo);


class CallableFunctionSubstr : public CallableFunctionStringSubstr
{
public:

	CallableFunctionSubstr():CallableFunctionStringSubstr("substr"){}

	int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1,3);
		if(pm==1) return 1;
		String s0=variant_cast<String>(ewsl.ci0.nbx[1]);
		size_t p1=variant_cast<size_t>(ewsl.ci0.nbx[2]);
		size_t p2=pm>2?variant_cast<size_t>(ewsl.ci0.nbx[2]):-1;
		return do_apply(ewsl,s0,p1,p2);
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionSubstr, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionSubstr, ObjectInfo);


class CallableFunctionStringSplit : public CallableFunction
{
public:

	CallableFunctionStringSplit(const String& name="string.split"):CallableFunction(name){}

	int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		String s0=variant_cast<String>(ewsl.ci1.nbp[StackState1::SBASE_THIS]);
		String s1=variant_cast<String>(ewsl.ci0.nbx[1]);
		return do_apply(ewsl,s0,s1);

	}

	int do_apply(Executor& ewsl,const String& s0,const String& s1)
	{
		arr_1t<String> _aStrings;
		_aStrings=string_split(s0,s1);
		arr_xt<Variant> _aVariants;
		_aVariants.resize(_aStrings.size());
		std::transform(_aStrings.begin(),_aStrings.end(),_aVariants.begin(),[](const String& s){Variant v;v.reset(s);return v;});				
		ewsl.ci0.nbx[1].reset(_aVariants);
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStringSplit, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStringSplit, ObjectInfo);


class CallableFunctionSplit : public CallableFunctionStringSplit
{
public:

	CallableFunctionSplit():CallableFunctionStringSplit("split"){}

	int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,2);
		String s0=variant_cast<String>(ewsl.ci0.nbx[1]);
		String s1=variant_cast<String>(ewsl.ci0.nbx[2]);
		return do_apply(ewsl,s0,s1);
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionSplit, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionSplit, ObjectInfo);


class CallableFunctionStringReplace : public CallableFunction
{
public:

	CallableFunctionStringReplace(const String& name="string.replace"):CallableFunction(name){}

	int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,2);
		String s0=variant_cast<String>(ewsl.ci1.nbp[StackState1::SBASE_THIS]);
		String s1=variant_cast<String>(ewsl.ci0.nbx[1]);
		String s2=variant_cast<String>(ewsl.ci0.nbx[2]);
		return do_apply(ewsl,s0,s1,s2);
	}

	int do_apply(Executor& ewsl,const String& s0,const String& s1,const String& s2)
	{
		ewsl.ci0.nbx[1].reset(string_replace(s0,s1,s2));
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStringReplace, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStringReplace, ObjectInfo);


class CallableFunctionReplace : public CallableFunctionStringReplace
{
public:

	CallableFunctionReplace():CallableFunctionStringReplace("replace"){}

	int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,3);
		String s0=variant_cast<String>(ewsl.ci0.nbx[1]);
		String s1=variant_cast<String>(ewsl.ci0.nbx[2]);
		String s2=variant_cast<String>(ewsl.ci0.nbx[3]);
		return do_apply(ewsl,s0,s1,s2);
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionReplace, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionReplace, ObjectInfo);


class CallableFunctionStringFormat : public CallableFunction
{
public:
	CallableFunctionStringFormat() :CallableFunction("string.format",1){}
	int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.ci0.nbx[1].reset(string_format_variants(ewsl.ci0.nbx + 1, pm));
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStringFormat, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStringFormat, ObjectInfo);


class CallableFunctionStringConcat : public CallableFunction
{
public:
	CallableFunctionStringConcat(const String& name="string.concat") :CallableFunction(name,0){}

	int do_apply(Executor& ewsl,const String& w,Variant* p,int n)
	{
		StringBuffer<char> res(w);
		for (int i = 0; i < n; i++)
		{
			res << pl_cast<String>::g(p[i]);
		}
		ewsl.ci0.nbx[1].reset(res.c_str());
		return 1;
	}

	int __fun_call(Executor& ewsl, int pm)
	{
		if(pm==0) ewsl.kerror("invalid param");
		String s0=variant_cast<String>(ewsl.ci1.nbp[StackState1::SBASE_THIS]);
		return do_apply(ewsl,s0,ewsl.ci0.nbx+1,pm);
	}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStringConcat, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStringConcat, ObjectInfo);


class CallableFunctionConcat : public CallableFunctionStringConcat
{
public:

	CallableFunctionConcat():CallableFunctionStringConcat("concat"){}

	int __fun_call(Executor& ewsl,int pm)
	{
		return do_apply(ewsl,"",ewsl.ci0.nbx+1,pm);
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionConcat, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionConcat, ObjectInfo);

void init_module_string()
{
	CG_GGVar &gi(CG_GGVar::current());
	gi.add_inner<CallableMetatableT<String> >();
	gi.add_inner<CallableMetatableT<StringBuffer<char> > >();

	gi.add_inner<CallableMetatableT<Stream> >();

	gi.add_inner<CallableFunctionStringLength>();
	gi.add_inner<CallableFunctionStringUpper>();
	gi.add_inner<CallableFunctionStringLower>();
	gi.add_inner<CallableFunctionStringReverse>();
	gi.add_inner<CallableFunctionStringSubstr>();

	gi.add_inner<CallableFunctionStringFormat>();
	gi.add_inner<CallableFunctionStringConcat>();
	gi.add_inner<CallableFunctionStringSplit>();
	gi.add_inner<CallableFunctionStringReplace>();

	gi.add_inner<CallableFunctionStringFind>();


	gi.add_inner<CallableFunctionUpper>();
	gi.add_inner<CallableFunctionLower>();
	gi.add_inner<CallableFunctionSubstr>();
	gi.add_inner<CallableFunctionConcat>();
	gi.add_inner<CallableFunctionSplit>();
	gi.add_inner<CallableFunctionReplace>();

}


EW_LEAVE
