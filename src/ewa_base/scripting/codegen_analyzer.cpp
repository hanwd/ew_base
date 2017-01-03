#include "ewa_base/scripting/codegen_visitor.h"
#include "ewa_base/scripting/codegen_analyzer.h"
#include "ewa_base/scripting/parser_nodes.h"

#include "ewa_base/scripting/executor.h"

EW_ENTER

bool TNodeVisitorCG_Analyzer::handle_var(const String& v)
{
	StackFrameInfoAnalyzer& sfa(*stk.back());

	if(v=="temp") return true;

	CG_Variable* q=sfa.cg_local.find(v,CG_Variable::FLAG_TOUCHED);
	if(q!=NULL)
	{
		return true;
	}

	for(int nf=(int)stk.size()-2;nf>=0;--nf)
	{
		q=stk[nf]->cg_local.find_capture(v,CG_Variable::FLAG_TOUCHED);

		if(q==NULL){}
		else if(q->type==VAR_LOCAL)
		{

			sfa.cg_local.add_capture(v,nf,q->index);

			if(sfa.cg_tnode.add_capture(nf))
			{
				CG_Variable* v=sfa.cg_local.find("#target_end");
				if(!v)
				{
					kerror("invalid #target_end");
				}

				CG_Variable* tn=new CG_Variable(VAR_LOCAL,String::Format("#target.%d",nf));
				tn->flags.add(CG_Variable::FLAG_TOUCHED);
				tn->next=v->next;

				v->next=tn;
			}

			return true;
		}
		else if(q->type==VAR_GLOBAL)
		{
			sfa.cg_local.add_global(v);
			return true;
		}
		else if(q->type==VAR_SYSTEM)
		{
			sfa.cg_local.add_system(v);
			return true;
		}
		else if (q->type == VAR_IMPORT)
		{
			sfa.cg_local.add_import(q->name, q->module);
			return true;
		}
		else if (q->type == VAR_EXPORT)
		{
			sfa.cg_local.add_export(q->name);
			return true;
		}
		else
		{
			Exception::XError("unknown var type");
		}
	}


	return false;
}

void TNodeVisitorCG_Analyzer::getvar(const String& v)
{

	if(handle_var(v))
	{
		return;
	}

	if (cgen.find_var(v)!=0)
	{
		return;
	}

	if (v == "_")
	{
		kerror("_ cannot be rvalue");
	}
	else
	{
		kerror("undefined variable:"+v);
	}
}

void TNodeVisitorCG_Analyzer::setvar(const String& v)
{

	if(handle_var(v))
	{
		return;
	}

	CG_Variable* q = cgen.find_var(v);

	if (!q){}
	else if(q->type == VAR_GLOBAL) return;
	else
	{
		kerror("system variable is readonly");
	}

	//int t = cgen.find_var(v);

	//if (t == 1){ return; }
	//else if (t == 2)
	//{
	//	kerror("system variable is readonly");
	//}
	

	if(cgen.flags.get(CodeGen::FLAG_IMPLICIT_GLOBAL))
	{
		define_var(v,VAR_GLOBAL);
		return;
	}

	if(cgen.flags.get(CodeGen::FLAG_IMPLICIT_LOCAL))
	{
		define_var(v,VAR_LOCAL);
		return;
	}

	kerror("undefined variable");
}

void TNodeVisitorCG_Analyzer::define_import(const String& name_, const String& module_)
{
	CG_Variable* q  = back().cg_local.add_import(name_,module_);
	if (q == NULL)
	{
		kerror("invalid variable name");
	}
	if (stk.size() == 1)
	{
		cgen.cg_system.add_import(name_,module_);
	}
}

void TNodeVisitorCG_Analyzer::define_var(const String& v,int t,int d)
{
	if (v == "_") return;

	if(t==VAR_DEFAULT)
	{
		if(back().cg_local.find_scope(v))
		{
			return;
		}

		if (cgen.cg_exports.empty())
		{
			t=VAR_GLOBAL;
		}
		else
		{
			cgen.cg_exports.insert(v);
			t=VAR_EXPORT;
		}

	}

	CG_Variable* q=NULL;

	if(t==VAR_LOCAL)
	{
		q=back().cg_local.add_local(v);
	}
	else if(t==VAR_GLOBAL)
	{
		if (cgen.cg_exports.empty())
		{
			q=back().cg_local.add_global(v);
		}
		else
		{
			kerror("cannot define global in module");
		}

	}
	else if(t==VAR_CAPTURE)
	{
		q=back().cg_local.add_capture(v,t,d);
	}
	else if(t==VAR_SYSTEM)
	{
		q=back().cg_local.add_system(v);
		if (q && stk.size() == 1)
		{
			cgen.cg_system.add_system(v);
		}
	}
	else if(t==VAR_EXPORT)
	{
		q=back().cg_local.add_export(v);
	}
	else
	{
		kerror("invalid variable type");
	}

	if(q==NULL)
	{
		kerror("invalid variable name");
	}
}


