
#include "ewa_base/scripting/codegen.h"
#include "ewa_base/scripting/codegen_generator.h"
#include "ewa_base/scripting/codegen_analyzer.h"
#include "ewa_base/scripting/parser_nodes.h"
#include "ewa_base/scripting/executor.h"
#include "ewa_base/scripting/parser.h"

#include "ewa_base/scripting/callable_class.h"

EW_ENTER

CG_Local::CG_Local(CodeGen& cgen_):cgen(cgen_)
{
	m_nLocalMax=0;
	m_nCaptureNum=0;
	m_nParamNum = 0;
}

size_t CG_Local::param_num()
{
	return m_nParamNum;
}

size_t CG_Local::local_max()
{
	return m_nLocalMax;
}

size_t CG_Local::capture_num()
{
	return m_nCaptureNum;
}

CG_Variable* CG_Local::find_scope(const String& name_)
{
	if(m_nLocalStk.empty())
	{
		return NULL;
	}

	for(CG_Variable* q=m_nLocalStk.back()->next;q!=m_nLocalStk.back()->tail;q=q->next)
	{
		if (q->name == name_)
		{
			return q;
		}

	}

	return NULL;
}

CG_Variable* CG_Local::find(const String& name_,int d)
{

	if(m_nLocalStk.empty())
	{
		return NULL;
	}

	CG_Variable* q=m_nLocalStk.back()->next;
	while(q)
	{
		if(q->name==name_)
		{
			q->flags.add(d);
			return q;
		}
		q=q->next;
	}

	return NULL;

}

CG_Variable* CG_Local::find_capture(const String& name_,int d)
{
	if(m_nLocalStk.empty())
	{
		return NULL;
	}

	CG_Variable* q=m_nLocalStk.back()->next;
	while(q)
	{
		if(q->name==name_)
		{
			if(q->type==VAR_LOCAL)
			{
				if(!q->flags.get(CG_Variable::FLAG_CAPTURED))
				{
					if(q->index>0)
					{
						// 函数参数被capture，改名并在target中增加原名变量
						// 在代码生成时将参数的值移动到target中
						CG_Variable* q2=new CG_Variable(*q);
						q2->index=-1;
						q->name="@@"+q->name;

						q2->next=q->next;
						q->next=q2;
						q=q2;
					}

					q->flags.add(CG_Variable::FLAG_INITIALIZED|CG_Variable::FLAG_TOUCHED|CG_Variable::FLAG_CAPTURED);
					q->index=m_nCaptureNum++;
					find("#target",d);
				}
				return q;
			}
			else if(q->type==VAR_GLOBAL)
			{
				q->flags.add(CG_Variable::FLAG_TOUCHED);
				return q;
			}
			else if(q->type==VAR_SYSTEM)
			{
				q->flags.add(CG_Variable::FLAG_TOUCHED);
				return q;
			}
			else if(q->type==VAR_EXPORT)
			{
				q->flags.add(CG_Variable::FLAG_TOUCHED);
				return q;
			}
			else if(q->type==VAR_IMPORT)
			{
				q->flags.add(CG_Variable::FLAG_TOUCHED);
				return q;
			}
			else
			{
				cgen.kerror("unknown_type");
			}

		}
		q=q->next;
	}

	return NULL;
}

CG_Variable* CG_Local::add_var(const String& name_,int type_)
{
	if(find_scope(name_)!=NULL)
	{
		cgen.kerror(String::Format("variable %s already defined",name_));
	}

	CG_Variable* q=new CG_Variable(type_,name_);
	m_nLocalStk.back()->insert(q);
	return q;
}

CG_Variable* CG_Local::add_local(const String& name_)
{
	return add_var(name_,VAR_LOCAL);
}

CG_Variable* CG_Local::add_global(const String& name_)
{
	return add_var(name_,VAR_GLOBAL);
}

CG_Variable* CG_Local::add_system(const String& name_)
{
	return add_var(name_,VAR_SYSTEM);
}

CG_Variable* CG_Local::add_export(const String& name_)
{
	return add_var(name_,VAR_EXPORT);
}

CG_Variable* CG_Local::add_import(const String& name_, const String& module_)
{
	CG_Variable* q=add_var(name_, VAR_IMPORT);
	if (q) q->module = module_;
	return q;
}

CG_Variable* CG_Local::add_param(const String& name_)
{

	CG_Variable* q=add_var(name_,VAR_LOCAL);
	if(q)
	{
		q->index=++m_nParamNum;
		q->flags.add(CG_Variable::FLAG_INITIALIZED|CG_Variable::FLAG_TOUCHED);
	}

	return q;
}

