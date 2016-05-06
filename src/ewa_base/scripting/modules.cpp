#include "ewa_base/scripting.h"
#include "ewa_base/serialization.h"
#include "ewa_base/util/strlib.h"

EW_ENTER

class CallableFunctionImport : public CallableFunction
{
public:
	CallableFunctionImport() :CallableFunction("import"){}

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this, pm, 1,2);

		String *p = ewsl.ci0.nbx[1].ptr<String>();
		if (p)
		{
			bool reload=false;
			if (pm == 2) reload = variant_cast<bool>(ewsl.ci0.nbx[2]);

			CG_GGVar::current().import(*p,reload);
			ewsl.ci0.nbx[1] = ewsl.tb0[*p];
		}
		else
		{
			Exception::XError("invalid module name");
		}
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionImport, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionImport, ObjectInfo);

class CallableFunctionCaptureLocal : public CallableFunction
{
public:
	CallableFunctionCaptureLocal():CallableFunction("#capture_local"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,3);

		CallableCode* pInst=dynamic_cast<CallableCode*>(ewsl.ci1.nsp[-2].kptr());
		if(!pInst)
		{
			ewsl.kerror("first param must be userfunction");
		}

		size_t nLLvar=(size_t)ewsl.ci1.nsp[-1].ref<int64_t>();
		if(nLLvar>=pInst->aLLVar.size())
		{
			ewsl.kerror("invalid local param position");
		}

		pInst->aLLVar[nLLvar]=ewsl.ci1.nsp[0];
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionCaptureLocal, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionCaptureLocal, ObjectInfo);


class CallableFunctionSame : public CallableFunction
{
public:
	CallableFunctionSame() :CallableFunction("#same"){}

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this, pm, 2);
		ewsl.ci0.nbx[1].reset<bool>(ewsl.ci0.nbx[1].kptr() == ewsl.ci0.nbx[2].kptr());		
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionSame, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionSame, ObjectInfo);