VisReturnType TNodeVisitorCG_AnalyzerRValue::visit(TNode_statement_macro* node, VisExtraParam visp)
{
	if (node->token.word == "import")
	{
		if (node->extra.size() != 1)
		{
			kerror("invalid syntax of #import");
		}
		if (node->param.empty())
		{			
			avis.define_var(node->extra[0].word,VAR_SYSTEM);
		}
		else
		{
			for (size_t i = 0; i < node->param.size(); i++)
			{
				avis.define_import(node->param[i].word, node->extra[0].word);
			}
		}

		return 0;
	}
	else if (node->token.word == "export")
	{
		for (size_t i = 0; i < node->extra.size(); i++)
		{
			avis.define_var(node->extra[i].word, VAR_EXPORT);
			cgen.cg_exports.insert(node->extra[i].word);
		}
		return 0;
	}
	else
	{
		return basetype::visit(node, visp);
	}
}

void TNodeVisitorCG_Analyzer::stackframe_enter(TNode_val_function* node)
{
	TNode_var* pvar=node->func_name?node->func_name->to_var():NULL;
	if(pvar)
	{
		define_var(pvar->token.word,VAR_DEFAULT);
	}
	else if(node->func_name)
	{
		node->func_name->accept(lvis,1);
	}

	stk.push_back(new StackFrameInfoAnalyzer(cgen,*node));

	StackFrameInfoAnalyzer& sfa(*stk.back());

	sfa.cg_local.enter1(node->tScope);

	if(node->exp_list)
	{
		size_t n=node->exp_list->aList.size();
		sfa.cg_tnode.param_num(n);
		for(size_t i=0;i<n;i++)
		{
			TNode_var* pvar=node->exp_list->aList[i].get();
			if(!sfa.cg_local.add_param(pvar->token.word))
			{
				kerror("invalid param");
			}
		}

		if(node->exp_list->dot3_pos()>0)
		{
			node->flags.add(TNode_val_function::FLAG_WITH_DOT3);
		}
		else
		{
			sfa.cg_local.add_local("...");
		}
	}
	else
	{
		sfa.cg_tnode.param_num(0);
		sfa.cg_local.add_local("...");
	}

	sfa.cg_local.add_local("this");
	sfa.cg_local.add_local("self");
	sfa.cg_local.add_local("#target");
	sfa.cg_local.add_local("#target_end");
}

void TNodeVisitorCG_Analyzer::stackframe_leave()
{

	StackFrameInfoAnalyzer& sfa(*stk.back());

	CG_Variable* q;

	q=sfa.cg_local.find("...");
	if(q && q->flags.get(CG_Variable::FLAG_INITIALIZED|CG_Variable::FLAG_TOUCHED))
	{
		sfa.cg_tnode.flags.add(TNode_val_function::FLAG_DOT3_USED);
		if(!sfa.cg_tnode.flags.get(TNode_val_function::FLAG_WITH_DOT3))
		{
			sfa.cg_tnode.param_num(sfa.cg_tnode.param_num()+1);
		}
	}


	q=sfa.cg_local.find("this");
	if(q && q->flags.get(CG_Variable::FLAG_INITIALIZED|CG_Variable::FLAG_TOUCHED))
	{
		sfa.cg_tnode.flags.add(TNode_val_function::FLAG_THIS_USED);
	}

	q=sfa.cg_local.find("self");
	if(q && q->flags.get(CG_Variable::FLAG_INITIALIZED|CG_Variable::FLAG_TOUCHED))
	{
		sfa.cg_tnode.flags.add(TNode_val_function::FLAG_SELF_USED);
	}

	q=sfa.cg_local.find("#target");
	if(q && q->flags.get(CG_Variable::FLAG_INITIALIZED|CG_Variable::FLAG_TOUCHED))
	{

	}


	sfa.cg_local.leave1();
	sfa.cg_local.update();

	sfa.cg_tnode.local_max(sfa.local_max());
	sfa.cg_tnode.capture_num(sfa.cg_local.capture_num());

	stk.pop_back();
}