CG_Variable* CG_Local::add_capture(const String& name_,int target_,int index_)
{
	CG_Variable* q=add_var(name_,VAR_CAPTURE);
	if(q)
	{
		q->index=index_;
		q->target=target_;
	}

	return q;
}

void CG_Local::enter2(CG_Block& v)
{
	if(m_nLocalStk.empty())
	{
		EW_ASSERT(v.parent==NULL);
	}
	else
	{
		EW_ASSERT(v.parent==m_nLocalStk.back());
	}


	m_nLocalStk.push_back(&v);
}

void CG_Local::leave2()
{
	m_nLocalStk.pop_back();
}


void CG_Local::add_r()
{

	CG_Block* p=m_nLocalStk.back();
	while(p)
	{
		//if(p==m_nLocalGrp.back()) break;

		if(p->flags.get(CG_Block::FLAG_EXCEPT))
		{
			cgen.emit(XOP_TRY_LEAVE);
		}

		p=p->parent;
	}
}

void CG_Local::add_b(int lv)
{
	if(m_nLocalStk.empty())
	{
		Exception::XError("invalid break");
	}

	CG_Block* p=m_nLocalStk.back();
	while(p)
	{
		if(p->flags.get(CG_Block::FLAG_BREAK_ABLE))
		{
			if(--lv==0)
			{
				cgen.emit(XOP_JUMP,0);
				p->m_aControlB.push_back(cgen.inst_pos());
			}
		}
		else if(p->flags.get(CG_Block::FLAG_EXCEPT))
		{
			cgen.emit(XOP_TRY_LEAVE);
		}
		p=p->parent;
	}

	if(m_nLocalStk.empty())
	{
		Exception::XError("invalid break");
	}
}

void CG_Local::add_c(int lv)
{
	if(m_nLocalStk.empty())
	{
		Exception::XError("invalid continue");
	}

	CG_Block* p=m_nLocalStk.back();
	while(p)
	{
		if(p->flags.get(CG_Block::FLAG_CONTINUE_ABLE))
		{
			if(--lv==0)
			{
				cgen.emit(XOP_JUMP,0);
				p->m_aControlC.push_back(cgen.inst_pos());
				return;
			}
		}
		else if(p->flags.get(CG_Block::FLAG_EXCEPT))
		{
			cgen.emit(XOP_TRY_LEAVE);
		}

		p=p->parent;
	}

	if(m_nLocalStk.empty())
	{
		Exception::XError("invalid continue");
	}
}

void CG_Local::set_b(int jp)
{
	if(m_nLocalStk.empty())
	{
		Exception::XError("invalid set_b");
	}

	CG_Block* p=m_nLocalStk.back();
	if(!p->flags.get(CG_Block::FLAG_BREAK_ABLE))
	{
		Exception::XError("invalid set_b");
	}

	for(size_t i=0;i<p->m_aControlB.size();i++)
	{
		cgen.set_jp(p->m_aControlB[i],jp);
	}
}

void CG_Local::set_c(int jp)
{
	if(m_nLocalStk.empty())
	{
		Exception::XError("invalid set_c");
	}

	CG_Block* p=m_nLocalStk.back();
	if(!p->flags.get(CG_Block::FLAG_CONTINUE_ABLE))
	{
		Exception::XError("invalid set_c");
	}

	for(size_t i=0;i<p->m_aControlC.size();i++)
	{
		cgen.set_jp(p->m_aControlC[i],jp);
	}
}

void CG_Local::update()
{
	m_nLocalMax=0;

	for(size_t i=0;i<m_nLocalGrp.size();i++)
	{
		CG_Block& blk(*m_nLocalGrp[i]);
		size_t nShift=0;

		for(CG_Variable* q=blk.tail;q;q=q->next)
		{
			if(q->type==VAR_LOCAL && q->index>0)
			{
				nShift=q->index;
				break;
			}
		}

		arr_1t<CG_Variable*> vars;

		for(CG_Variable* q=blk.next;q!=blk.tail;q=q->next)
		{
			if(q->type!=VAR_LOCAL)
			{
				continue;
			}
			else if(q->flags.get(CG_Variable::FLAG_CAPTURED))
			{
				q->type=VAR_CAPTURE;
				q->target=-1;
				continue;
			}

			if(!q->flags.get(CG_Variable::FLAG_INITIALIZED|CG_Variable::FLAG_TOUCHED))
			{
				q->index=-1;
				continue;
			}
			vars.push_back(q);
		}


		for(arr_1t<CG_Variable*>::reverse_iterator it=vars.rbegin();it!=vars.rend();++it)
		{
			if((*it)->type==VAR_LOCAL)
			{
				(*it)->index=++nShift;
			}
		}

		if(nShift>m_nLocalMax) m_nLocalMax=nShift;
	}
}

