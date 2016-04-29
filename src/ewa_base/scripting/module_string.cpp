#include "ewa_base/scripting.h"
#include "ewa_base/util/strlib.h"
EW_ENTER



template<>
CallableMetatableT<String>::CallableMetatableT()
:CallableMetatable("string")
{
	CG_GGVar::current().sm_meta[type_flag<type>::value].reset(this);
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
		r.reset<String>(rhs);
	}
};

template<typename P>
class CallableFunctionStringT : public CallableFunction
{
public:

	CallableFunctionStringT(const String &n) :CallableFunction(n){}

	static int do_apply(Executor& ewsl, Variant& v)
	{
		String* _pval = v.ptr<String>();
		if (!_pval)
		{
			ewsl.kerror("invalid param");
		}
		P::g(ewsl.ci0.nbx[1], *_pval);
		return 1;
	}

	int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this, pm, 0);
		return do_apply(ewsl, ewsl.ci1.nbp[StackState1::SBASE_THIS]);
	}
};


class CallableFunctionStringLength : public CallableFunctionStringT<pl_string_length>
{
public:
	CallableFunctionStringLength() :CallableFunctionStringT<pl_string_length>("string.length"){}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStringLength, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStringLength, ObjectInfo);

class CallableFunctionStringLower : public CallableFunctionStringT<pl_string_lower>
{
public:
	CallableFunctionStringLower() :CallableFunctionStringT<pl_string_lower>("string.lower"){}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStringLower, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStringLower, ObjectInfo);

class CallableFunctionStringUpper : public CallableFunctionStringT<pl_string_upper>
{
public:
	CallableFunctionStringUpper() :CallableFunctionStringT<pl_string_upper>("string.upper"){}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStringUpper, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStringUpper, ObjectInfo);

class CallableFunctionStringReverse : public CallableFunctionStringT<pl_string_reverse>
{
public:
	CallableFunctionStringReverse() :CallableFunctionStringT<pl_string_reverse>("string.reverse"){}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStringReverse, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStringReverse, ObjectInfo);



class CallableFunctionStringSubstr : public CallableFunction
{
public:

	CallableFunctionStringSubstr():CallableFunction("string.substr"){}

	int __fun_call(Executor& ewsl,int pm)
	{


		String* _pval = ewsl.ci1.nbp[StackState1::SBASE_THIS].ptr<String>();
		if(!_pval)
		{
			ewsl.kerror("invalid param");
		}

		if (pm == 0)
		{
			ewsl.push(*_pval);
			return 0;
		}

		StringBuffer<wchar_t> rhs(*_pval);
		int sz=(int)rhs.size();
		int p1,p2;

		if(pm>0)
		{
			p1=variant_cast<int>(ewsl.ci0.nbx[1]);
			if(p1<0)
			{
				p1+=sz;
			}
			if(p1<0) p1=0;
		}
		else
		{
			p1=0;
		}

		if(pm>1)
		{
			p2=variant_cast<int>(ewsl.ci0.nbx[2]);
			if(p2<0)
			{
				p2+=sz+1;
			}
			if(p2>=sz)
			{
				p2=sz;
			}
		}
		else
		{
			p2=sz;
		}

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
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStringSubstr, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionStringSubstr, ObjectInfo);


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

	CallableFunctionStringConcat() :CallableFunction("string.concat",1){}

	int __fun_call(Executor& ewsl, int pm)
	{
		StringBuffer<char> res;
		for (int i = 1; i <= pm; i++)
		{
			res << pl_cast<String>::g(ewsl.ci0.nbx[i]);
		}
		ewsl.ci0.nbx[1].reset<String>(res);
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStringConcat, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionStringConcat, ObjectInfo);


void init_module_string()
{
	CG_GGVar &gi(CG_GGVar::current());
	gi.add_inner<CallableMetatableT<String> >();

	gi.add_inner<CallableFunctionStringLength>();
	gi.add_inner<CallableFunctionStringUpper>();
	gi.add_inner<CallableFunctionStringLower>();
	gi.add_inner<CallableFunctionStringReverse>();
	gi.add_inner<CallableFunctionStringSubstr>();

	gi.add_inner<CallableFunctionStringFormat>();
	gi.add_inner<CallableFunctionStringConcat>();
		

}


EW_LEAVE
