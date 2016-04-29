#include "ewa_base/scripting.h"
#include "ewa_base/util/json.h"

EW_ENTER


template<unsigned N>
class fk_print_to_stringbuffer_1
{
public:

	static void g(StringBuffer<char>& sb,const arr_1t<Variant>& v)
	{
		sb<<String::Format("array:%p",&v);
	}

	template<typename X>
	static void g(StringBuffer<char>& sb,const arr_xt<X>& v)
	{
		sb<<String::Format("array:%p",&v);
	}

	static void g(StringBuffer<char>& sb,const String& v)
	{
		sb<<v;
	}

	static void g(StringBuffer<char>& sb,const ColonData& v)
	{
		sb<<"colondata";
	}

	static void g(StringBuffer<char>& sb,const dcomplex& v)
	{
		sb<<String::Format("complex(%g,%g)",v.real(),v.imag());
	}

	static void g(StringBuffer<char>& sb,const VariantTable& v)
	{
		sb<<String::Format("table:%p",&v);
	}

	static void g(StringBuffer<char>& sb,CallableData* v)
	{
		if(v==NULL)
		{
			sb<<"nil";
			return;
		}

		String s;
		if(v->ToValue(s))
		{
			sb<<s;
		}
		else
		{
			sb<<String::Format("object:%p",v);
		}
	
	}


	static void g(StringBuffer<char>& sb,bool v)
	{
		sb<<v;
	}

	static void g(StringBuffer<char>& sb,int64_t v)
	{
		sb<<v;
	}

	static void g(StringBuffer<char>& sb,double v)
	{
		sb<<v;
	}

	template<typename X>
	static void g(StringBuffer<char>& sb,const X& v)
	{
		sb<<v;
	}


	static void value(StringBuffer<char>& sb,const Variant& v)
	{
		typedef typename flag_type<N>::type type;
		g(sb,variant_handler<type>::raw(v));
	}

};

template<unsigned N>
class fk_print_to_stringbuffer_2
{
public:

	template<typename X>
	static void k(StringBuffer<char>& sb,const X& v)
	{
		sb<<v;
	}

	static void k(StringBuffer<char>& sb,const Variant& v)
	{
		sb<<pl_cast<String>::g(v);
	}

	template<typename X>
	static void g(StringBuffer<char>& sb,const arr_xt<X>& v)
	{
		const arr_xt_dims& dims(v.size_ptr());
		int d=6;for(;d>1&&dims[d-1]==1;--d);

		if(d==1)
		{
			for(size_t i=0;i<dims[0];i++)
			{
				k(sb,v(i));
				sb<<"\n";
			}
			sb<<"\n";
		}
		else if(d==2)
		{
			for(size_t i=0;i<dims[0];i++)
			{
				for(size_t j=0;j<dims[1];j++)
				{
					k(sb,v(i,j));
					sb<<" ";
				}
				sb<<"\n";
			}
			sb<<"\n";
		}		
	}

	static void g(StringBuffer<char>& sb,const String& v)
	{
		sb<<v;
	}

	static void g(StringBuffer<char>& sb,const dcomplex& v)
	{
		sb<<String::Format("complex(%g,%g)",v.real(),v.imag());
	}

	static void g(StringBuffer<char>& sb,const VariantTable& v)
	{
		sb<<"table(size:"<<v.size()<<",addr:"<<(void*)&v<<")\n";
		for(size_t i=0;i<v.size();i++)
		{
			sb<<v.get(i).first<<"\t: "<<pl_cast<String>::g(v.get(i).second)<<"\n";
		}
		sb<<"\n";
	}

	static void g(StringBuffer<char>& sb,const CallableData* const& v)
	{
		fk_print_to_stringbuffer_1<N>::g(sb,v);
	}


	static void g(StringBuffer<char>& sb,bool v)
	{
		sb<<v;
	}

	static void g(StringBuffer<char>& sb,int64_t v)
	{
		sb<<v;
	}

	static void g(StringBuffer<char>& sb,double v)
	{
		sb<<v;
	}

	template<typename X>
	static void g(StringBuffer<char>& sb,const X& v)
	{
		sb<<v;
	}

	static void value(StringBuffer<char>& sb,const Variant& v)
	{
		typedef typename flag_type<N>::type type;
		g(sb,variant_handler<type>::raw(v));
	}

};


template<unsigned N>
class fk_print_to_stringbuffer_3
{
public:

	template<typename X>
	static void k(StringBuffer<char>& sb,const X& v)
	{
		sb<<v;
	}

	static void g(StringBuffer<char>& sb,const Variant& v)
	{
		sb<<pl_cast<String>::g(v);
	}