void CG_Local::enter1(CG_Block& v)
{
	EW_ASSERT(v.parent==NULL);
	EW_ASSERT(v.next==NULL);
	EW_ASSERT(v.tail==NULL);


	if(!m_nLocalStk.empty())
	{
		v.parent=m_nLocalStk.back();
		v.next = v.tail = v.parent;
	}

	m_nLocalGrp.push_back(&v);
	m_nLocalStk.push_back(&v);
}

void CG_Local::leave1()
{
	m_nLocalStk.pop_back();
}

size_t StackFrameInfo::local_max()
{
	size_t n=cg_local.local_max()+1;
	return n;
}

void CodeGen::stackframe_enter(TNode_val_function* node)
{
	cg_frame.push_back(new StackFrameInfoCodeGen(*this,*node));
	CG_Local& cg_local(back().cg_local);
	cg_local.enter2(node->tScope);

	size_t n=node->capture_num();

	if(n>0)
	{
		emit_push((int)n);
		emit_getsys("#new_target");
		emit_call(1,1);
		emit_setvar("#target");

		// 处理函数参数被capture，把参数移动到target中
		CG_Variable* q=node->tScope.next;
		while(q!=node->tScope.tail)
		{
			const String& name(q->name);
			if(name.substr(0,2)=="@@")
			{
				emit(XOP_GET_LOCAL,q->index);
				emit_getvar("#target");
				q=q->next;
				EW_ASSERT(q->target<0);
				EW_ASSERT(q->index<(int)n);
				EW_ASSERT(q->name==name.substr(2));
				emit(XOP_SET_INDEX_N_REF,q->index);
			}
			q=q->next;
		}
	}

}


void CodeGen::stackframe_leave()
{
	emit(XOP_HALT);

	cg_pinst.reset(new CallableCode);

	back().cg_local.leave2();

	DataPtrT<StackFrameInfoCodeGen> pbi(cg_frame.back());
	cg_frame.pop_back();

	cg_pinst->pSource = cg_source;
	cg_pinst->tok_beg=pbi->cg_tnode.tok_beg;
	cg_pinst->tok_end=pbi->cg_tnode.tok_end;


	cg_pinst->aLLVar.swap(pbi->cg_llvar.aLLVar);
	cg_pinst->aInsts.swap(pbi->cg_insts.aInsts);
	cg_pinst->aInfos.swap(pbi->cg_insts.aInfos);
	cg_pinst->aDepends.swap(pbi->cg_imports);

	cg_pinst->nParam=pbi->cg_tnode.param_num();
	cg_pinst->nLocal=pbi->cg_tnode.local_max();
	cg_pinst->flags	=pbi->cg_tnode.flags;

	if (pbi->cg_tnode.func_name)
	{
		cg_pinst->SetName(pbi->cg_tnode.func_name->token.word);
	}
	cg_pinst->__set_helpdata(pbi->cg_tnode.m_sHelp);

	if(!cg_frame.empty())
	{
		emit_push(cg_pinst);

		if(pbi->cg_tnode.aCaptured.empty())
		{
			// no captured target, function
			return;
		}

		// closure

		// find captured target shift
		String s0=String::Format("#target.%d",(int)pbi->cg_tnode.aCaptured[0]);
		CG_Variable* q0=pbi->cg_tnode.tScope.next;
		for(;q0;q0=q0->next)
		{
			if(q0->name==s0)
			{
				break;
			}
		}
		if(!q0)
		{
			kerror("invalid target shift");
		}

		cg_pinst->nShift=q0->index;

		// create closure with captured targets
		for(size_t i=0;i<pbi->cg_tnode.aCaptured.size();i++)
		{
			size_t d=pbi->cg_tnode.aCaptured[i];
			CG_Variable* q=cg_frame[d]->cg_local.find("#target");
			int s=q->index;
			for(size_t j=d;j<cg_frame.size();j++)
			{
				s-=cg_frame[j]->cg_local.local_max();
			}
			emit(XOP_GET_LOCAL,s);
		}
		emit_getsys("#new_closure");
		emit_call(pbi->cg_tnode.aCaptured.size()+1,1);
	}
	else if (module != "")
	{
		CG_GGVar::current().find2(module);
		for (bst_set<String>::iterator it = cg_exports.begin(); it != cg_exports.end(); ++it)
		{
			CG_GGVar::current().find2(module + "." + (*it));
		}		
	}

}