TNodeVisitorCG_AnalyzerUnit::TNodeVisitorCG_AnalyzerUnit(TNodeVisitorCG_Analyzer& cg_)
:TNodeVisitorCG(cg_.cgen),avis(cg_),lvis(cg_.lvis),rvis(cg_.rvis)
{

}

TNodeVisitorCG_AnalyzerLValue::TNodeVisitorCG_AnalyzerLValue(TNodeVisitorCG_Analyzer& cg_)
:TNodeVisitorCG_AnalyzerUnit(cg_)
{

}

TNodeVisitorCG_AnalyzerRValue::TNodeVisitorCG_AnalyzerRValue(TNodeVisitorCG_Analyzer& cg_)
:TNodeVisitorCG_AnalyzerUnit(cg_)
{

}


VisReturnType TNodeVisitorCG_AnalyzerUnit::visit(TNode_expression_call* node,VisExtraParam visp)
{

	TNode_var* pvar=node->tbl->to_var();
	if(pvar && pvar->token.word=="pack" && node->exp_list->dot3_pos()==1)
	{
		return replace_by(new TNode_var("..."));
	}

	if(pvar && pvar->token.word==":")
	{
		if(node->flags.get(TNode_expression::FLAG_SUB_INDEX))
		{
			pvar->token.word="#colon_data";
		}
		else
		{
			pvar->token.word="colon";
		}
	}

	if(node->cls)
	{
		visit_it<TNode_expression>(node->cls,visp);
	}


	visit_it<TNode_expression>(node->tbl, visp);
	node->exp_list->accept(rvis,visp);

	return def_value();

}

VisReturnType TNodeVisitorCG_AnalyzerUnit::visit(TNode_expression_list* node,VisExtraParam visp)
{
	for(TNode_expression_list::nodelist::iterator it=node->aList.begin();it!=node->aList.end();++it)
	{
		visit_it<TNode_expression>(*it,visp);
	}
	return def_value();
}

VisReturnType TNodeVisitorCG_AnalyzerUnit::visit(TNode_braket* node,VisExtraParam visp)
{
	if(node->flags.get(TNode_braket::FLAG_WITH_BRAKET2))
	{
		if(node->row_size>0)
		{
			int c=node->row_size;
			int n=node->exp_list->aList.size();
			if(n%c!=0)
			{
				cgen.kerror("invalid matrix");
			}
			int r=n/c;


			TNode_expression_call* trans = new TNode_expression_call(node->exp_list->token);
			trans->tbl.reset(new TNode_var("trans"));
			trans->exp_list.reset(new TNode_expression_list);

			DataPtrT<TNode_expression> qe(trans);

			TNode_expression_call* reshape = new TNode_expression_call(node->exp_list->token);

			trans->exp_list->aList.append(reshape);

			reshape->tbl.reset(new TNode_var("reshape"));
			reshape->exp_list.reset(new TNode_expression_list);

			tokItem t1,t2;
			t1.type=t2.type=TOK_INTEGER;
			t1.word<<r;
			t2.word<<c;


			TNode_expression_call* kc = new TNode_expression_call(node->exp_list->token);

			reshape->exp_list->aList.append(kc);
			reshape->exp_list->aList.append(new TNode_val_integer(t2));
			reshape->exp_list->aList.append(new TNode_val_integer(t1));


			kc->tbl.reset(new TNode_var("pack"));
			kc->exp_list=node->exp_list;

			visit_it<TNode_expression>(qe,visp);
			return replace_by(qe.get());


		}
		else if(node->exp_list->dot3_pos()==1)
		{
			avis.getvar("...");
			return replace_by(new TNode_var("..."));
		}
		else
		{
			TNode_expression_call* kc = new TNode_expression_call(node->exp_list->token);
			DataPtrT<TNode_expression> qe(kc);
			kc->tbl.reset(new TNode_var("pack"));
			kc->exp_list=node->exp_list;

			visit_it<TNode_expression>(qe,visp);
			return replace_by(qe.get());
		}
	}

	basetype::visit(node,visp);

	return def_value();
}