class CallableFunctionClone : public CallableFunction
{
public:
	CallableFunctionClone():CallableFunction("clone"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		CallableData* q=ewsl.ci1.nsp[0].kptr();
		if(q && q->GetRef()>1)
		{
			ewsl.ci1.nsp[0].kptr(q->Clone(1));
		}
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionClone, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionClone, ObjectInfo);




class CallableFunctionTypeId : public CallableFunction
{
public:
	CallableFunctionTypeId():CallableFunction("typeid"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		ewsl.ci1.nsp[0].reset<int64_t>(ewsl.ci1.nsp[0].type());
		return 1;
	}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionTypeId, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionTypeId, ObjectInfo);

class CallableFunctionAssert : public CallableFunction
{
public:
	CallableFunctionAssert():CallableFunction("assert"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if(pm==0) return 0;
		if(variant_cast<bool>(ewsl.ci0.nbx[1]))
		{
			return pm;
		}

		if(pm==1)
		{
			ewsl.push("assertion failure");
			ewsl.kthrow(1);
		}
		else
		{
			ewsl.kthrow(pm-1);
		}
		return STACK_BALANCED;		
	}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionAssert, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionAssert, ObjectInfo);


class CallableFunctionHelp : public CallableFunction
{
public:
	CallableFunctionHelp():CallableFunction("help"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if (pm == 0)
		{
			this_logger().PrintLn("usage: help(function_name[,level]);");
			return 0;
		}

		int lv=0;
		if(pm>=2)
		{
			lv=variant_cast<int>(ewsl.ci0.nbx[2]);
		}

		DataPtrT<CallableData> _kp=ewsl.ci0.nbx[1].kptr();

		HelpData* _hd=_kp ? _kp->__get_helpdata() : NULL;

		String _sHelp;
		if (_hd)
		{
			_hd->ToValue(_sHelp, 0);
		}
		else
		{
			_sHelp = "no_help";
		}

		this_logger().PrintLn(_sHelp);

		return 0;		
	}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionHelp, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionHelp, ObjectInfo);

class CallableFunctionIsNil : public CallableFunction
{
public:
	CallableFunctionIsNil() :CallableFunction("is_nil"){}

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		bool f = true;
		if (pm > 0)
		{
			f = ewsl.ci0.nbx[1].is_nil();
		}
		ewsl.ci0.nbx[1].reset(f);
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionIsNil, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionIsNil, ObjectInfo);

class CallableFunctionIsTable : public CallableFunction
{
public:
	CallableFunctionIsTable() :CallableFunction("is_table"){}

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		bool f = false;
		if (pm > 0)
		{
			CallableData* p= ewsl.ci0.nbx[1].kptr();
			f = p && p->ToTable();
		}
		ewsl.ci0.nbx[1].reset(f);
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionIsTable, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionIsTable, ObjectInfo);

class CallableFunctionTranslate : public CallableFunction
{
public:
	CallableFunctionTranslate():CallableFunction("translate"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if(pm==1)
		{
			String *p=ewsl.ci0.nbx[1].ptr<String>();
			if(p)
			{
				ewsl.ci0.nbx[1].reset(Translate(*p));
			}
			else
			{
				this_logger().LogTrace("not string");
			}
			return 1;	
		}
		else if(pm==2)
		{
			String *p=ewsl.ci0.nbx[1].ptr<String>();
			String *d=ewsl.ci0.nbx[2].ptr<String>();
			if(p && d)
			{
				ewsl.ci0.nbx[1].reset(Translate(*p,*d));
			}
			else
			{
				this_logger().LogTrace("not string");
			}
			return 1;	
		}

		ewsl.kerror("invalid param count");
		return 1;		
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionTranslate, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionTranslate, ObjectInfo);

class CallableFunctionTranslateHT : public CallableFunction
{
public:
	CallableFunctionTranslateHT():CallableFunction("_hT"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		String *p=ewsl.ci0.nbx[1].ptr<String>();
		if(p)
		{
			ewsl.ci0.nbx[1].reset(Translate(*p));
		}
		else
		{
			this_logger().LogTrace("not string");
		}
		return 1;		
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionTranslateHT, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionTranslateHT, ObjectInfo);

class CallableFunctionTranslateZT : public CallableFunction
{
public:
	CallableFunctionTranslateZT():CallableFunction("_zT"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		String *p=ewsl.ci0.nbx[1].ptr<String>();
		if(p)
		{
			ewsl.ci0.nbx[1].reset(Translate(*p,""));
		}
		else
		{
			this_logger().LogTrace("not string");
		}
		return 1;		
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionTranslateZT, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionTranslateZT, ObjectInfo);

class CallableFunctionTranslateKT : public CallableFunction
{
public:
	CallableFunctionTranslateKT():CallableFunction("_kT"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		String *p=ewsl.ci0.nbx[1].ptr<String>();
		if(p)
		{

		}
		else
		{
			this_logger().LogTrace("not string");
		}
		return 1;		
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionTranslateKT, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionTranslateKT, ObjectInfo);

class CallableTableProxyGlobal : public CallableFunction
{
public:

	CallableTableProxyGlobal() :CallableFunction("_G"){}
	int __setindex(Executor& ewsl,const String& si){return CallableTableOperators::__do_setindex(ewsl,ewsl.tb1,si);}
	int __getindex(Executor& ewsl,const String& si){return CallableTableOperators::__do_getindex(ewsl,ewsl.tb1,si);}
	virtual int __getarray(Executor& ewsl,int pm){return CallableTableOperators::__do_getarray(ewsl,ewsl.tb1,pm);}
	virtual int __setarray(Executor& ewsl,int pm){return CallableTableOperators::__do_setarray(ewsl,ewsl.tb1,pm);}
	void __get_iterator(Executor& ewsl,int nd){CallableTableOperators::__do_get_iterator(ewsl,ewsl.tb1,nd);}

	bool ToValue(String& v,int) const{v="table:global variable proxy";return true;}