	template<typename X>
	static void g(StringBuffer<char>& sb,const arr_xt<X>& v)
	{
		const arr_xt_dims& dims(v.size_ptr());

		int d=6;for(;d>1&&dims[d-1]==1;--d);

		sb<<"array(size:";
		for(int i=0;i<d;i++)
		{
			sb<<dims[i];
			if(i!=d-1) sb<<"x";
		}
		sb<<",addr:";
		sb<<(void*)&v;
		sb<<")\n";

		if(d==1)
		{
			for(size_t i=0;i<dims[0];i++)
			{
				g(sb,v(i));
				sb<<"\n";
			}

		}
		else if(d==2)
		{

			for(size_t i=0;i<dims[0];i++)
			{
				for(size_t j=0;j<dims[1];j++)
				{
					g(sb,v(i,j));
					sb<<" ";
				}
				sb<<"\n";
			}
			//sb<<"\n";
		}		
	}

	static void g(StringBuffer<char>& sb,const String& v)
	{
		sb<<v;
	}

	static void g(StringBuffer<char>& sb,const dcomplex& v)
	{
		sb<<String::Format("complex(%g,%g)",v.real(),v.imag());
	}

	static void g(StringBuffer<char>& sb,const VariantTable& v)
	{
		sb<<"table(size:"<<v.size()<<",addr:"<<(void*)&v<<")\n";
		for(size_t i=0;i<v.size();i++)
		{
			sb<<v.get(i).first<<"\t: "<<pl_cast<String>::g(v.get(i).second)<<"\n";
		}
		sb<<"\n";
	}

	static void g(StringBuffer<char>& sb,CallableData* v)
	{
		fk_print_to_stringbuffer_1<N>::g(sb,v);
	}


	static void g(StringBuffer<char>& sb,bool v)
	{
		sb<<v;
	}

	static void g(StringBuffer<char>& sb,int64_t v)
	{
		sb<<v;
	}

	static void g(StringBuffer<char>& sb,double v)
	{
		sb<<v;
	}

	template<typename X>
	static void g(StringBuffer<char>& sb,const X& v)
	{
		sb<<v;
	}

	static void value(StringBuffer<char>& sb,const Variant& v)
	{
		typedef typename flag_type<N>::type type;
		g(sb,variant_handler<type>::raw(v));
	}

};

class CallableFunctionPrintBase : public CallableFunction
{
public:

	CallableFunctionPrintBase(const String& s,int f):CallableFunction(s,f)
	{
		aBreakers["beg0"];
		aBreakers["beg1"];
		aBreakers["end0"];
		aBreakers["end1"];
	}

	bst_map<String,String> aBreakers;

	void _do_print_to_stringbuffer_1(StringBuffer<char>& sb,Variant* bp,int pm)
	{
		typedef void (*fn)(StringBuffer<char>&,const Variant&);
		typedef lookup_table_4bit<fk_print_to_stringbuffer_1,fn> lk;

		for(int i=1;i<=pm;i++)
		{
			sb<<(i== 1?aBreakers["beg0"]:aBreakers["beg1"]);
			lk::test(bp[i].type())(sb,bp[i]);
			sb<<(i==pm?aBreakers["end0"]:aBreakers["end1"]);	
		}
	}

	void _do_print_to_stringbuffer_2(StringBuffer<char>& sb,Variant* bp,int pm)
	{
		typedef void (*fn)(StringBuffer<char>&,const Variant&);
		typedef lookup_table_4bit<fk_print_to_stringbuffer_2,fn> lk;

		for(int i=1;i<=pm;i++)
		{
			sb<<(i== 1?aBreakers["beg0"]:aBreakers["beg1"]);
			lk::test(bp[i].type())(sb,bp[i]);
			sb<<(i==pm?aBreakers["end0"]:aBreakers["end1"]);	
		}
	}

	void _do_print_to_stringbuffer_3(StringBuffer<char>& sb,Variant* bp,int pm)
	{
		typedef void (*fn)(StringBuffer<char>&,const Variant&);
		typedef lookup_table_4bit<fk_print_to_stringbuffer_3,fn> lk;

		for(int i=1;i<=pm;i++)
		{
			sb<<(i== 1?aBreakers["beg0"]:aBreakers["beg1"]);
			lk::test(bp[i].type())(sb,bp[i]);
			sb<<(i==pm?aBreakers["end0"]:aBreakers["end1"]);	
		}
	}

	int __setindex(Executor& ewsl,const String& id)
	{
		if(aBreakers.find(id)!=aBreakers.end())
		{
			aBreakers[id]=pl_cast<String>::g(ewsl.ci1.nsp[0]);
			ewsl.popq();
			return 0;
		}

		ewsl.kerror("invalid index");
		return -1;

	}

