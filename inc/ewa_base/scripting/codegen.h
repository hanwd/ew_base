#ifndef __H_EW_SCRIPTING_CODEGEN__
#define __H_EW_SCRIPTING_CODEGEN__

#include "ewa_base/scripting/parser_visitor.h"
#include "ewa_base/scripting/callable_code.h"
#include "ewa_base/scripting/parser_nodes.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE CodeGen;


class DLLIMPEXP_EWA_BASE CG_Local
{
public:

	CodeGen& cgen;

	CG_Local(CodeGen& cgen_);

	enum
	{
		FLAG_SET=1<<0,
		FLAG_GET=1<<1,
	};

	CG_Variable* find(const String& name_,int d=0);
	CG_Variable* find_capture(const String& name_,int d=0);
	CG_Variable* find_scope(const String& name_);

	CG_Variable* add_var(const String& name_,int type_);

	CG_Variable* add_param(const String& name_);
	CG_Variable* add_global(const String& name_);
	CG_Variable* add_system(const String& name_);
	CG_Variable* add_export(const String& name_);
	CG_Variable* add_import(const String& name_,const String& module_);

	CG_Variable* add_local(const String& name_);
	CG_Variable* add_capture(const String& name_,int target_,int index_);

	void enter2(CG_Block& v);
	void leave2();

	void enter1(CG_Block& v);
	void leave1();

	size_t param_num();
	size_t local_max();
	size_t capture_num();

	void add_b(int lv);
	void add_c(int lv);
	void set_b(int jp);
	void set_c(int jp);

	void update();

private:

	arr_1t<CG_Block*> m_nLocalStk;
	arr_1t<CG_Block*> m_nLocalGrp;

	size_t m_nParamNum;
	size_t m_nLocalMax;
	size_t m_nCaptureNum;

};


class DLLIMPEXP_EWA_BASE CG_LLVar
{
public:

	int add(const Variant& v)
	{
		aLLVar.push_back(v);
		return aLLVar.size() - 1;
	}

	int add()
	{
		aLLVar.push_back(Variant());
		return aLLVar.size() - 1;
	}

//	void get(arr_1t<Variant>& a)
//	{
//		m_aVariables.swap(a);
//	}
//
//private:
	arr_1t<Variant> aLLVar;
};


class DLLIMPEXP_EWA_BASE CG_Insts
{
public:

	//void get(arr_1t<XopInst>& o,arr_1t<tokInfo>& i)
	//{
	//	EW_ASSERT(aInsts.size() == aInfos.size());

	//	o.swap(aInsts);
	//	i.swap(aInfos);
	//}

	void emit(int op,int pm=0)
	{
		aInsts.push_back(XopInst(op,pm));
		aInfos.push_back(tokInfo());
	}

	void emit(int op, tokInfo& tk,int pm = 0)
	{
		aInsts.push_back(XopInst(op, pm));
		aInfos.push_back(tk);
	}

	void set_jp(int n,int d=-1)
	{
		if(d<0)
		{
			d=(int)aInsts.size();
		}

		aInsts[n].p1=d-n-1;
	}

	//void emit(int op,int pmc,int kep)
	//{
	//	aInsts.push_back(XopInst(op,pmc,kep));
	//	aInfos.push_back(tokInfo());
	//}

	void emit(int op,tokInfo& tk,int pmc,int kep)
	{
		aInsts.push_back(XopInst(op,pmc,kep));
		aInfos.push_back(tk);
	}	

	int size()
	{
		return (int)aInsts.size();
	}


//private:
	arr_1t<XopInst> aInsts;
	arr_1t<tokInfo> aInfos;
};


class DLLIMPEXP_EWA_BASE CG_CList
{
public:

	int find(const String& s)
	{
		int id=m_aVariables.find1(s);
		return id>=0?m_aVariables.get(id).second.first:-1;
	}

	void add(const String& s,int n,int d)
	{
		m_aVariables[s].first=n;
		m_aVariables[s].second=d;
	}

	typedef indexer_map<String,std::pair<int,int> >::iterator iterator;

	iterator begin(){return m_aVariables.begin();}
	iterator end(){return m_aVariables.end();}

	void swap(CG_CList& o){m_aVariables.swap(o.m_aVariables);}

private:
	indexer_map<String,std::pair<int,int> > m_aVariables;
	
};


