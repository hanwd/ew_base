#include "ewa_base/scripting/parser.h"
#include "ewa_base/scripting/parser_nodes.h"

EW_ENTER


void TNodeVisitorFormater::indent()
{
	for(int i=0;i<nIndent;i++)
	{
		aBuffer.append('\t');
	}
}

void TNodeVisitorFormater::indent_inc()
{
	++nIndent;
}

void TNodeVisitorFormater::indent_dec()
{
	--nIndent;
}


VisReturnType TNodeVisitorFormater::visit(TNode_expression_list*node,VisExtraParam p)
{
	for(TNode_expression_list::nodelist::iterator it=node->aList.begin();it!=node->aList.end();)
	{
		(*it)->accept(*this,p);++it;
		if(it!=node->aList.end()) aBuffer<<",";
	}
	return def_value();
}

VisReturnType TNodeVisitorFormater::visit(TNode_variable_list* node,VisExtraParam p)
{
	for(TNode_variable_list::nodelist::iterator it=node->aList.begin();it!=node->aList.end();)
	{
		(*it)->accept(*this,p);++it;
		if(it!=node->aList.end()) aBuffer<<",";
	}
	return def_value();
}

VisReturnType TNodeVisitorFormater::visit(TNode_var* node,VisExtraParam)
{
	aBuffer<<node->token.word;
	return def_value();
}

VisReturnType TNodeVisitorFormater::visit(TNode_val* node,VisExtraParam)
{
	aBuffer<<node->token.word;
	return def_value();
}


VisReturnType TNodeVisitorFormater::visit(TNode_expression_op1* node,VisExtraParam p)
{
	aBuffer<<node->token.word;
	node->param[0]->accept(*this,p);
	return def_value();
}

VisReturnType TNodeVisitorFormater::visit(TNode_expression_op2* node,VisExtraParam p)
{
	node->getp(0)->accept(*this,p);
	aBuffer<<(node->token.word);
	node->getp(1)->accept(*this,p);
	return def_value();
}

VisReturnType TNodeVisitorFormater::visit(TNode_expression_op_assign* node,VisExtraParam p)
{
	node->getp(0)->accept(*this,p);
	aBuffer<<(node->token.word);
	node->getp(1)->accept(*this,p);
	return def_value();
}

VisReturnType TNodeVisitorFormater::visit(TNode_statement_assignment*node,VisExtraParam p)
{
	indent();

	node->value->accept(*this,p);
	return def_value();
}

VisReturnType TNodeVisitorFormater::visit(TNode_statement_if*node,VisExtraParam p)
{
	indent();

	aBuffer<<"if";
	aBuffer<<"(";
	node->expr_cond->accept(*this,p);
	aBuffer<<")";
	aBuffer<<"\n";

	node->stmt_if_1->accept(*this,p);
	if(node->stmt_if_0)
	{
		aBuffer<<"else";
		aBuffer<<"\n";
		node->stmt_if_0->accept(*this,p);
	}
	return def_value();

}
VisReturnType TNodeVisitorFormater::visit(TNode_statement_loop* node,VisExtraParam p)
{
	indent();

	if(node->token.word=="while")
	{
		aBuffer<<"while";
		aBuffer<<"(";
		node->expr_cond->accept(*this,p);
		aBuffer<<")";
		aBuffer<<"\n";

		node->stmt_body->accept(*this,p);
	
	}
	else if(node->token.word=="do")
	{
		aBuffer<<"do";
		aBuffer<<"\n";
		node->stmt_body->accept(*this,p);
		aBuffer<<"while";
		aBuffer<<"(";
		node->expr_cond->accept(*this,p);
		aBuffer<<")";
		aBuffer<<"\n";
	}
	else
	{
		aBuffer<<"for";
		aBuffer<<"(";
		node->stmt_init->accept(*this,p);
		aBuffer<<";";
		node->expr_cond->accept(*this,p);
		aBuffer<<";";
		node->stmt_fini->accept(*this,p);
		aBuffer<<")";
		aBuffer<<"\n";

		node->stmt_body->accept(*this,p);
	}
	return def_value();

}
VisReturnType TNodeVisitorFormater::visit(TNode_val_class*node,VisExtraParam p)
{
	
	return def_value();
}

VisReturnType TNodeVisitorFormater::visit(TNode_statement_nil*,VisExtraParam p)
{
	aBuffer<<(";");
	return def_value();
}


VisReturnType TNodeVisitorFormater::visit(TNode_statement_list*node,VisExtraParam p)
{
	for(TNode_statement_list::nodelist::iterator it=node->aList.begin();it!=node->aList.end();++it)
	{
		(*it)->accept(*this,p);
		aBuffer<<"\n";
	}
	return def_value();
}

VisReturnType TNodeVisitorFormater::visit(TNode_statement_block*node,VisExtraParam p)
{
	indent();
	aBuffer<<"{";
	aBuffer<<"\n";
	indent_inc();
	TNodeVisitorFormater::visit((TNode_statement_list*)node,p);
	indent_dec();
	aBuffer<<"}";
	aBuffer<<"\n";
	return def_value();
}

VisReturnType TNodeVisitorFormater::visit(TNode_val_function*node,VisExtraParam p)
{
	indent();

	aBuffer<<"function ";
	aBuffer<<(node->token.word);
	aBuffer<<"(";
	node->exp_list->accept(*this,p);
	aBuffer<<")";
	aBuffer<<"\n";

	visit_base(node,p);
	return def_value();

}

VisReturnType TNodeVisitorFormater::visit(TNode_expression_call* node,VisExtraParam p)
{
	node->tbl->accept(*this,p);
	aBuffer<<"(";
	node->exp_list->accept(*this,p);
	aBuffer<<")";
	return def_value();
}

VisReturnType TNodeVisitorFormater::visit(TNode_expression_dot* node,VisExtraParam p)
{
	node->tbl->accept(*this,p);
	aBuffer<<".";
	aBuffer<<(node->token.word);
	return def_value();
}

VisReturnType TNodeVisitorFormater::visit(TNode_expression_arr* node,VisExtraParam p)
{
	node->tbl->accept(*this,p);
	aBuffer<<"[";
	node->exp_list->accept(*this,p);
	aBuffer<<"]";
	return def_value();
}

EW_LEAVE