	DECLARE_OBJECT_CACHED_INFO(CallableTableProxyGlobal, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableTableProxyGlobal, ObjectInfo);


class CallableCommand : public CallableFunction
{
public:
	CallableCommand(const String& s):CallableFunction(s){}
};

class CallableCommandClear : public CallableCommand
{
public:

	CallableCommandClear():CallableCommand("clear"){}

	virtual int __fun_call(Executor& ewsl,int)
	{
		ewsl.tb1.clear();
		return 0;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableCommandClear, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableCommandClear, ObjectInfo);

class CallableCommandClc : public CallableCommand
{
public:

	CallableCommandClc():CallableCommand("clc"){}
	virtual int __fun_call(Executor& ewsl,int)
	{
		this_logger().LogCommand("clc");
		return 0;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableCommandClc, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableCommandClc, ObjectInfo);

class CallableCommandLoadEwsl : public CallableCommand
{
public:

	CallableCommandLoadEwsl() :CallableCommand("load_ewsl"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1,2);

		int type=Executor::INDIRECT_FILE;
		if(pm==2)
		{
			type=variant_cast<int>(ewsl.ci0.nbx[2]);
		}

		String p=ewsl.ci0.nbx[1].get<String>();
		if (ewsl.prepare(p, type))
		{
			ewsl.ci0.nbx[1] = *ewsl.ci1.nsp;
			return 1;
		}
		ewsl.kerror("load_ewsl error");
		return 0;	
	}
	DECLARE_OBJECT_CACHED_INFO(CallableCommandLoadEwsl, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableCommandLoadEwsl, ObjectInfo);


class CallableCommandExecutor : public CallableCommand
{
public:

	int type;
	
	CallableCommandExecutor(const String& s,int t):CallableCommand(s),type(t){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
	
		if(pm==0) return 0;
		String expr=ewsl.ci0.nbx[1].get<String>();	

		if(!ewsl.prepare(expr,type)||!ewsl.ci1.nsp->kptr())
		{
			ewsl.kerror("error");
		}
		ewsl.ci2.pfn.reset(ewsl.ci1.nsp->kptr());
		for(int i=1;i<pm;i++)
		{
			ewsl.ci0.nbx[i]=ewsl.ci0.nbx[i+1];
		}
		ewsl.ci1.nsp=ewsl.ci0.nbx+pm-1;
		return ewsl.ci2.pfn->__fun_call(ewsl,pm-1);
	}
};

class CallableCommandExec : public CallableCommandExecutor
{
public:
	CallableCommandExec():CallableCommandExecutor("exec",Executor::INDIRECT_FILE){}	
	DECLARE_OBJECT_CACHED_INFO(CallableCommandExec, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableCommandExec, ObjectInfo);

class CallableCommandEval : public CallableCommandExecutor
{
public:
	CallableCommandEval() :CallableCommandExecutor("eval",Executor::DIRECT_STRING){}
	DECLARE_OBJECT_CACHED_INFO(CallableCommandEval, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableCommandEval, ObjectInfo);


class CallableCommandNoop : public CallableCommand
{
public:

