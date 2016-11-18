#include "ewa_base/scripting.h"
#include "ewa_base/util/json.h"
#include "ewa_base/xml/xml_document.h"
#include "ewa_base/serialization/serializer_stream.h"
EW_ENTER


template<unsigned N>
class fk_print_to_stringbuffer_1
{
public:

	static void g(StringBuffer<char>& sb,const arr_1t<Variant>& v)
	{
		sb<<String::Format("array:0x%p",&v);
	}

	template<typename X>
	static void g(StringBuffer<char>& sb,const arr_xt<X>& v)
	{
		sb<<String::Format("array:0x%p",&v);
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
		if(v.imag()<0.0)
		{
			sb<<String::Format("%g%gi",v.real(),v.imag());
		}
		else
		{
			sb<<String::Format("%g+%gi",v.real(),v.imag());
		}

	}

	static void g(StringBuffer<char>& sb,const VariantTable& v)
	{
		sb<<String::Format("table:0x%p",&v);
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
			sb<<String::Format("object:0x%p",v);
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

	static void k(StringBuffer<char>& sb,const dcomplex& v)
	{
		fk_print_to_stringbuffer_1<N>::g(sb,v);
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
		sb<<String::Format("%g+%gi",v.real(),v.imag());
	}

	static void g(StringBuffer<char>& sb,const VariantTable& v)
	{
		sb<<"table(size:"<<v.size()<<",addr:0x"<<(void*)&v<<")\n";
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
		sb<<",addr:0x";
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
		fk_print_to_stringbuffer_1<N>::g(sb,v);
	}

	static void g(StringBuffer<char>& sb,const VariantTable& v)
	{
		sb<<"table(size:"<<v.size()<<",addr:0x"<<(void*)&v<<")\n";
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


class CallableFunctionPuts : public CallableFunctionPrintBase
{
public:

	CallableFunctionPuts():CallableFunctionPrintBase("io.puts",1)
	{
		aBreakers["end0"]="";
		aBreakers["end1"]="";
	}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		StringBuffer<char> sb;
		_do_print_to_stringbuffer_1(sb,ewsl.ci0.nbx,pm);
		this_logger().Print(sb.c_str());
		return 0;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionPuts, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionPuts, ObjectInfo);

class CallableFunctionPrint : public CallableFunctionPrintBase
{
public:

	CallableFunctionPrint():CallableFunctionPrintBase("io.print",1)
	{
		aBreakers["end0"]=" ";
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
			_do_print_to_stringbuffer_2(sb,ewsl.ci0.nbx+1,1);
			bool f=sb.save(variant_cast<String>(ewsl.ci0.nbx[1]));
			ewsl.ci0.nbx[1].reset(f);
		}

		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionSaveTxt, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionSaveTxt, ObjectInfo);


class ArrayParser
{
public:


	void skip_comment_1()
	{
		if (pchar[0] == '/'&&pchar[1] == '/')
		{
			for (pchar += 2; *pchar; pchar++)
			{
				if (pchar[0] == '\n')
				{
					break;
				}
			}
		}
	}

	void skip_blank()
	{
		for (;;)
		{
			while (*pchar == ' ' || *pchar == '\r' || *pchar == '\t') pchar++;
			if(pchar[0]=='/'&&pchar[1]=='/') skip_comment_1();
			break;
		}
	}


	const char* pchar;
	bool is_complex;

	arr_xt<double> darr;
	arr_xt<dcomplex> carr;


	double read_double()
	{
		double s=1.0;
		double v=0;

		if(pchar[0]=='+') pchar++;
		else if(pchar[0]=='-')
		{
			s=-1.0;
			pchar++;
		}

		if(pchar[0]!='.'&&pchar[0]<'0'&&pchar[0]>'9')
		{
			Exception::XError("invalid number");
		}

		while(pchar[0]>='0'&&pchar[0]<='9')
		{
			v=v*10.0+double(pchar[0]-'0');
			pchar++;
		}

		if(pchar[0]=='.')
		{
			double d=0.1;
			pchar++;
			while(pchar[0]>='0'&&pchar[0]<='9')
			{
				v=v+d*double(pchar[0]-'0');
				d=d*0.1;
				pchar++;
			}
		}

		if(pchar[0]=='e'||pchar[0]=='E')
		{
			pchar++;
			int pow_value=0;
			int pow_sign=1;

			if(pchar[0]=='+') pchar++;
			else if(pchar[0]=='-')
			{
				pow_sign=-1;
				pchar++;
			}
			while(pchar[0]>='0'&&pchar[0]<='9')
			{
				pow_value=pow_value*10+(pchar[0]-'0');
				pchar++;
			}
			v=v*::pow(10.0,double(pow_sign*pow_value));
		}

		return v*s;
	}

	void read_number()
	{
		double v1=read_double();
		double v2=0.0;

		if(pchar[0]=='i'||pchar[0]=='j')
		{
			v2=v1;
			v1=0.0;
		}
		else if(pchar[0]=='+'||pchar[0]=='-')
		{
			v2=read_double();
			if(pchar[0]=='i'||pchar[0]=='j')
			{
				pchar++;
			}
			else
			{
				Exception::XError("invalid imag part");
			}
			if(!is_complex)
			{
				is_complex=true;
				carr.assign(darr.begin(),darr.end());
				darr.clear();
			}
		}

		if(is_complex)
		{
			carr.push_back(dcomplex(v1,v2));
		}
		else
		{
			darr.push_back(v1);
		}
	}

	template<typename T>
	Variant adjust(arr_xt<T>& val,size_t c)
	{

		size_t n=val.size();
		if(n%c!=0) Exception::XError("invalid col");
		size_t r=n/c;

		val.reshape(c,r);

		Variant var;
		arr_xt<T>& dst(var.ref<arr_xt<T> >());
		dst.resize(r,c);
		for(size_t i=0;i<r;i++) for(size_t j=0;j<c;j++)
		{
			dst(i,j)=val(j,i);
		}

		return var;
	}

	Variant parse(const String& data)
	{
		is_complex=false;

		int col=-1;

		pchar = data.c_str();
		while(*pchar)
		{
			int n=0;
			while(1)
			{
				skip_blank();
				if(pchar[0]=='\0'||pchar[0]=='\n') break;
				read_number();
				n++;
			}

			if(pchar[0]=='\0') break;

			pchar++;
			if(n==0) continue;
			if(col<0) col=n;
			else if(col!=n)
			{
				Exception::XError("invalid colsize");
			}
		}

		if(col<1) Exception::XError("invalid col");

		if(is_complex)
		{
			return adjust(carr,col);
		}
		else
		{
			return adjust(darr,col);
		}
	}



};


Variant parse_array(const String& s)
{
	ArrayParser parser;
	return parser.parse(s);
}

class CallableFunctionLoadTxt : public CallableFunction
{
public:
	CallableFunctionLoadTxt():CallableFunction("io.load_txt",1)
	{

	}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		String file=variant_cast<String>(ewsl.ci0.nbx[1]);
		StringBuffer<char> sb;
		if(!sb.load(file,FILE_TYPE_TEXT))
		{
			ewsl.kerror("invalid file");
		}
		ewsl.ci0.nbx[1]=parse_array(sb);
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionLoadTxt, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionLoadTxt, ObjectInfo);

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
		if(!sb.load(*p,FILE_TYPE_TEXT))
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



class CallableFunctionLoadVar : public CallableFunction
{
public:

	CallableFunctionLoadVar():CallableFunction("io.load_var",1){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		String *p=ewsl.ci0.nbx[1].ptr<String>();
		if(!p)
		{
			ewsl.kerror("invalid param");
			return 0;
		}

		SerializerFile ar;

		arr_1t<Variant> var;
		if(ar.file.open(*p,FLAG_FILE_RD))
		{
			ar.reader() & var;
		}
		else
		{
			ewsl.kerror("invalid file");
		}

		std::copy(var.begin(), var.end(), ewsl.ci0.nbx + 1);

		return var.size();

	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionLoadVar, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionLoadVar, ObjectInfo);

class CallableFunctionSaveVar : public CallableFunction
{
public:

	CallableFunctionSaveVar():CallableFunction("io.save_var",1){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1,-1);
		String *p=ewsl.ci0.nbx[1].ptr<String>();
		if(!p)
		{
			ewsl.kerror("invalid param");
			return 0;
		}

		SerializerFile ar;

		arr_1t<Variant> var;
		var.assign(ewsl.ci0.nbx + 2, pm - 1);

		if(ar.file.open(*p,FLAG_FILE_WC))
		{
			ar.writer() & var;
		}
		else
		{
			ewsl.kerror("invalid file");
		}

		return 0;

	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionSaveVar, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionSaveVar, ObjectInfo);



class CallableFunctionToJson : public CallableFunction
{
public:

	CallableFunctionToJson():CallableFunction("io.to_json",1){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		Variant var;
		to_json(ewsl.ci0.nbx[1],var.ref<StringBuffer<char> >());
		ewsl.ci0.nbx[1]=var;
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionToJson, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionToJson, ObjectInfo);


class CallableFunctionParseJson : public CallableFunction
{
public:

	CallableFunctionParseJson():CallableFunction("io.parse_json",1){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		if(StringBuffer<char>* p=ewsl.ci0.nbx[1].ptr<StringBuffer<char> >())
		{
			ewsl.ci0.nbx[1]=parse_json(*p);
		}
		else
		{
			ewsl.ci0.nbx[1]=parse_json(variant_cast<String>(ewsl.ci0.nbx[1]));
		}
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionParseJson, ObjectInfo);

};

IMPLEMENT_OBJECT_INFO(CallableFunctionParseJson, ObjectInfo);



template<>
class CallableWrapT<XmlDocument> : public CallableData
{
public:
	XmlDocument value;
};

class CallableXmlNode : public CallableData
{
public:

	DataPtrT<CallableWrapT<XmlDocument> > pxmldoc;

	CallableXmlNode():node(NULL),parent(NULL){}
	CallableXmlNode(CallableWrapT<XmlDocument>* p):pxmldoc(p),node(NULL),parent(NULL)
	{
		if(pxmldoc)
		{
			parent=&pxmldoc->value;
			node=parent->GetFirstChild();
		}
	}

	XmlNode* node;
	XmlNode* parent;

	CallableXmlNode* ctor(XmlNode* p,XmlNode* c)
	{
		if(c==NULL) return NULL;
		CallableXmlNode* nd=new CallableXmlNode(*this);
		nd->parent=p;
		nd->node=c;
		return nd;
	}

	virtual int __getindex(Executor& ewsl,const String& si)
	{
		Variant& vtop(*ewsl.ci1.nsp);
		if(!node)
		{
			vtop.clear();
		}
		else if(si=="first")
		{
			vtop.reset(ctor(node,node->GetFirstChild()));
		}
		else if(si=="next")
		{
			vtop.reset(ctor(parent,node->GetNext()));			
		}
		else if(si=="type")
		{
			vtop.reset(node->GetType());			
		}
		else if(si=="name")
		{
			vtop.reset(node->GetName());			
		}
		else if(si=="value")
		{
			vtop.reset(node->GetValue());			
		}
		else
		{
			ewsl.kerror("invalid index");
		}

		return STACK_BALANCED;
	
	}
};


class CallableFunctionParseXml : public CallableFunction
{
public:

	CallableFunctionParseXml():CallableFunction("io.parse_xml",1){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		String xml=variant_cast<String>(ewsl.ci0.nbx[1]);
		DataPtrT<CallableWrapT<XmlDocument> > nd(new CallableWrapT<XmlDocument>);
		if(!nd->value.LoadStr(xml.c_str()))
		{
			ewsl.ci0.nbx[1].clear();
		}
		else
		{
			ewsl.ci0.nbx[1].kptr(new CallableXmlNode(nd.get()));
		}
		return 1;	
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionParseXml, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionParseXml, ObjectInfo);


class CallableFunctionLoadXml : public CallableFunction
{
public:

	CallableFunctionLoadXml():CallableFunction("io.load_xml",1){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		String xml=variant_cast<String>(ewsl.ci0.nbx[1]);
		DataPtrT<CallableWrapT<XmlDocument> > nd(new CallableWrapT<XmlDocument>);
		if(!nd->value.LoadXml(xml.c_str()))
		{
			ewsl.ci0.nbx[1].clear();
		}
		else
		{
			ewsl.ci0.nbx[1].kptr(new CallableXmlNode(nd.get()));
		}
		return 1;	
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionLoadXml, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionLoadXml, ObjectInfo);



void init_module_io()
{
	CG_GGVar& gi(CG_GGVar::current());

	gi.add_inner<CallableFunctionLoadVar>();
	gi.add_inner<CallableFunctionSaveVar>();
	gi.add_inner<CallableFunctionLoadTxt>();
	gi.add_inner<CallableFunctionSaveTxt>();
	gi.add_inner<CallableFunctionLoadJson>();
	gi.add_inner<CallableFunctionLoadXml>();


	gi.add_inner<CallableFunctionPuts>();
	gi.add_inner<CallableFunctionPrint>();
	gi.add_inner<CallableFunctionPrintLn>();
	gi.add_inner<CallableFunctionPrintEx>();
	gi.add_inner<CallableFunctionShowTemp>();
	gi.add_inner<CallableFunctionParseXml>();

	gi.add_inner<CallableFunctionToJson>();
	gi.add_inner<CallableFunctionParseJson>();



}

EW_LEAVE