	int __getindex(Executor& ewsl,const String& id)
	{
		if(aBreakers.find(id)!=aBreakers.end())
		{
			ewsl.push(aBreakers[id]);
			return 1;
		}

		ewsl.kerror("invalid index");
		return -1;
	}
};

class CallableFunctionPrint : public CallableFunctionPrintBase
{
public:

	CallableFunctionPrint():CallableFunctionPrintBase("io.print",1)
	{
		aBreakers["end0"]="";
		aBreakers["end1"]=" ";
	}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		StringBuffer<char> sb;
		_do_print_to_stringbuffer_1(sb,ewsl.ci0.nbx,pm);
		this_logger().Print(sb.c_str());
		return 0;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionPrint, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionPrint, ObjectInfo);

class CallableFunctionPrintLn : public CallableFunctionPrintBase
{
public:
	CallableFunctionPrintLn():CallableFunctionPrintBase("io.println",1)
	{
		aBreakers["end0"]="\n";
		aBreakers["end1"]=" ";
	}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if(pm>0)
		{
			StringBuffer<char> sb;
			_do_print_to_stringbuffer_1(sb,ewsl.ci0.nbx,pm);
			this_logger().Print(sb.c_str());
		}
		else
		{

			this_logger().Print(aBreakers["end0"]);
		}
		return 0;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionPrintLn, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionPrintLn, ObjectInfo);


class CallableFunctionPrintEx : public CallableFunctionPrintBase
{
public:
	CallableFunctionPrintEx():CallableFunctionPrintBase("io.printex",1)
	{
		aBreakers["end0"]="\n";
		aBreakers["end1"]="\n";
	}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if(pm>0)
		{
			StringBuffer<char> sb;
			_do_print_to_stringbuffer_3(sb,ewsl.ci0.nbx,pm);

			this_logger().Print(sb);
		}
		else
		{
			this_logger().Print(aBreakers["end0"]);
		}
		return 0;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionPrintEx, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionPrintEx, ObjectInfo);


class CallableFunctionShowTemp : public CallableFunctionPrintBase
{
public:

	CallableFunctionShowTemp():CallableFunctionPrintBase("#show_temp",1)
	{
		aBreakers["beg0"]="results: ";
		aBreakers["end0"]="\n";

		aBreakers["beg1"]="";
		aBreakers["end1"]=",";
	}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if(pm>0)
		{
			StringBuffer<char> sb;
			_do_print_to_stringbuffer_3(sb,ewsl.ci0.nbx,pm);
			this_logger().Print(sb);
		}
		return pm;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionShowTemp, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionShowTemp, ObjectInfo);


class CallableFunctionSaveTxt : public CallableFunctionPrintBase
{
public:
	CallableFunctionSaveTxt():CallableFunctionPrintBase("io.save_txt",1)
	{
		aBreakers["end0"]="\n";
		aBreakers["end1"]=" ";
	}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if(pm!=2)
		{
			ewsl.ci0.nbx[1].reset(false);
		}
		else
		{
			StringBuffer<char> sb;
			_do_print_to_stringbuffer_2(sb,ewsl.ci0.nbx,1);
			bool f=sb.save(variant_cast<String>(ewsl.ci0.nbx[2]));
			ewsl.ci0.nbx[1].reset(f);
		}			

		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionSaveTxt, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionSaveTxt, ObjectInfo);



class CallableFunctionLoadJson : public CallableFunction
{
public:

	CallableFunctionLoadJson():CallableFunction("io.load_json"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		String *p=ewsl.ci0.nbx[1].ptr<String>();
		if(!p)
		{
			ewsl.kerror("invalid param");
			return 0;
		}

		StringBuffer<char> sb;
		if(!sb.load(*p,FILE_TEXT))
		{
			ewsl.kerror("invalid jsonfile");
			return 0;
		}

		ewsl.ci0.nbx[1]=parse_json(sb);		

		return 1;
	
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionLoadJson, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionLoadJson, ObjectInfo);

void init_module_io()
{
	CG_GGVar& gi(CG_GGVar::current());
	gi.add_inner<CallableFunctionPrint>();
	gi.add_inner<CallableFunctionPrintLn>();
	gi.add_inner<CallableFunctionPrintEx>();
	gi.add_inner<CallableFunctionShowTemp>();
	gi.add_inner<CallableFunctionSaveTxt>();
	gi.add_inner<CallableFunctionLoadJson>();

}

EW_LEAVE
