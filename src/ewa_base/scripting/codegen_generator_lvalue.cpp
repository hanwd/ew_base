#include "ewa_base/scripting/codegen_generator.h"
#include "ewa_base/scripting/parser_nodes.h"
#include "ewa_base/scripting/executor.h"
#include "ewa_base/scripting/callable_table.h"

EW_ENTER


void TNodeVisitorCG_GeneratorLValue::handle(TNode*)
{
	cgen.kerror("invalid lvalue");
}


VisReturnType TNodeVisitorCG_GeneratorLValue::visit(TNode_expression_op2* node,VisExtraParam p)
{
	handle(node);
	return 0;
}

VisReturnType TNodeVisitorCG_GeneratorLValue::visit(TNode_expression_op1* node,VisExtraParam p)
{
	handle(node);
	return 0;
}

VisReturnType TNodeVisitorCG_GeneratorLValue::visit(TNode_var* node,VisExtraParam)
{
	if(node->token.word=="")
	{
		cgen.emit(XOP_SADJ,-1);
		return def_value();
	}

	cgen.emit_setvar(node->token.word);
	return def_value();

}

VisReturnType TNodeVisitorCG_GeneratorLValue::visit(TNode_val_nil* node,VisExtraParam)
{
	cgen.emit(XOP_SADJ,-1);	
	return def_value();

}

VisReturnType TNodeVisitorCG_GeneratorLValue::visit(TNode_variable_list* node,VisExtraParam visp)
{
	if(visp==(VisExtraParam)node->aList.size())
	{
		for(TNode_variable_list::nodelist::reverse_iterator it=node->aList.rbegin();it!=node->aList.rend();++it)
		{
			(*it)->accept(*this,0);
		}
	}
	else
	{
		cgen.kerror("error");
	}


	return def_value();
}

VisReturnType TNodeVisitorCG_GeneratorLValue::visit(TNode_expression_dot* node,VisExtraParam)
{
	node->tbl->accept(rvis,1);
	cgen.emit_setidx(node->token.word);	
	return def_value();
}

VisReturnType TNodeVisitorCG_GeneratorLValue::visit(TNode_expression_arr* node,VisExtraParam visp)
{
	visit_arr(node,visp,XOP_SET_ARRAY);
	return def_value();
}

VisReturnType TNodeVisitorCG_GeneratorLValue::visit(TNode_expression_call*,VisExtraParam)
{
	cgen.kerror("function call cannot be lvale");
	return def_value();
}

VisReturnType TNodeVisitorCG_GeneratorLValue::visit(TNode_expression_list* node,VisExtraParam visp)
{

	if(visp!=(VisExtraParam)node->aList.size())
	{
		cgen.kerror("error");
	}

	for(TNode_expression_list::nodelist::reverse_iterator it=node->aList.rbegin();it!=node->aList.rend();++it)
	{
		(*it)->accept(*this,0);
	}	
	return def_value();
}


EW_LEAVE