void CodeGen::emit_getsys(const String& s)
{
	StackFrameInfoCodeGen& sfa(*cg_frame.back());
	Variant v; v.reset(s);
	emit(XOP_GET_GGVAR, sfa.cg_llvar.add(v));
}

bool CodeGen::is_library(const String& s)
{
	if (s.find('.') >= 0) return false;
	StackFrameInfoCodeGen& sfa(*cg_frame.back());
	CG_Variable* q=sfa.cg_local.find(s);
	if (!q) q = find_var(s);

	return q && q->type == VAR_SYSTEM && s!="_G";
}

CG_Variable* CodeGen::find_var(const String& s)
{
	CG_Variable* q;
	q = cg_global.find_var(s);
	if (q) return q;
	q = cg_system.find_var(s);
	if (q) return q;
	return CG_GGVar::current().find_var(s);
}

void CodeGen::emit_getvar(const String& s)
{
	StackFrameInfoCodeGen& sfa(*cg_frame.back());
	CG_Variable* q=sfa.cg_local.find(s);
	if (!q) q = find_var(s);

	if(q!=NULL)
	{
		if(q->type==VAR_LOCAL)
		{
			if(q->index<1)
			{
				System::DebugBreak();
			}
			emit(XOP_GET_LOCAL,q->index);
		}
		else if(q->type==VAR_CAPTURE)
		{
			CG_Variable* t;

			if(q->target<0)
			{
				t=sfa.cg_local.find("#target");
			}
			else
			{
				t=sfa.cg_local.find(String::Format("#target.%d",q->target));
			}

			if(!t||t->index<1)
			{
				kerror("invalid #target");
			}

			emit(XOP_GET_LOCAL,t->index);
			emit(XOP_GET_INDEX_N,q->index);
		}
		else if(q->type==VAR_GLOBAL)
		{
			Variant v;v.reset(s);
			emit(XOP_GET_TABLE,sfa.cg_llvar.add(v));
		}
		else if (q->type == VAR_SYSTEM)
		{
			emit_getsys(s);
		}
		else if (q->type == VAR_EXPORT)
		{
			if (module == "")
			{
				Variant v; v.reset(s);
				emit(XOP_GET_TABLE, sfa.cg_llvar.add(v));
			}
			else
			{
				String name = module + "." + s;

				//if (cg_frame.size() > 1)
				//{
				//	StackFrameInfoCodeGen& sfa(*cg_frame.back());
				//	sfa.cg_imports.insert(module);
				//}

				emit_getsys(name);
			}
		}
		else if (q->type == VAR_IMPORT)
		{
			StackFrameInfoCodeGen& sfa(*cg_frame.back());
			//sfa.cg_imports.insert(q->module);
			emit_getsys(q->module+"."+s);
		}
		else
		{
			kerror("invalid variable type");
		}

		return;

	}


	if(s=="temp")
	{
		emit(XOP_GET_SBASE,StackState1::SBASE_TEMP);
		return;
	}

	kerror_undefined_variable(s);

}

void CodeGen::emit_setvar(const String& s)
{

	CG_Variable* q;

	q=cg_frame.back()->cg_local.find(s);
	if (!q) q = find_var(s);

	if(q!=NULL)
	{
		if (q->flags.get(CG_Variable::FLAG_READONLY))
		{
			kerror("variable readonly");
		}
		else if(q->type==VAR_LOCAL)
		{
			emit(XOP_SET_LOCAL,q->index);
		}
		else if(q->type==VAR_CAPTURE)
		{
			CG_Variable* t;

			if(q->target<0)
			{
				t=cg_frame.back()->cg_local.find("#target");
			}
			else
			{
				t=cg_frame.back()->cg_local.find(String::Format("#target.%d",q->target));
			}

			if(!t||t->index<1)
			{
				kerror("invalid #target");
			}

			emit(XOP_GET_LOCAL,t->index);
			emit(XOP_SET_INDEX_N,q->index);
		}
		else if(q->type==VAR_GLOBAL)
		{
			Variant v;v.reset(s);
			emit(XOP_SET_TABLE,cg_frame.back()->cg_llvar.add(v));
		}
		else if (q->type == VAR_SYSTEM)
		{
			Variant v;v.reset(s);
			emit(XOP_SET_GGVAR,cg_frame.back()->cg_llvar.add(v));
		}
		else if (q->type == VAR_EXPORT)
		{
			if (module == "")
			{
				Variant v; v.reset(s);
				emit(XOP_SET_TABLE, cg_frame.back()->cg_llvar.add(v));
			}
			else
			{
				Variant v;v.reset(module+"."+s);
				emit(XOP_SET_GGVAR,cg_frame.back()->cg_llvar.add(v));
			}
		}
		else
		{
			kerror("invalid local");
		}

		return;
	}

	//StackFrameInfoCodeGen& sfa(*cg_frame.back());

	//int t = find_var(s);
	//if (t == 1)
	//{
	//	Variant v; v.reset(s);
	//	emit(XOP_SET_TABLE, sfa.cg_llvar.add(v));
	//	return;
	//}
	//else if (t == 2)
	//{
	//	kerror("cannot modifiy system var");
	//}

	if(s=="temp")
	{
		emit(XOP_SET_SBASE,StackState1::SBASE_TEMP);
		return;
	}

	kerror_undefined_variable(s);

}


