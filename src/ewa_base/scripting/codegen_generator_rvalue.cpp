#include "ewa_base/scripting/codegen_generator.h"
#include "ewa_base/scripting/parser_nodes.h"
#include "ewa_base/scripting/executor.h"
#include "ewa_base/scripting/callable_table.h"

EW_ENTER


void TNodeVisitorCG_GeneratorRValue::handle(TNode*)
{
	kerror("unknown expected TNode");
}

VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode* node,VisExtraParam)
{
	handle(node);
	return def_value();
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_variable_list* node,VisExtraParam)
{
	kerror("TNode_variable_list cannot be rvalue!!!");
	return def_value();
}

VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_expression*,VisExtraParam)
{
	kerror("unknown TNode_expression");
	return def_value();
}

VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_val_nil* node,VisExtraParam visp)
{
	EW_ASSERT(node!=NULL);
	EW_ASSERT(visp>=-1);

	if(visp==0) return 0;
	if(visp<2)
	{
		cgen.emit_push_nil(1);
		return 1;
	}
	else
	{
		cgen.emit_push_nil(visp);
		return visp;		
	}
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_braket* node,VisExtraParam visp)
{
	EW_ASSERT(node!=NULL);
	EW_ASSERT(visp>=-1 && visp<=1);

	int kep=visp==0?0:1;
	if(node->flags.get(TNode_braket::FLAG_WITH_BRAKET2))
	{
		cgen.kerror("invalid braket2");
	}
	VisReturnType n=node->exp_list->accept(*this,kep);

	EW_ASSERT(n==kep);
	return kep;

}

VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_expression_list* node,VisExtraParam visp)
{

	VisExtraParam sz=(VisExtraParam)node->aList.size();

	if(visp<0)
	{
		int n=1;
		for(int i=0;i<sz;i++)
		{
			if(i==sz-1)
			{
				n=node->aList[i]->accept(*this,-1);
				EW_ASSERT(n==1||n==-1);
			}
			else
			{
				node->aList[i]->accept(*this,1);
			}
		}
		return n>=0?sz:-sz;
	}
	else
	{
		int d=visp-sz;
		for(int i=0;i<sz;i++)
		{
			if(i==sz-1)
			{
				int k=d+1;
				d-=node->aList[i]->accept(*this,k>=0?k:0)-1;
			}
			else
			{
				node->aList[i]->accept(*this,1);
			}
		}

		if(d>0)
		{
			cgen.emit_push_nil(d);
		}
		else if(d<0)
		{
			cgen.emit(XOP_SADJ,d);
		}

		return visp;
	}

}

VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_item* node,VisExtraParam)
{
	cgen.kerror("unknown TNode_item");
	return def_value();
}

VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_var* node,VisExtraParam visp)
{
	if(visp==0) return 0;
	EW_ASSERT(visp==1||visp==-1);

	if(node->token.word=="")
	{
		cgen.emit_push_nil(1);
		return 1;
	}
	else
	{
		cgen.emit_getvar(node->token.word);
		return 1;
	}
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_val* node,VisExtraParam)
{
	cgen.kerror("unknown TNode_val");
	return def_value();
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_val_variant* node,VisExtraParam visp)
{
	if(visp==0) return 0;
	EW_ASSERT(visp==1||visp==-1);

	cgen.emit_push(node->value);
	return 1;
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_val_integer* node,VisExtraParam visp)
{
	if(visp==0) return 0;
	EW_ASSERT(visp==1||visp==-1);

	cgen.emit_push(node->value.get<int64_t>());
	return 1;
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_val_boolean* node,VisExtraParam visp)
{
	if(visp==0) return 0;
	EW_ASSERT(visp==1||visp==-1);

	cgen.emit_push(node->value.get<bool>());
	return 1;
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_expression_op* node,VisExtraParam)
{
	cgen.kerror("unknown TNode_expression_op:"+node->token.word);
	return def_value();
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_expression_op1* node,VisExtraParam visp)
{
	EW_ASSERT(visp>=-1&&visp<=1);

	if(node->token.word=="+")
	{
		node->param[0]->accept(*this,1);
	}
	else if(node->token.word=="-")
	{
		node->param[0]->accept(*this,1);
		cgen.emit(XOP1_NEG);
	}
	else if(node->token.word=="~")
	{
		node->param[0]->accept(*this,1);
		cgen.emit(XOP1_BITWISE_NOT);
	}
	else if(node->token.word=="++"||node->token.word=="--")
	{

		int xop2=node->token.word=="++"?XOP2_ADD:XOP2_SUB;

		node->param[0]->accept(*this,1);
		if(node->flags.get(TNode_expression::FLAG_POST))
		{
			cgen.emit(XOP_GET_STACK,0);
			cgen.emit_push(1);
			cgen.emit(xop2);

			node->param[0]->accept(lvis,1);
		}
		else
		{
			cgen.emit_push(1);
			cgen.emit(xop2);
			cgen.emit(XOP_GET_STACK,0);
			node->param[0]->accept(lvis,1);
		}

	}
	else if(node->token.word=="!")
	{
		node->param[0]->accept(*this,1);
		cgen.emit(XOP1_NOT);
	}
	else
	{
		cgen.kerror("unknown TNode_expression_op1:"+node->token.word);
	}
	return 1;
}


