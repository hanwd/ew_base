#ifndef __H_EW_SCRIPTING_PARSER_VISITOR__
#define __H_EW_SCRIPTING_PARSER_VISITOR__

#include "ewa_base/basic.h"

EW_ENTER

typedef intptr_t VisExtraParam;
typedef intptr_t VisReturnType;

class DLLIMPEXP_EWA_BASE TNode;

class DLLIMPEXP_EWA_BASE TNode_expression;
class DLLIMPEXP_EWA_BASE TNode_variable_list;
class DLLIMPEXP_EWA_BASE TNode_expression_list;
class DLLIMPEXP_EWA_BASE TNode_item;
class DLLIMPEXP_EWA_BASE TNode_var;
class DLLIMPEXP_EWA_BASE TNode_braket;
class DLLIMPEXP_EWA_BASE TNode_val;
class DLLIMPEXP_EWA_BASE TNode_val_empty;
class DLLIMPEXP_EWA_BASE TNode_val_nil;
class DLLIMPEXP_EWA_BASE TNode_val_boolean;
class DLLIMPEXP_EWA_BASE TNode_val_integer;
class DLLIMPEXP_EWA_BASE TNode_val_variant;

class DLLIMPEXP_EWA_BASE TNode_val_function;
class DLLIMPEXP_EWA_BASE TNode_val_class;

class DLLIMPEXP_EWA_BASE TNode_expression_op;
class DLLIMPEXP_EWA_BASE TNode_expression_op1;
class DLLIMPEXP_EWA_BASE TNode_expression_op2;
class DLLIMPEXP_EWA_BASE TNode_expression_op_assign;
class DLLIMPEXP_EWA_BASE TNode_expression_call;
class DLLIMPEXP_EWA_BASE TNode_expression_dot;
class DLLIMPEXP_EWA_BASE TNode_expression_arr;

class DLLIMPEXP_EWA_BASE TNode_statement;
class DLLIMPEXP_EWA_BASE TNode_statement_macro;
class DLLIMPEXP_EWA_BASE TNode_statement_nil;
class DLLIMPEXP_EWA_BASE TNode_statement_assignment;
class DLLIMPEXP_EWA_BASE TNode_statement_if;
class DLLIMPEXP_EWA_BASE TNode_statement_loop;

class DLLIMPEXP_EWA_BASE TNode_statement_list;
class DLLIMPEXP_EWA_BASE TNode_statement_block;
class DLLIMPEXP_EWA_BASE TNode_statement_switch;

class DLLIMPEXP_EWA_BASE TNode_statement_control;
class DLLIMPEXP_EWA_BASE TNode_statement_return;
class DLLIMPEXP_EWA_BASE TNode_statement_throw;
class DLLIMPEXP_EWA_BASE TNode_statement_try;

class DLLIMPEXP_EWA_BASE TNode_item_select;

// for future use

class DLLIMPEXP_EWA_BASE TNode_extern1;
class DLLIMPEXP_EWA_BASE TNode_extern2;







class DLLIMPEXP_EWA_BASE TNodeVisitor : public Object
{
public:
	virtual void parse(TNode*){}

	virtual void handle(TNode*);

	static inline VisReturnType def_value(){return VisReturnType();}
	static inline VisExtraParam def_param(){return VisExtraParam();}

	virtual VisReturnType visit(TNode*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_variable_list*,VisExtraParam){return def_value();}

	virtual VisReturnType visit(TNode_expression*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_val_nil*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_expression_list*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_item*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_var*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_braket*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_val*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_val_variant*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_val_integer*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_val_boolean*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_val_function*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_val_class*,VisExtraParam){return def_value();}

	virtual VisReturnType visit(TNode_expression_op*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_expression_op1*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_expression_op2*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_expression_op_assign*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_expression_call*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_expression_dot*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_expression_arr*,VisExtraParam){return def_value();}

	virtual VisReturnType visit(TNode_statement*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_statement_nil*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_statement_assignment*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_statement_if*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_statement_loop*,VisExtraParam){return def_value();}

	virtual VisReturnType visit(TNode_statement_list*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_statement_block*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_statement_control*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_statement_return*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_statement_switch*,VisExtraParam){return def_value();}


	virtual VisReturnType visit(TNode_extern1*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_extern2*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_item_select*,VisExtraParam){return def_value();}

	virtual VisReturnType visit(TNode_statement_macro*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_statement_throw*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_statement_try*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_val_empty*,VisExtraParam){return def_value();}

	virtual void visit_enter(TNode*){}
	virtual void visit_leave(TNode*){}


};