class DLLIMPEXP_EWA_BASE CG_Array
{
public:

	class arr_info
	{
	public:
		arr_1t<int> info;
		size_t parm;
		size_t tags;

		arr_info(size_t n)
		{
			info.resize(n);
			parm=0;
			tags=0;
		}
	};

	void enter(size_t n)
	{
		m_aArrayOps.push_back(arr_info(n));
	}

	void leave()
	{
		m_aArrayOps.pop_back();
	}

	void leave(arr_1t<int>& v)
	{
		if(m_aArrayOps.back().tags==0)
		{
			v.clear();
		}
		else
		{
			m_aArrayOps.back().info.swap(v);
		}
		m_aArrayOps.pop_back();		
	}

	void parm(size_t d)
	{
		if(m_aArrayOps.empty())
		{
			return;
		}
		m_aArrayOps.back().parm=d;
	}
	
	bool mark()
	{
		if(m_aArrayOps.empty())
		{
			return false;
		}
		m_aArrayOps.back().tags++;
		m_aArrayOps.back().info[m_aArrayOps.back().parm]++;
		return true;
	}

	bool test()
	{
		if(m_aArrayOps.empty()) return false;
		return m_aArrayOps.back().info[m_aArrayOps.back().parm]!=0;
	}

private:
	arr_1t<arr_info> m_aArrayOps;
};

class CG_Flags
{
public:

	void enter(){m_aFlags.push_back(0);}
	void leave(){m_aFlags.pop_back();}
	void leave(int& n){n=m_aFlags.back();m_aFlags.pop_back();}


	bool touch()
	{
		if(m_aFlags.empty()) return false;
		m_aFlags.back()++;
		return true;
	}

private:
	arr_1t<int> m_aFlags;
};



class DLLIMPEXP_EWA_BASE StackFrameInfo : public ObjectData
{
public:
	StackFrameInfo(CodeGen& g,TNode_val_function& f):cg_tnode(f),cg_local(g)
	{
		cg_tnode.flags.add(TNode_val_function::FLAG_SELF_USED);
	}

	TNode_val_function& cg_tnode;
	CG_Local cg_local;

	size_t local_max();
};

class DLLIMPEXP_EWA_BASE StackFrameInfoAnalyzer : public StackFrameInfo
{
public:
	StackFrameInfoAnalyzer(CodeGen& g,TNode_val_function& f):StackFrameInfo(g,f){}

	CG_Flags cg_array;

};


class DLLIMPEXP_EWA_BASE StackFrameInfoCodeGen : public StackFrameInfo
{
public:
	StackFrameInfoCodeGen(CodeGen& g,TNode_val_function& f)
		:StackFrameInfo(g,f)
	{

	}

	
	CG_Insts cg_insts;
	CG_LLVar cg_llvar;
	bst_set<String> cg_imports;
	
};

class TNodeVisitorCG_Analyzer;

class DLLIMPEXP_EWA_BASE CG_VariableGlobal : public CG_Variable
{
public:
	CG_VariableGlobal() :CG_Variable(VAR_BREAKER){}

	~CG_VariableGlobal()
	{
		clear();
	}

	indexer_map<String, CG_Variable*> varmap;

	CG_Variable* find_var(const String& s)
	{
		int id = varmap.find1(s);
		return id < 0 ? NULL : varmap.get(id).second;
	}

	void add(CG_Variable* q)
	{
		CG_Variable* &q2 = varmap[q->name];
		if (q2 && q2->type == q->type && q2->name == q->name && q2->module == q->module)
		{
			delete q;
			return;
		}

		q2 = q;
		q->next = next;
		next = q;
	}

	void add_global(const String& s)
	{
		CG_Variable* q = new CG_Variable(VAR_GLOBAL, s);
		add(q);
	}

	void add_system(const String& s)
	{
		CG_Variable* q = new CG_Variable(VAR_SYSTEM, s);
		add(q);
	}

	void add_import(const String& s,const String& module)
	{
		CG_Variable* q = new CG_Variable(VAR_IMPORT, s);
		q->module = module;
		add(q);
	}

	void clear()
	{
		varmap.clear();
		while (next)
		{
			CG_Variable* q = next;
			next=q->next;
			delete q;
		}
	}

	void add_globals(const VariantTable& tb1);

};


class DLLIMPEXP_EWA_BASE CodeGen
{