class xop2_info : public ObjectData
{
public:

	xop2_info(int c):op_code(c){}
	virtual int code(int t1,int t2){return op_code;}
	virtual int rett(int t1,int t2){return 0;}

protected:
	int op_code;
};

indexer_map<String,DataPtrT<xop2_info> >& _g_get_mp_op2()
{
	static indexer_map<String,DataPtrT<xop2_info> > mp_op2;
	if (mp_op2.size() == 0)
	{
		mp_op2["+"].reset(new xop2_info(XOP2_ADD));
		mp_op2["-"].reset(new xop2_info(XOP2_SUB));
		mp_op2["%"].reset(new xop2_info(XOP2_MOD));
		mp_op2["*"].reset(new xop2_info(XOP2_MUL));
		mp_op2["/"].reset(new xop2_info(XOP2_DIV));
		mp_op2["^"].reset(new xop2_info(XOP2_POW));

		mp_op2[".*"].reset(new xop2_info(XOP2_DOT_MUL));
		mp_op2["./"].reset(new xop2_info(XOP2_DOT_DIV));
		mp_op2[".^"].reset(new xop2_info(XOP2_DOT_POW));

		mp_op2[">"].reset(new xop2_info(XOP2_GT));
		mp_op2["<"].reset(new xop2_info(XOP2_LT));
		mp_op2[">="].reset(new xop2_info(XOP2_GE));
		mp_op2["<="].reset(new xop2_info(XOP2_LE));
		mp_op2["=="].reset(new xop2_info(XOP2_EQ));
		mp_op2["!="].reset(new xop2_info(XOP2_NE));

		mp_op2["==="].reset(new xop2_info(XOP2_SAME));

		// && and || // use lazy evaluate
		//mp_op2["&&"].reset(new xop2_info(XOP2_AND));
		//mp_op2["||"].reset(new xop2_info(XOP2_OR));

		mp_op2["^^"].reset(new xop2_info(XOP2_XOR));


		mp_op2[">>"].reset(new xop2_info(XOP2_SHIFT_R));
		mp_op2["<<"].reset(new xop2_info(XOP2_SHIFT_L));

		mp_op2["&"].reset(new xop2_info(XOP2_BITWISE_AND));
		mp_op2["|"].reset(new xop2_info(XOP2_BITWISE_OR));
		//mp_op2["^"].reset(new xop2_info(XOP2_BITWISE_XOR));

		mp_op2[".."].reset(new xop2_info(XOP2_CAT));
	}
	return mp_op2;
}

VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_expression_opn* node, VisExtraParam)
{
	if (node->token.word == "?")
	{
		node->param[0]->accept(*this, 1);
		cgen.emit(XOP_JUMP_0);
		int jp = cgen.inst_pos();
		node->param[1]->accept(*this, 1);
		cgen.emit(XOP_JUMP);
		int je = cgen.inst_pos();
		cgen.set_jp(jp);
		node->param[2]->accept(*this, 1);
		cgen.set_jp(je);
	}
	else
	{
		kerror("syntax error");
	}

	return 1;

}
VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_expression_op2* node,VisExtraParam)
{

	indexer_map<String,DataPtrT<xop2_info> >& mp_op2(_g_get_mp_op2());

	const String& sop(node->token.word);

	xop2_info* op = mp_op2[sop].get();
	if(op)
	{
		node->param[0]->accept(*this,1);
		node->param[1]->accept(*this,1);
		cgen.emit(op->code(0,0));
	}
	else if (sop == "\\")
	{
		node->param[1]->accept(*this,1);
		node->param[0]->accept(*this,1);
		cgen.emit(XOP2_DIV);
	}
	else if (sop == ".\\")
	{
		node->param[1]->accept(*this,1);
		node->param[0]->accept(*this,1);
		cgen.emit(XOP2_DOT_DIV);
	}
	else if (sop == "&&")
	{
		node->param[0]->accept(*this,1);
		cgen.emit(XOP_GET_STACK,0);
		cgen.emit(XOP_JUMP_0);
		int jp=cgen.inst_pos();
		cgen.emit(XOP_SADJ,-1);
		node->param[1]->accept(*this,1);
		cgen.set_jp(jp);
	}
	else if (sop == "||")
	{
		node->param[0]->accept(*this,1);
		cgen.emit(XOP_GET_STACK,0);
		cgen.emit(XOP_JUMP_1);
		int jp=cgen.inst_pos();
		cgen.emit(XOP_SADJ,-1);
		node->param[1]->accept(*this,1);
		cgen.set_jp(jp);
	}
	else
	{
		cgen.kerror("unknown TNode_expression_op2:"+node->token.word);
	}
	return 1;
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_expression_op_assign* node,VisExtraParam visp)
{

	TNode_expression* lval=node->param[0].get();
	TNode_expression* rval=node->param[1].get();

	if(rval->pm_count()==0)
	{
		TNode_variable_list* p=lval->to_var_list();
		if(!p||visp!=0)
		{
			cgen.kerror("unknown error");
		}
		else if(node->flags.get(TNode_var::FLAG_LOCAL))
		{
			// local a,b,c; // initialize local variables to nil
			cgen.emit_push_nil(p->aList.size());
			for(TNode_variable_list::nodelist::iterator it=p->aList.begin();it!=p->aList.end();++it)
			{
				const String& s((*it)->token.word);
				if(s!="") cgen.emit_setvar(s);
			}
			return 0;
		}
		else if(node->flags.get(TNode_var::FLAG_GLOBAL))
		{
			return 0;
		}

	}

	int n=lval->pm_count();
	if(n<0)
	{
		kerror("invalid lvalue");
	}

	int r=rval->accept(*this,n);
	if(r!=n)
	{
		kerror("invalid rval");
	}

	if(visp<0)
	{
		visp=lval->pm_count();
	}

	if(visp==0)
	{
		lval->accept(lvis,n);
		return 0;
	}
	else if(r==1&&visp==1)
	{
		cgen.emit(XOP_GET_STACK,0);
		lval->accept(lvis,1);
		return visp;
	}
	else if(visp>0)
	{
		for(int i=0;i<n;i++)
		{
			cgen.emit(XOP_GET_STACK,1-n);
		}
		lval->accept(lvis,n);
		if(visp>n)
		{
			cgen.emit_push_nil(visp-n);
		}
		else if(visp<n)
		{
			cgen.emit(XOP_SADJ,visp-n);
		}
		return visp;
	}
	else
	{
		kerror("error");
		return 0;
	}

}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_expression_call* node,VisExtraParam visp)
{

	VisReturnType n=node->exp_list->accept(*this,-1);

	node->tbl->accept(*this,1);

	if(node->cls)
	{
		node->cls->accept(*this,1);
		cgen.emit(XOP_SET_SBASE,StackState1::SBASE_THIS);
		cgen.emit_this_call(n,visp);
	}
	else if (dynamic_cast<TNode_expression_dot*>(node->tbl.get()) != NULL)
	{
		cgen.emit_this_call(n,visp);
	}
	else
	{
		cgen.emit_call(n,visp);
	}

	return visp;

}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_expression_dot* node,VisExtraParam)
{
	TNode_var* pvar = dynamic_cast<TNode_var*>(node->tbl.get());

	if (pvar && cgen.is_library(pvar->token.word))
	{
		String fullname = pvar->token.word + "." + node->token.word;
		cgen.emit_getsys(fullname);
	}
	else
	{
		node->tbl->accept(*this,1);
		cgen.emit_getidx(node->token.word);
	}

	return 1;

}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_expression_arr* node,VisExtraParam visp)
{
	return visit_arr(node,visp,XOP_GET_ARRAY);
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_statement_macro* node, VisExtraParam visp)
{
	if (node->token.word == "import")
	{
		size_t n = node->extra.size();
		if (n != 1)
		{
			kerror("invalid syntax of #import");
		}

		cgen.back().cg_imports.insert(node->extra[0].word);

		cgen.emit_push(node->extra[0].word);
		cgen.emit_getsys("import");
		cgen.emit_call(1, 0);

		return 0;
	}
	else
	{
		return basetype::visit(node, visp);
	}
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_statement*,VisExtraParam)
{
	cgen.kerror("unknown TNode_statement");
	return def_value();
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_statement_nil*,VisExtraParam)
{
	return def_value();
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_statement_assignment* node,VisExtraParam visp)
{
	EW_UNUSED(visp);
	EW_ASSERT(visp==0);

	node->value->accept(*this,0);
	return def_value();
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_statement_if* node,VisExtraParam pm)
{
	EW_UNUSED(pm);

	node->expr_cond->accept(*this,1);

	cgen.emit(XOP_JUMP_0);
	int j0=cgen.inst_pos();
	node->stmt_if_1->accept(*this,node->kep);

	cgen.emit(XOP_JUMP);
	int je=cgen.inst_pos();

	cgen.set_jp(j0);

	if(node->stmt_if_0)
	{
		node->stmt_if_0->accept(*this,node->kep);
	}
	else if (node->kep != 0)
	{

	}
	cgen.set_jp(je);

	return def_value();
}

VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_statement_control* node,VisExtraParam)
{
	if(node->token.word=="break")
	{
		cgen.back().cg_local.add_b(1);
	}
	else if(node->token.word=="continue")
	{
		cgen.back().cg_local.add_c(1);
	}
	else if(node->token.word=="break2")
	{
		cgen.back().cg_local.add_b(2);
	}
	else if(node->token.word=="continue2")
	{
		cgen.back().cg_local.add_c(2);
	}
	else
	{
		cgen.kerror("unknown TNode_statement_control:"+node->token.word);
	}

	return def_value();
}

VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_statement_try* node,VisExtraParam)
{
	cgen.emit(XOP_TRY_ENTER);
	int j1=cgen.inst_pos();
	cgen.back().cg_local.enter2(node->tScope_try);
	node->pBlock_try->accept(*this,0);
	cgen.back().cg_local.leave2();
	cgen.emit(XOP_TRY_LEAVE);

	cgen.emit(XOP_JUMP);
	int j2=cgen.inst_pos();

	cgen.set_jp(j1);

	if(node->exp_list->dot3_pos()>0)
	{
		cgen.emit_getsys("#dot3_adjust");
		cgen.emit_call(-1,node->exp_list->pm_count());
	}
	else
	{
		cgen.emit_getsys("#kepn_adjust");
		cgen.emit_call(-1,node->exp_list->pm_count());
	}


	cgen.back().cg_local.enter2(node->tScope_catch);

	TNode_variable_list::nodelist::reverse_iterator it=node->exp_list->aList.rbegin();
	for(;it!=node->exp_list->aList.rend();++it)
	{
		TNode_var* p=(TNode_var*)(*it).get();
		cgen.emit_setvar(p->token.word);
	}
	
	node->pBlock_catch->accept(*this,0);
	cgen.back().cg_local.leave2();

	cgen.set_jp(j2);
	if(node->pBlock_finally)
	{
		cgen.back().cg_local.enter2(node->tScope_finally);
		node->pBlock_finally->accept(*this,0);
		cgen.back().cg_local.leave2();
	}

	return def_value();
}

VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_statement_loop* node,VisExtraParam)
{

	cgen.back().cg_local.enter2(node->tScope);


	if(node->token.word=="for_each")
	{

		int n=(int)node->fe_variable->aList.size();

		node->fe_container->accept(*this,1);
		if(n==1)
		{
			cgen.emit_getsys("#get_iterator1");
			cgen.emit_call(1,1);
		}
		else if(n==2)
		{
			cgen.emit_getsys("#get_iterator2");
			cgen.emit_call(1,1);
		}
		else
		{
			cgen.kerror("error");
		}

		cgen.emit_setvar("#iterator");

		int jc=cgen.inst_end();

		cgen.emit_getvar("#iterator");
		cgen.emit_call(0,n+1);
		node->fe_variable->accept(lvis,n);
		cgen.emit(XOP_JUMP_0);
		int je=cgen.inst_pos();

		node->stmt_body->accept(*this,0);
		cgen.back().cg_local.add_c(1);

		cgen.back().cg_local.set_b(-1);
		cgen.back().cg_local.set_c(jc);

		cgen.set_jp(je);

	}
	else
	{
		if(node->stmt_init)
		{
			node->stmt_init->accept(*this,0);
		}

		if(node->token.word!="do")
		{
			cgen.emit(XOP_JUMP);
		}

		int j1=cgen.inst_end();
		node->stmt_body->accept(*this,0);
		int jc=cgen.inst_end();

		if(node->stmt_fini)
		{
			node->stmt_fini->accept(*this,0);
		}

		if(node->token.word!="do")
		{
			cgen.set_jp(j1-1);
		}

		if(node->expr_cond)
		{
			node->expr_cond->accept(*this,1);
			int j2=cgen.inst_end();
			cgen.emit(XOP_JUMP_1,j1-j2-1);
		}
		else
		{
			int j2=cgen.inst_end();
			cgen.emit(XOP_JUMP,j1-j2-1);
		}

		cgen.back().cg_local.set_b(-1);
		cgen.back().cg_local.set_c(jc);

	}

	cgen.back().cg_local.leave2();

	return def_value();
}

VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_statement_list* node,VisExtraParam)
{
	for(TNode_statement_list::nodelist::iterator it=node->aList.begin();it!=node->aList.end();++it)
	{
		(*it)->accept(*this,0);
	}
	return def_value();
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_statement_block* node,VisExtraParam visp)
{
	cgen.back().cg_local.enter2(node->tScope);
	visit_base(node,visp);
	cgen.back().cg_local.leave2();
	return def_value();
}

VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_statement_return* node,VisExtraParam)
{
	int n=node->value->accept(*this,-2);
	cgen.back().cg_local.add_r();
	cgen.emit(XOP_HALT,n);
	return def_value();
}

VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_statement_throw* node,VisExtraParam)
{
	int n=node->value->accept(*this,-2);
	cgen.emit(XOP_TRY_THROW,n);
	return def_value();
}

VisReturnType TNodeVisitorCG::visit(TNode_statement_macro* node,VisExtraParam)
{
	if(node->token.word=="variable")
	{
		size_t n=node->param.size();
		if(n==0||n>2)
		{
			kerror("syntax error");
		}

		if(node->param[0].word=="implicit")
		{
			bool implicit_global=true;
			if(n==1){}
			else if(node->param[1].word=="local")
			{
				implicit_global=false;
			}
			else if(node->param[1].word!="global")
			{
				kerror("syntax error");
			}

			cgen.flags.set(CodeGen::FLAG_IMPLICIT_GLOBAL,implicit_global);
			cgen.flags.set(CodeGen::FLAG_IMPLICIT_LOCAL,!implicit_global);
		}
		else if(node->param[0].word=="explicit")
		{
			if(n!=1)
			{
				kerror("syntax error");
			}

			cgen.flags.del(CodeGen::FLAG_IMPLICIT_GLOBAL);
			cgen.flags.del(CodeGen::FLAG_IMPLICIT_LOCAL);
		}
		else
		{
			kerror("syntax error");
		}

	}
	else if(node->token.word=="results")
	{
		size_t n=node->param.size();
		if(n!=1)
		{
			kerror("syntax error");
		}

		if(node->param[0].word=="show")
		{
			cgen.flags.add(CodeGen::FLAG_SHOW_RESULTS);
		}
		else if(node->param[0].word=="hide")
		{
			cgen.flags.del(CodeGen::FLAG_SHOW_RESULTS);
		}
		else
		{
			kerror("syntax error");
		}
	}
	else if (node->token.word == "import")
	{

	}
	else if (node->token.word == "export")
	{

	}
	else
	{
		kerror("syntax error");
	}
	return 0;
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_val_class* node,VisExtraParam visp)
{

	if(visp<-1||visp>1)
	{
		cgen.kerror("invalid visp");
	}

	TNode_var* pvar=dynamic_cast<TNode_var*>(node->func_name.get());

	cgen.back().cg_local.enter2(node->tScope);

	VisReturnType n=0;
	
	if(node->base_list)
	{
		n=node->base_list->accept(*this,-1);
	}

	cgen.emit_getsys("#new_metatable");
	cgen.emit_call(n,3);
	cgen.emit_setvar("meta");
	cgen.emit_setvar("self");

	if(node->func_name)
	{
		if(visp==1)
		{
			cgen.emit(XOP_GET_STACK,0);
		}
		node->func_name->accept(lvis,1);

	}
	else if(visp==0)
	{
		cgen.emit(XOP_SADJ,-1);
	}

	node->func_body.get()->accept(*this,0);

	cgen.back().cg_local.leave2();

	return visp==0?0:1;
}