class DLLIMPEXP_EWA_BASE TNodeVisitorVisitBase : public TNodeVisitor
{
public:


	template<typename T>
	VisReturnType visit_base(T* v,VisExtraParam p){return visit((typename T::basetype*)v,p);}

	virtual VisReturnType visit(TNode*,VisExtraParam);
	virtual VisReturnType visit(TNode_variable_list*,VisExtraParam);

	virtual VisReturnType visit(TNode_expression*,VisExtraParam);
	virtual VisReturnType visit(TNode_val_nil*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_list*,VisExtraParam);
	virtual VisReturnType visit(TNode_item*,VisExtraParam);
	virtual VisReturnType visit(TNode_var*,VisExtraParam);
	virtual VisReturnType visit(TNode_val*,VisExtraParam);
	virtual VisReturnType visit(TNode_braket*,VisExtraParam);
	virtual VisReturnType visit(TNode_val_variant*,VisExtraParam);
	virtual VisReturnType visit(TNode_val_integer*,VisExtraParam);
	virtual VisReturnType visit(TNode_val_boolean*,VisExtraParam);

	virtual VisReturnType visit(TNode_expression_op*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_op1*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_op2*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_op_assign*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_call*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_dot*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_arr*,VisExtraParam);

	virtual VisReturnType visit(TNode_statement*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_nil*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_assignment*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_if*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_loop*,VisExtraParam);

	virtual VisReturnType visit(TNode_statement_list*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_block*,VisExtraParam);
	virtual VisReturnType visit(TNode_val_function*,VisExtraParam);

	virtual VisReturnType visit(TNode_val_class*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_control*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_return*,VisExtraParam);

	virtual VisReturnType visit(TNode_extern1*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_extern2*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_item_select*, VisExtraParam);

	virtual VisReturnType visit(TNode_statement_macro*,VisExtraParam){return def_value();}

	virtual VisReturnType visit(TNode_statement_throw*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_try*,VisExtraParam);

	virtual VisReturnType visit(TNode_val_empty*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_switch*,VisExtraParam);


};

class DLLIMPEXP_EWA_BASE TNodeVisitorRecursive : public TNodeVisitorVisitBase
{
public:

	virtual VisReturnType visit(TNode_variable_list*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_list*,VisExtraParam);
	virtual VisReturnType visit(TNode_braket*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_op*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_op1*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_op2*,VisExtraParam);
	virtual VisReturnType visit(TNode_item_select*,VisExtraParam);

	virtual VisReturnType visit(TNode_expression_call*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_dot*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_arr*,VisExtraParam);

	virtual VisReturnType visit(TNode_statement_assignment*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_if*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_loop*,VisExtraParam);

	virtual VisReturnType visit(TNode_statement_list*,VisExtraParam);
	virtual VisReturnType visit(TNode_val_function*,VisExtraParam);

	virtual VisReturnType visit(TNode_val_class*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_return*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_throw*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_try*,VisExtraParam);

	virtual VisReturnType visit(TNode_statement_switch*,VisExtraParam);

};

class DLLIMPEXP_EWA_BASE TNodeVisitorFormater : public TNodeVisitorVisitBase
{
public:

	TNodeVisitorFormater(){nIndent=0;}

	virtual VisReturnType visit(TNode_expression_list*,VisExtraParam);
	virtual VisReturnType visit(TNode_variable_list*,VisExtraParam);
	virtual VisReturnType visit(TNode_var*,VisExtraParam);
	virtual VisReturnType visit(TNode_val*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_op1*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_op2*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_op_assign*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_assignment*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_if*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_loop*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_nil*,VisExtraParam);

	virtual VisReturnType visit(TNode_statement_list*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_block*,VisExtraParam);
	virtual VisReturnType visit(TNode_val_function*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_call*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_dot*,VisExtraParam);
	virtual VisReturnType visit(TNode_expression_arr*,VisExtraParam);

	virtual VisReturnType visit(TNode_val_class*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_control*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_statement_return*,VisExtraParam){return def_value();}

	virtual VisReturnType visit(TNode_extern1*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_extern2*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_item_select*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_statement_macro*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_statement_throw*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_statement_try*,VisExtraParam){return def_value();}

	virtual VisReturnType visit(TNode_val_empty*,VisExtraParam){return def_value();}
	virtual VisReturnType visit(TNode_statement_switch*,VisExtraParam){return def_value();}



	void indent();
	void indent_inc();
	void indent_dec();

	int nIndent;
	StringBuffer<char> aBuffer;
};



EW_LEAVE
#endif