void CodeGen::emit_setref(const String& s)
{

	CG_Variable* q;

	q=cg_frame.back()->cg_local.find(s);
	if(q!=NULL)
	{
		if(q->type==VAR_LOCAL)
		{
			emit(XOP_SET_LOCAL_REF, q->index);
		}
		else 
		{
			kerror("invalid local");
		}

		return;
	}
	
	kerror("invalid local");

}

CodeGen::CodeGen()
{
	flags.add(FLAG_SAVE_TEMP|FLAG_SHOW_RESULTS|FLAG_IMPLICIT_GLOBAL);
}


void CG_VariableGlobal::add_globals(const VariantTable& tb1)
{
	for (size_t i = 0; i < tb1.size(); i++)
	{
		add_global(tb1.get(i).first);
	}
}



void CodeGen::clear()
{
	cg_nodes.clear();
	cg_pinst.reset(NULL);
	cg_pstmt.reset(NULL);
	cg_frame.clear();

	cg_global.clear();
}


bool CodeGen::parse(TNode_statement_list* q)
{

	cg_pstmt.reset(q);

	TNodeVisitorCG_Analyzer		vis_phase1(*this);
	TNodeVisitorCG_Generator	vis_phase2(*this);

	DataPtrT<TNode_val_function> fun(new TNode_val_function);
	fun->func_body.reset(q);

	size_t n=q->aList.size();
	for(size_t i=0;i<n;i++)
	{
		TNode_statement_macro* pstmt=dynamic_cast<TNode_statement_macro*>(q->aList[i].get());
		if (!pstmt) continue;

		if (pstmt->token.word == "function")
		{
			if(fun->exp_list)
			{
				kerror("multiple #function");
			}

			fun->exp_list.reset(new TNode_variable_list);
			for (size_t j = 0; j<pstmt->param.size(); j++)
			{
				fun->exp_list->aList.append(new TNode_var(pstmt->param[j].word));
			}
			q->aList[i].reset(new TNode_statement_nil);
		}

	}

	try
	{
		vis_phase1.parse(fun.get());
		vis_phase2.parse(fun.get());
	}
	catch(std::exception& e)
	{
		this_logger().LogError(e.what());
		return false;
	}

	return cg_pinst.get()!=NULL;
}


bool CodeGen::parse(const String& p)
{
	cg_source.reset(new CallableSource(p));

	Parser parser;
	DataPtrT<TNode_statement_list> q=parser.parse(p);
	if(!q) return false;
	return parse(q.get());
}

void CodeGen::kerror(const String& s)
{
	if(!cg_nodes.empty())
	{
		tokItem &item(cg_nodes.back()->token);
		if(item.line>0)
		{
			Exception::XError(String::Format("CodeGen:%s at line:%d, pos:%d, token is %s.",s,item.line,item.cpos,item.word));
			return;
		}
	}

	Exception::XError(String::Format("CodeGen:%s",s));
}

void CodeGen::kerror_undefined_variable(const String& s)
{
	kerror(String::Format("undefined variable:%s",s));
}

void TNodeVisitorCG::kerror(const String& s)
{
	cgen.kerror(s);
}

bool CodeGen::prepare(const String& ss, int t)
{
	CodeGen& cgen(*this);

	try
	{

		if (t == Executor::DIRECT_STRING)
		{
			if (!cgen.parse(ss))
			{
				return false;
			}
		}
		else
		{

			StringBuffer<char> sb;
			if (!sb.load(ss))
			{
				return false;
			}

			if (!cgen.parse(sb))
			{
				return false;
			}
		}

		return true;
	}
	catch (std::exception& e)
	{
		this_logger().LogError(e.what());
		return false;
	}

}


EW_LEAVE