	arr_1t<DataPtrT<StackFrameInfoCodeGen> > cg_frame;
	DataPtrT<TNode_statement_list> cg_pstmt;
	DataPtrT<CallableCode> cg_pinst;

	arr_1t<TNode*> cg_nodes;

public:

	friend class CG_Local;
	friend class TNodeVisitorCG;
	friend class TNodeVisitorCG_Analyzer;
	friend class TNodeVisitorCG_AnalyzerRValue;

	CG_VariableGlobal cg_global;
	CG_VariableGlobal cg_system;

	CG_Variable* find_var(const String& s);

	bool prepare(const String& ss, int t);

	StackFrameInfoCodeGen& back(){return *cg_frame.back();}

	String module;

	bst_set<String> cg_exports;


	CodeGen();


	void clear();

	DataPtrT<CallableCode> get(){return cg_pinst;}

	bool parse(const String& p);
	bool parse(TNode_statement_list* q);
	
	void kerror(const String& s);
	void kerror_undefined_variable(const String& s);

	void stackframe_enter(TNode_val_function* node);
	void stackframe_leave();


	void emit_push_nil(int n=1)
	{
		emit(XOP_PUSH_NIL,n);
	}

	void emit_push(int32_t v)
	{
		emit(XOP_PUSH_INT,v);
	}

	void emit_push(int64_t v)
	{
		emit(XOP_PUSH_INT,v);
	}

	void emit_push(bool v)
	{
		emit(XOP_PUSH_BOOL,v?1:0);
	}

	void emit_push(double v)
	{
		Variant d;d.reset(v);
		emit(XOP_GET_LLVAR,back().cg_llvar.add(d));
	}

	void emit_push(const String& v)
	{
		Variant d;d.reset(v);
		emit(XOP_GET_LLVAR,back().cg_llvar.add(d));
	}

	void emit_push(const char* v)
	{
		Variant d;d.reset(v);
		emit(XOP_GET_LLVAR,back().cg_llvar.add(d));
	}

	void emit_push(CallableData* v)
	{
		Variant d;d.reset(v);
		emit(XOP_GET_LLVAR,back().cg_llvar.add(d));
	}

	template<typename T>
	void emit_push(const DataPtrT<T>& v)
	{
		emit(XOP_GET_LLVAR,back().cg_llvar.add((CallableData*)v.get()));
	}

	void emit_push(const Variant& v)
	{
		emit(XOP_GET_LLVAR,back().cg_llvar.add(v));
	}

	int emit(int op,int p1=0)
	{
		back().cg_insts.emit(op, cg_nodes.back()->token, p1);
		return back().cg_insts.size()-1;
	}

	int emit(int op,int p1,int p2)
	{
		back().cg_insts.emit(op, cg_nodes.back()->token, p1, p2);
		return back().cg_insts.size()-1;
	}

	int inst_pos()
	{
		return ((int)back().cg_insts.size())-1;
	}

	int inst_end()
	{
		return (int)back().cg_insts.size();
	}

	void set_jp(int n,int d=-1)
	{
		back().cg_insts.set_jp(n,d);		
	}

	void emit_call(int pmc,int kep)
	{
		emit(XOP_CALL,pmc,kep);
	}

	void emit_this_call(int pmc,int kep)
	{
		emit(XOP_THIS_CALL,pmc,kep);
	}

	void emit_call_n(int pmc,int kep)
	{
		emit(XOP_CALL,pmc,kep);
	}

	bool is_library(const String& s);

	void emit_getsys(const String& s);
	void emit_getvar(const String& s);
	void emit_setvar(const String& s);

	void emit_setref(const String& s);

	void emit_getidx(const String& s)
	{
		Variant r;r.reset(s);
		int d=back().cg_llvar.add(r);
		emit(XOP_GET_INDEX,d);
		return;
	}

	void emit_setidx(const String& s)
	{
		Variant r;r.reset(s);
		int d=back().cg_llvar.add(r);
		emit(XOP_SET_INDEX,d);
		return;
	}


	DataPtrT<CallableSource> cg_source;

	BitFlags flags;

	enum
	{

		FLAG_SAVE_TEMP			=1<<1,
		FLAG_SHOW_RESULTS		=1<<2,
		FLAG_IMPLICIT_GLOBAL	=1<<3,
		FLAG_IMPLICIT_LOCAL		=1<<4,
	};



};


EW_LEAVE

#endif
