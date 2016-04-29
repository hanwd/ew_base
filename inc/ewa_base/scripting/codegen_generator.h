#ifndef __H_EW_SCRIPTING_CODEGEN_GENERATOR__
#define __H_EW_SCRIPTING_CODEGEN_GENERATOR__

#include "ewa_base/scripting/codegen_visitor.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE TNodeVisitorCG_Generator;
class DLLIMPEXP_EWA_BASE TNodeVisitorCG_GeneratorRValue;
class DLLIMPEXP_EWA_BASE TNodeVisitorCG_GeneratorLValue;


class DLLIMPEXP_EWA_BASE TNodeVisitorCG_GeneratorUnit : public TNodeVisitorCG
{
public:

	typedef TNodeVisitorCG basetype;

	TNodeVisitorCG_GeneratorUnit(TNodeVisitorCG_Generator& cg_);

	TNodeVisitorCG_GeneratorLValue& lvis;
	TNodeVisitorCG_GeneratorRValue& rvis;

	VisReturnType visit_arr(TNode_expression_arr*,VisExtraParam,int);

};

class DLLIMPEXP_EWA_BASE TNodeVisitorCG_GeneratorRValue : public TNodeVisitorCG_GeneratorUnit
{
public:
	TNodeVisitorCG_GeneratorRValue(TNodeVisitorCG_Generator& cg_):TNodeVisitorCG_GeneratorUnit(cg_){}


	virtual void handle(TNode*);

	virtual VisReturnType visit(TNode*,VisExtraParam);
	virtual VisReturnType visit(TNode_variable_list*,VisExtraParam);

	virtual VisReturnType visit(TNode_expression*,VisExtraParam);
	virtual VisReturnType visit(TNode_val_nil*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_list*,VisExtraParam);
	virtual VisReturnType visit(TNode_braket*,VisExtraParam);

	virtual VisReturnType visit(TNode_item*,VisExtraParam);
	virtual VisReturnType visit(TNode_var*,VisExtraParam);
	virtual VisReturnType visit(TNode_val*,VisExtraParam);
	virtual VisReturnType visit(TNode_val_variant*,VisExtraParam);
	virtual VisReturnType visit(TNode_val_integer*,VisExtraParam);
	virtual VisReturnType visit(TNode_val_boolean*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_op*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_op1*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_op2*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_opn*, VisExtraParam);

	virtual VisReturnType visit(TNode_expression_op_assign*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_call*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_dot*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_arr*,VisExtraParam);

	virtual VisReturnType visit(TNode_statement*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_nil*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_assignment*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_if*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_loop*,VisExtraParam);
	virtual VisReturnType visit(TNode_val_class*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_list*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_block*,VisExtraParam);
	virtual VisReturnType visit(TNode_val_function*,VisExtraParam);
	virtual VisReturnType visit(TNode_val_empty*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_return*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_control*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_switch*,VisExtraParam);

	virtual VisReturnType visit(TNode_statement_try*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_throw*,VisExtraParam);

	virtual VisReturnType visit(TNode_statement_macro*, VisExtraParam);


};

class DLLIMPEXP_EWA_BASE TNodeVisitorCG_GeneratorLValue : public TNodeVisitorCG_GeneratorUnit
{
public:
	TNodeVisitorCG_GeneratorLValue(TNodeVisitorCG_Generator& cg_):TNodeVisitorCG_GeneratorUnit(cg_){}

	virtual void handle(TNode*);

	virtual VisReturnType visit(TNode_var*,VisExtraParam);
	virtual VisReturnType visit(TNode_variable_list*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_dot*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_arr*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_list*,VisExtraParam);
	virtual VisReturnType visit(TNode_val_nil*,VisExtraParam);

	virtual VisReturnType visit(TNode_expression_op2* node,VisExtraParam p);
	virtual VisReturnType visit(TNode_expression_op1* node,VisExtraParam p);


};


class DLLIMPEXP_EWA_BASE TNodeVisitorCG_Generator : public TNodeVisitorCG
{
public:

	TNodeVisitorCG_GeneratorLValue lvis;
	TNodeVisitorCG_GeneratorRValue rvis;

	virtual void parse(TNode* q);

	TNodeVisitorCG_Generator(CodeGen& g);
	void kerror(const String& s);

};

EW_LEAVE

#endif
