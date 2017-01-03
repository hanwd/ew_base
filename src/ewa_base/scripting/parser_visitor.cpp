#include "ewa_base/scripting/parser.h"
#include "ewa_base/scripting/parser_nodes.h"

EW_ENTER

void TNodeVisitor::handle(TNode* q)
{
	
}

VisReturnType TNodeVisitorVisitBase::visit(TNode* node,VisExtraParam)
{
	handle(node);
	return def_value();
}

VisReturnType TNodeVisitorVisitBase::visit(TNode_variable_list* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_expression* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_val_nil* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_expression_list* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_item* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_var* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_val* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_braket* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_val_variant* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_val_integer* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_val_boolean* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_expression_op* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_expression_op1* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_expression_op2* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_expression_op_assign* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_expression_call* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_expression_dot* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_expression_arr* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_statement* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_statement_nil* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_statement_assignment* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_statement_if* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_statement_loop* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_val_class* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_statement_list* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_statement_block* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_val_function* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_statement_control* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_statement_return* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_val_empty* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_statement_switch* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_statement_try* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_statement_throw* node,VisExtraParam visp){return visit_base(node,visp);}
VisReturnType TNodeVisitorVisitBase::visit(TNode_item_select* node, VisExtraParam visp){ return visit_base(node, visp); }


VisReturnType TNodeVisitorRecursive::visit(TNode_variable_list* node,VisExtraParam p)
{
	for(TNode_variable_list::nodelist::iterator it=node->aList.begin();it!=node->aList.end();++it)
	{
		(*it)->accept(*this,p);
	}
	return def_value();
}

VisReturnType TNodeVisitorRecursive::visit(TNode_expression_list* node,VisExtraParam p)
{
	for(TNode_expression_list::nodelist::iterator it=node->aList.begin();it!=node->aList.end();++it)
	{
		(*it)->accept(*this,p);
	}
	return def_value();
}

VisReturnType TNodeVisitorRecursive::visit(TNode_braket* node,VisExtraParam p)
{
	return node->exp_list->accept(*this,p);
}

VisReturnType TNodeVisitorRecursive::visit(TNode_expression_op* node,VisExtraParam p)
{
	int s=node->size();
	for(int i=0;i<s;i++)
	{
		node->getp(s)->accept(*this,p);
	}
	return def_value();
}

VisReturnType TNodeVisitorRecursive::visit(TNode_expression_op1* node,VisExtraParam p)
{
	node->param[0]->accept(*this,p);
	return def_value();
}

VisReturnType TNodeVisitorRecursive::visit(TNode_expression_op2* node,VisExtraParam p)
{
	if(node->param[1]) node->param[1]->accept(*this,p);
	if(node->param[0]) node->param[0]->accept(*this,p);
	return def_value();
}

VisReturnType TNodeVisitorRecursive::visit(TNode_item_select* node,VisExtraParam p)
{
	for (size_t i = 0; i < node->exp_list->aList.size(); i++)
	{
		if (node->exp_list->aList[i]) node->exp_list->aList[i]->accept(*this, p);
	}
	return def_value();
}

VisReturnType TNodeVisitorRecursive::visit(TNode_expression_call* node,VisExtraParam p)
{

	if(node->cls)
	{
		node->cls->accept(*this,p);
	}

	node->tbl->accept(*this,p);
	node->exp_list->accept(*this,p);

	return def_value();
}

VisReturnType TNodeVisitorRecursive::visit(TNode_expression_dot* node,VisExtraParam p)
{
	node->tbl->accept(*this,p);
	return def_value();
}

VisReturnType TNodeVisitorRecursive::visit(TNode_expression_arr* node,VisExtraParam p)
{
	node->tbl->accept(*this,p);
	node->exp_list->accept(*this,p);
	return def_value();
}


VisReturnType TNodeVisitorRecursive::visit(TNode_statement_assignment* node,VisExtraParam p)
{
	node->value->accept(*this,p);
	return def_value();
}

VisReturnType TNodeVisitorRecursive::visit(TNode_statement_if* node,VisExtraParam p)
{
	node->expr_cond->accept(*this,p);
	if(node->stmt_if_1)
	{
		node->stmt_if_1->accept(*this,p);
	}
	if(node->stmt_if_0)
	{
		node->stmt_if_0->accept(*this,p);
	}
	return def_value();
}

VisReturnType TNodeVisitorRecursive::visit(TNode_statement_try* node,VisExtraParam)
{
	node->pBlock_try->accept(*this,0);
	node->exp_list->accept(*this,0);
	node->pBlock_catch->accept(*this,0);
	if(node->pBlock_finally) node->pBlock_finally->accept(*this,0);
	return 0;
}

VisReturnType TNodeVisitorRecursive::visit(TNode_statement_loop* node,VisExtraParam p)
{
	if(node->stmt_init)
	{
		node->stmt_init->accept(*this,p);
	}
	if(node->expr_cond)
	{
		node->expr_cond->accept(*this,p);
	}
	if(node->stmt_fini)
	{
		node->stmt_fini->accept(*this,p);
	}
	if(node->stmt_body)
	{
		node->stmt_body->accept(*this,p);
	}
	if(node->fe_container)
	{
		node->fe_container->accept(*this,p);
	}
	return def_value();
}


VisReturnType TNodeVisitorRecursive::visit(TNode_statement_list* node,VisExtraParam p)
{
	for(TNode_statement_list::nodelist::iterator it=node->aList.begin();it!=node->aList.end();++it)
	{
		(*it)->accept(*this,p);
	}
	return def_value();
}

VisReturnType TNodeVisitorRecursive::visit(TNode_val_function* node,VisExtraParam p)
{
	if(node->exp_list)
	{
		node->exp_list->accept(*this,p);
	}
	if(node->func_name)
	{
		node->func_name->accept(*this,1);
	}

	node->func_body->accept(*this,p);
	return def_value();
}


VisReturnType TNodeVisitorRecursive::visit(TNode_val_class* node,VisExtraParam p)
{
	return node->func_body->accept(*this,p);
}


VisReturnType TNodeVisitorRecursive::visit(TNode_statement_return* node,VisExtraParam p)
{
	return node->value->accept(*this,p);
}

VisReturnType TNodeVisitorRecursive::visit(TNode_statement_throw* node,VisExtraParam p)
{
	return node->value->accept(*this,p);
}

VisReturnType TNodeVisitorRecursive::visit(TNode_statement_switch* node,VisExtraParam p)
{
	if (node->token.word == "switch")
	{
		node->expr_cond->accept(*this,p);
		for(TNode_statement_switch::case_iterator it1=node->case_list.begin();it1!=node->case_list.end();++it1)
		{
			//for(TNode_statement_switch::item_iterator it2=(*it1)->case_value.begin();
			//	it2!=(*it1)->case_value.end();++it2)
			//{
			//	(*it2)->accept(*this,p);
			//}

			if ((*it1)->case_value) (*it1)->case_value->accept(*this, p);
			(*it1)->case_stmt->accept(*this,p);
		}
	}
	else if (node->token.word == "judge")
	{
		for(TNode_statement_switch::case_iterator it1=node->case_list.begin();it1!=node->case_list.end();++it1)
		{			
			if((*it1)->case_value) (*it1)->case_value->accept(*this, p);
			(*it1)->case_stmt->accept(*this,p);
		}
	}


	return def_value();
}

EW_LEAVE