VisReturnType TNodeVisitorCG_AnalyzerUnit::visit(TNode_expression_dot* node,VisExtraParam visp)
{
	return rvis.visit_it<TNode_expression>(node->tbl,visp);
}

VisReturnType TNodeVisitorCG_AnalyzerUnit::visit(TNode_expression_arr* node,VisExtraParam visp)
{
	avis.back().cg_local.enter1(node->tScope);

	avis.define_var("#arr",VAR_LOCAL);
	avis.define_var("begin",VAR_LOCAL);
	avis.define_var("end",VAR_LOCAL);

	visit_it<TNode_expression>(node->tbl,visp);

	size_t n=node->exp_list->aList.size();
	node->exp_flag.resize(n);

	for(size_t i=0;i<n;i++)
	{
		avis.back().cg_array.enter();
		node->exp_list->aList[i]->flags.add(TNode_expression::FLAG_SUB_INDEX);
		rvis.visit_it<TNode_expression>(node->exp_list->aList[i],0);
		avis.back().cg_array.leave(node->exp_flag[i]);
	}

	int d=0;
	for(size_t i=0;i<n;i++)
	{
		d+=node->exp_flag[i];
	}
	if(d==0)
	{
		node->exp_flag.clear();
	}


	avis.back().cg_local.leave1();

	return def_value();
}

VisReturnType TNodeVisitorCG_AnalyzerLValue::visit(TNode* node,VisExtraParam)
{
	avis.kerror("invalid lvalue");
	return def_value();
}

VisReturnType TNodeVisitorCG_AnalyzerLValue::visit(TNode_var* node,VisExtraParam)
{

	String& name(node->token.word);

	if(name=="")
	{
		return def_value();
	}
	else if(name=="...")
	{
		node->flags.del(TNode_var::FLAG_LOCAL|TNode_var::FLAG_GLOBAL);

		avis.setvar("...");
		return def_value();
	}
	else if(name=="this"||name=="begin"||name=="end")
	{
		avis.kerror(String::Format("%s cannot be lvalue",name));
	}
	else if(node->flags.get(TNode_var::FLAG_LOCAL))
	{
		avis.define_var(node->token.word,VAR_LOCAL);
	}
	else if(node->flags.get(TNode_var::FLAG_GLOBAL))
	{
		avis.define_var(node->token.word,VAR_GLOBAL);
		return def_value();
	}

	if (node->token.word == "_")
	{
		node->token.word = "";
	}
	else
	{
		avis.setvar(node->token.word);
	}

	return def_value();

}


//VisReturnType TNodeVisitorCG_AnalyzerRValue::visit(TNode_expression_op2* node, VisExtraParam pm)
//{
//	return basetype::visit(node, pm);
//}

VisReturnType TNodeVisitorCG_AnalyzerRValue::visit(TNode* node,VisExtraParam)
{
	(void)node;

	return def_value();
}

VisReturnType TNodeVisitorCG_AnalyzerRValue::visit(TNode_expression_op1* node,VisExtraParam p)
{
	if(node->param[0]) visit_it<TNode_expression>(node->param[0],p);
	return def_value();
}

VisReturnType TNodeVisitorCG_AnalyzerRValue::visit(TNode_expression_op2* node,VisExtraParam p)
{
	if(node->param[1]) visit_it<TNode_expression>(node->param[1],p);
	if(node->param[0]) visit_it<TNode_expression>(node->param[0],p);
	return def_value();
}

VisReturnType TNodeVisitorCG_AnalyzerRValue::visit(TNode_item_select* node,VisExtraParam p)
{
	for (size_t i = 0; i < node->exp_list->aList.size(); i++)
	{
		if (node->exp_list->aList[i]) visit_it<TNode_expression>(node->exp_list->aList[i], p);
	}
	return def_value();
}