	CallableCommandNoop() :CallableCommand("noop"){}
	virtual int __fun_call(Executor&, int n){return n;}
	DECLARE_OBJECT_CACHED_INFO(CallableCommandNoop, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableCommandNoop, ObjectInfo);


void CG_GGVar::expand(const String& name)
{
	CG_GGVar& gi(CG_GGVar::current());
	CallableTableEx* p = dynamic_cast<CallableTableEx*>(gi[name].kptr());
	if (!p) return;
	String lib = name;
	for (VariantTable::const_iterator it = p->value.begin(); it != p->value.end(); ++it)
	{
		gi[lib + "." + (*it).first] = (*it).second;
	}
}


void init_module_math();
void init_module_coroutine();
void init_module_io();
void init_module_os();
void init_module_array();
void init_module_table();
void init_module_string();
void init_module_number();
void init_module_logger();

void CG_GGVar::_init()
{
	CG_GGVar& gi(*this);

	gi.reserve(4*1024);

	gi.add(new CallableWrapT<String>("1.0 beta"), "sys.VERSION");

	gi.add_inner<CallableFunctionCaptureLocal>();
	gi.add_inner<CallableFunctionSame>();

	gi.add_inner<CallableTableProxyGlobal>();
	gi.add_inner<CallableCommandNoop>();
	gi.add_inner<CallableFunctionImport>();
	gi.add_inner<CallableFunctionClone>();
	gi.add_inner<CallableFunctionTypeId>();
	gi.add_inner<CallableFunctionIsTable>();
	gi.add_inner<CallableFunctionIsNil>();

	gi.add_inner<CallableCommandClear>();
	gi.add_inner<CallableCommandClc>();
	gi.add_inner<CallableCommandExec>();
	gi.add_inner<CallableCommandEval>();
	gi.add_inner<CallableCommandLoadEwsl>();

	gi.add_inner<CallableFunctionAssert>();
	gi.add_inner<CallableFunctionHelp>();
	gi.add_inner<CallableFunctionTranslate>();
	gi.add_inner<CallableFunctionTranslateKT>();
	gi.add_inner<CallableFunctionTranslateHT>();
	gi.add_inner<CallableFunctionTranslateZT>();

	init_module_number();
	init_module_string();
	init_module_table();
	init_module_array();

	init_module_io();
	init_module_os();
	init_module_math();
	init_module_logger();
	init_module_coroutine();

	gi.add(NULL,"pcall");
	gi.add(NULL,"map");
	gi.add(NULL,"reduce");
	gi.add(NULL,"select");
	gi.add(NULL,"join");
	gi.add(NULL, "#internal_end");

	_bInited=true;

	for (size_t i = 0; i < gi.size(); i++)
	{
		if (gi.get(i).first == "#internal_end") break;
		gi.sm_vmap[gi.get(i).first] = new CG_Variable(VAR_SYSTEM, gi.get(i).first);
	}

	size_t n = gi.size();
	for (size_t i = 0; i < n; i++)
	{
		CallableTableEx* p = dynamic_cast<CallableTableEx*>(gi.get(i).second.kptr());
		if (!p) continue;
		String lib = gi.get(i).first;
		for (VariantTable::const_iterator it=p->value.begin();it!=p->value.end();++it)
		{
			gi[lib + "." + (*it).first] = (*it).second;
		}	
	}

	Executor ewsl;
	ewsl.execute(
"\n"
"function pcall(fn,...)\n"
"{\n"
"	try \n"
"		return true,fn(...);\n"
"	catch(...) \n"
"		return false,...;\n"
"};\n"
"\n"
"function map(x,f)\n"
"{\n"
"	local r=[];\n"
"	local k=0;\n"
"	for_each(v in x)\n"
"	{\n"
"		r.push(f(v,k++));\n"
"	}\n"
"	return r;\n"
"};\n"
"\n"
"function select(x,f)\n"
"{\n"
"	local r=[];\n"
"	local k=0;\n"
"	for_each(v in x)\n"
"	{\n"
"		if(f(v,k++)) r.push(v);\n"
"	}\n"
"	return r;	\n"
"};\n"
"\n"
"function join(x,r)\n"
"{\n"
"	local z=r;\n"
"	return reduce(x,def(x,y,k)\n"
"	{\n"
"		return k==0?x#y..z..x;\n"
"	});\n"
"};\n"
"function reduce(x,f,n)\n"
"{\n"
"	local k=0;\n"
"	for_each(v in x)\n"
"	{\n"
"		n=f(v,n,k++);	\n"
"	}\n"
"	return n;\n"
"};\n"
"\n"
		
		
		
		);
	for(size_t i=0;i<ewsl.tb1.size();i++)
	{
		gi[ewsl.tb1.get(i).first]=ewsl.tb1.get(i).second;
	}

}


static CG_GGVar* CG_GGVar_instance;

CG_GGVar& CG_GGVar::current()
{
	if (!CG_GGVar_instance)
	{
		static CG_GGVar gInstance;
		CG_GGVar_instance = &gInstance;

		gInstance._init();
	}
	return *CG_GGVar_instance;
}

CG_GGVar::~CG_GGVar()
{
	for (auto it = sm_vmap.begin(); it != sm_vmap.end(); ++it)
	{
		delete (*it).second;
	}
	sm_vmap.clear();
}

CG_Variable* CG_GGVar::find_var(const String& s)
{
	CG_GGVar& gi(CG_GGVar::current());
	indexer_map<String, CG_Variable*>& sm_vmap(gi.sm_vmap);
	int id = sm_vmap.find1(s);
	return id < 0 ? NULL : sm_vmap.get(id).second;
}

void CG_GGVar::add(CallableFunction* p)
{
	EW_ASSERT(p!=NULL);
	add(p, p->GetName(), p->flags.val());
}

void CG_GGVar::add(CallableMetatable* p)
{
	EW_ASSERT(p!=NULL);
	add(p, p->m_sClassName, 0);
}


void CG_GGVar::import(const String& lib,bool reload)
{

	CG_GGVar &tb0(CG_GGVar::current());

	if (!reload && !tb0[lib].is_nil())
	{
		return;
	}

	int idx = -1;

	static bst_set<String> loading;

	if(!loading.insert(lib).second)
	{
		return;
	}

	try
	{


		DataPtrT<CallableModule> pmodule(new CallableModule(lib));

		CodeGen cgen;
		cgen.module = lib;

		String libfile="";
	
		arr_1t<String> libpaths=string_split(System::GetEnv("EWSL_LIBPATH","ewsl_lib"),";");

		for(size_t i=0;i<libpaths.size();++i)
		{
			String path=libpaths[i];
			if(path=="") continue;

			char ch=path.c_str()[path.size()-1];
			if(ch!='\\' && ch!='/')
			{
				path+="/";
			}

			arr_1t<String> files=System::FindAllFiles(path,lib+".*");
			if(files.empty()) continue;

			libfile=path+files[0];
			break;
		}

		if(libfile=="")
		{
			libfile="ewsl_lib/"+lib + ".ewsl";
		}

		if (cgen.prepare(libfile, Executor::INDIRECT_FILE))
		{
			Executor lexer;
			lexer.push(cgen.get());

			if (lexer.callx(0, 0))
			{

				for (auto x = cgen.cg_exports.begin(); x != cgen.cg_exports.end(); ++x)
				{
					pmodule->value[*x] = tb0[lib + "." + (*x)];
				}
				
				tb0[lib].kptr(pmodule);
				loading.erase(lib);
				return;
			}
		}


	}
	catch (...)
	{

	}

	loading.erase(lib);	
	Exception::XError("unknown module:"+lib);
}

void CG_GGVar::add(CallableData* p,const String& s,int f)
{

	CG_GGVar& gi(CG_GGVar::current());
	arr_1t<String> ap = string_split(s, ".");

	size_t n = ap.size();

	if(n==0)
	{
		return;
	}
	
	if(n==1)
	{
		gi[s].kptr(p);
		return;
	}

	if(f!=0)
	{
		gi[ap[n-1]].kptr(p);
	}


	VariantTable *pt=&gi;
	for(size_t i=0;i<n;i++)
	{
		if(i+1==n)
		{
			if (i == 1 && _bInited)
			{
				gi[s].kptr(p);
			}

			(*pt)[ap[i]].kptr(p);
			break;
		}

		intptr_t d=pt->find1(ap[i]);
		if(d>=0)
		{
			pt=pt->get(d).second.ptr<VariantTable>();
			if (!pt)
			{
				return;
			}
		}
		else
		{
			String libname=string_join(ap.begin(),ap.begin()+i+1,".");
			DataPtrT<CallableModule> tb = new CallableModule(libname);

			(*pt)[ap[i]].kptr(tb);pt=&tb->value;

			if (i == 1 && _bInited)
			{
				gi[ap[0]+"."+ap[1]].kptr(tb);
			}

		}
	}

}

CG_GGVar::CG_GGVar()
{

}


EW_LEAVE