class CallableDataSwitch : public CallableObject
{
public:
	indexer_map<Variant,int64_t> jpm;

	int64_t jpd;
	int64_t jps;

	CallableDataSwitch()
	{
		jpd=-1;
	}

	bool AddCase(Variant& v,int64_t p)
	{
		if(jpm.find1(v)<0)
		{
			jpm[v]=p-jps-1;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool AddDefault(int64_t p)
	{
		if(jpd!=-1) return false;
		jpd=p-jps-1;
		return true;
	}

	int __fun_call(Executor& ewsl,int)
	{
		Variant &top(*ewsl.ci1.nsp);
		int id=jpm.find1(top);
		if(id<0)
		{
			top.reset(jpd);
		}
		else
		{
			top.reset(jpm.get(id).second);
		}
		return 1;
	}

};


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_statement_switch* node,VisExtraParam)
{
	if (node->token.word == "switch")
	{
		DataPtrT<CallableDataSwitch> pCaswitch(new CallableDataSwitch);

		cgen.back().cg_local.enter2(node->tScope);

		node->expr_cond->accept(*this,1);
		cgen.emit_push(pCaswitch.get());
		cgen.emit(XOP_CALL);
		cgen.emit(XOP_JUMP_N);

		pCaswitch->jps=cgen.inst_pos();

		for(TNode_statement_switch::case_iterator it1=node->case_list.begin();it1!=node->case_list.end();++it1)
		{
			Variant v;

			TNode_expression_list *elist = dynamic_cast<TNode_expression_list *>((*it1)->case_value.get());

			if(elist)
			{
				for (size_t i = 0; i < elist->aList.size();i++)
				//for(TNode_statement_switch::item_iterator it2=(*it1)->case_value.begin();it2!=(*it1)->case_value.end();++it2)
				{
					TNode_expression* eitem=elist->aList[i].get();
					if(!eitem->get_value(v))
					{
						kerror("case must be constant");
					}
					if(!pCaswitch->AddCase(v,cgen.inst_end()))
					{
						kerror("case already exists");
					}
				}
			}
			else
			{
				if(!pCaswitch->AddDefault(cgen.inst_end()))
				{
					kerror("multiple default cases");
				}
			}

			(*it1)->case_stmt->accept(*this,0);

			cgen.back().cg_local.add_b(1);

		}

		int jp_exit=cgen.inst_end();

		cgen.back().cg_local.set_b(jp_exit);

		pCaswitch->AddDefault(jp_exit);


		cgen.back().cg_local.leave2();
	}
	else if (node->token.word == "judge")
	{
		cgen.back().cg_local.enter2(node->tScope);

		arr_1t<int> jpx;
		for (TNode_statement_switch::case_iterator it1 = node->case_list.begin(); it1 != node->case_list.end(); ++it1)
		{
			TNode_expression *cond = (*it1)->case_value.get();
			if (cond)
			{
				cond->accept(*this, 1);
				cgen.emit(XOP_JUMP_0);
				int j0 = cgen.inst_pos();
				(*it1)->case_stmt->accept(*this, 0);
				cgen.emit(XOP_JUMP);
				jpx.push_back(cgen.inst_pos());
				cgen.set_jp(j0);
			}
			else
			{
				(*it1)->case_stmt->accept(*this, 0);
			}
		}

		for (size_t i = 0; i < jpx.size(); i++) cgen.set_jp(jpx[i]);

		cgen.back().cg_local.leave2();
	}


	return def_value();
}

VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_val_empty* node,VisExtraParam visp)
{
	(void)node;
	if(visp==0) return 0;
	cgen.emit_push_nil();
	return 1;
}


VisReturnType TNodeVisitorCG_GeneratorRValue::visit(TNode_val_function* node,VisExtraParam visp)
{
	if(visp==-1)
	{
		visp=1;
	}
	else if(visp>1)
	{
		cgen.kerror("invalid visp");
	}

	cgen.stackframe_enter(node);
	node->func_body.get()->accept(*this,0);
	cgen.stackframe_leave();

	if(node->func_name)
	{
		if(visp!=0)
		{
			cgen.emit(XOP_GET_STACK,0);
		}
		node->func_name->accept(lvis,1);
	}
	else if(visp==0)
	{
		cgen.emit(XOP_SADJ,-1);
	}

	return visp;
}


EW_LEAVE