VisReturnType TNodeVisitorCG_AnalyzerRValue::visit(TNode_statement_try* node,VisExtraParam)
{
	node->tScope_try.flags.add(CG_Block::FLAG_EXCEPT);

	avis.back().cg_local.enter1(node->tScope_try);
	node->pBlock_try->accept(*this,0);
	avis.back().cg_local.leave1();

	avis.back().cg_local.enter1(node->tScope_catch);

	if(!node->exp_list)
	{
		node->exp_list.reset(new TNode_variable_list);
	}
	else
	{
		TNode_variable_list::nodelist::iterator it=node->exp_list->aList.begin();
		for(;it!=node->exp_list->aList.end();++it)
		{
			TNode_var* p=(TNode_var*)(*it).get();
			if(p->token.word!="...")
			{
				avis.define_var(p->token.word,VAR_LOCAL);
			}
		}
	}

	node->pBlock_catch->accept(*this,0);
	avis.back().cg_local.leave1();

	if(node->pBlock_finally)
	{
		avis.back().cg_local.enter1(node->tScope_finally);
		node->pBlock_finally->accept(*this,0);
		avis.back().cg_local.leave1();
	}

	return def_value();
}

VisReturnType TNodeVisitorCG_AnalyzerRValue::visit(TNode_statement_loop* node,VisExtraParam)
{
	node->tScope.flags.add(CG_Block::FLAG_BREAK_ABLE|CG_Block::FLAG_CONTINUE_ABLE);
	avis.back().cg_local.enter1(node->tScope);

	if(node->token.word=="for_each")
	{
		avis.define_var("#iterator",VAR_LOCAL);
		avis.getvar("#iterator");

		for(TNode_variable_list::nodelist::iterator it=node->fe_variable->aList.begin();it!=node->fe_variable->aList.end();++it)
		{
			TNode_var* p=(TNode_var*)(*it).get();
			avis.define_var(p->token.word,VAR_LOCAL);
			avis.getvar(p->token.word);
		}

		visit_it<TNode_expression>(node->fe_container,1);

		node->fe_variable->accept(lvis,0);
		node->stmt_body->accept(*this,0);
	}
	else
	{
		if(node->stmt_init)
		{
			node->stmt_init->accept(*this,0);
		}

		node->stmt_body->accept(*this,0);

		if(node->stmt_fini)
		{
			node->stmt_fini->accept(*this,0);
		}

		if(node->expr_cond)
		{
			node->expr_cond->accept(*this,1);
		}
	}

	avis.back().cg_local.leave1();

	return 0;

}

VisReturnType TNodeVisitorCG_AnalyzerRValue::visit(TNode_var* node,VisExtraParam)
{

	if(node->flags.get(TNode_var::FLAG_LOCAL|TNode_var::FLAG_GLOBAL))
	{
		cgen.kerror("invalid local/global");
	}

	if(node->token.word=="...")
	{
		DataPtrT<TNode_expression_call> kc=new TNode_expression_call(node->token);
		kc->exp_list=new TNode_expression_list;
		kc->exp_list->aList.append(node);
		kc->tbl=new TNode_var("unpack");
		avis.getvar("...");

		return replace_by(kc.get());
	}
	else if(node->token.word=="begin"||node->token.word=="end")
	{
		avis.back().cg_array.touch();
		avis.getvar(node->token.word);
		avis.getvar("#arr");
	}
	else
	{
		avis.getvar(node->token.word);
	}

	return def_value();
}


VisReturnType TNodeVisitorCG_AnalyzerRValue::visit(TNode_val_variant* node,VisExtraParam)
{
	if(!node->update())
	{
		kerror("invalid variant");
	}
	return def_value();
}

VisReturnType TNodeVisitorCG_AnalyzerRValue::visit(TNode_statement_assignment* node,VisExtraParam visp)
{
	LockState<bool> lock(show_temp,!node->flags.get(TNode::FLAG_WITH_SEMI|TNode_statement::FLAG_WITH_SEMI2));

	return basetype::visit(node,visp);
}

VisReturnType TNodeVisitorCG_AnalyzerRValue::visit(TNode_expression_op_assign* node,VisExtraParam)
{


	if (node->param[0]->pm_count() == 0)
	{

		TNode_expression_list* plist = node->param[1]->to_exp_list();
		TNode_var* pvar = NULL;

		if (plist && plist->aList.size() == 1)
		{
			pvar = plist->aList[0]->to_var();
		}

		// clear -> clear(),  clc -> clc();
		if (pvar && (pvar->token.word == "clear" || pvar->token.word == "clc"))
		{
			DataPtrT<TNode_expression_call> expcall(new TNode_expression_call(pvar->token));
			expcall->tbl.reset(pvar);
			expcall->exp_list.reset(new TNode_expression_list);

			plist->aList[0].reset(expcall.get());
			node->param[1]->flags.add(TNode::FLAG_WITH_SEMI);
		}
		else if (cgen.flags.get(CodeGen::FLAG_SAVE_TEMP))
		{
			node->param[0].reset(new TNode_var("temp"));
		}
	}

	if (
		show_temp &&
		!node->flags.get(TNode::FLAG_WITH_SEMI|TNode::FLAG_SUB_ASSIGN) &&
		cgen.flags.get(CodeGen::FLAG_SHOW_RESULTS) &&
		node->param[1]->pm_count() != 0)
	{
		DataPtrT<TNode_expression_call> ncall(new TNode_expression_call(tokItem()));
		ncall->exp_list = node->param[1]->to_exp_list();
		if (!ncall->exp_list)
		{
			ncall->exp_list.reset(new TNode_expression_list);
			ncall->exp_list->aList.append(node->param[1].get());
		}

		ncall->tbl.reset(new TNode_var("#show_temp"));
		TNode_expression_list* p = new TNode_expression_list;
		p->aList.append(ncall);
		node->param[1].reset(p);
	}

	if(node->param[0]->dot3_pos()>0)
	{
		if(node->token.word!="=")
		{
			avis.kerror("invalid assignment");
		}

		DataPtrT<TNode_expression_call> kc=new TNode_expression_call(tokItem());
		kc->tbl.reset(new TNode_var("#dot3_adjust"));
		kc->exp_list=node->param[1]->to_exp_list();
		if(!kc->exp_list)
		{
			avis.kerror("invalid exp_list");
		}
		node->param[1]=kc;
	}
	else if(node->token.word.size()>1)
	{
		if(node->param[0]->pm_count()!=1)
		{
			avis.kerror("invalid assignmet");
		}

		DataPtrT<TNode_expression_op2> n(new TNode_expression_op2);
		n->param[0]=node->param[0];
		n->param[1]=node->param[1];

		n->token.word=node->token.word.substr(0,node->token.word.size()-1);
		node->token.word="=";
		node->param[1]=n;
	}



	if(node->param[1])
	{
		visit_it<TNode_expression>(node->param[1],0);
	}

	if(node->param[0])
	{
		node->param[0]->accept(lvis,0);
	}


	return def_value();
}


VisReturnType TNodeVisitorCG_AnalyzerRValue::visit(TNode_val_function* node,VisExtraParam)
{

	avis.stackframe_enter(node);
	node->func_body->accept(*this,0);
	avis.stackframe_leave();

	return def_value();
}

VisReturnType TNodeVisitorCG_AnalyzerRValue::visit(TNode_statement_switch* node,VisExtraParam visp)
{
	node->tScope.flags.add(CG_Block::FLAG_BREAK_ABLE);
	avis.back().cg_local.enter1(node->tScope);
	basetype::visit(node,visp);
	avis.back().cg_local.leave1();

	return def_value();
}

VisReturnType TNodeVisitorCG_AnalyzerRValue::visit(TNode_statement_block* node,VisExtraParam visp)
{
	avis.back().cg_local.enter1(node->tScope);
	visit_base(node,visp);
	avis.back().cg_local.leave1();
	return def_value();
}


VisReturnType TNodeVisitorCG_AnalyzerRValue::visit(TNode_val_class* node,VisExtraParam)
{

	TNode_var* pvar=dynamic_cast<TNode_var*>(node->func_name.get());
	if(pvar)
	{
		avis.define_var(pvar->token.word,VAR_DEFAULT);
	}
	else if(node->func_name)
	{
		node->func_name->accept(lvis,0);
	}

	if(node->base_list)
	{
		node->base_list->accept(rvis,0);
	}

	avis.back().cg_local.enter1(node->tScope);
	avis.define_var("self",VAR_LOCAL);
	avis.define_var("meta",VAR_LOCAL);

	avis.getvar("self");
	avis.getvar("meta");

	node->func_body->accept(*this,0);

	avis.back().cg_local.leave1();

	return def_value();
}


TNodeVisitorCG_Analyzer::TNodeVisitorCG_Analyzer(CodeGen& g)
	:TNodeVisitorCG(g)
	,lvis(*this)
	,rvis(*this)

{

}

void TNodeVisitorCG_Analyzer::kerror(const String& s)
{
	basetype::kerror(String::Format("Analyzer:%s",s));
}

EW_LEAVE

