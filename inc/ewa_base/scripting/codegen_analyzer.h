#ifndef __H_SCRIPTING_CODEGEN_ANALYZER__
#define __H_SCRIPTING_CODEGEN_ANALYZER__

#include "ewa_base/scripting/codegen_visitor.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE TNodeVisitorCG_Analyzer;
class DLLIMPEXP_EWA_BASE TNodeVisitorCG_AnalyzerRValue;
class DLLIMPEXP_EWA_BASE TNodeVisitorCG_AnalyzerLValue;

class DLLIMPEXP_EWA_BASE TNodeVisitorCG_AnalyzerUnit : public TNodeVisitorCG
{
public:
	typedef TNodeVisitorCG basetype;

	TNodeVisitorCG_AnalyzerUnit(TNodeVisitorCG_Analyzer& cg_);

	TNodeVisitorCG_Analyzer& avis;
	TNodeVisitorCG_AnalyzerLValue& lvis;
	TNodeVisitorCG_AnalyzerRValue& rvis;

	DataPtrT<TNode> node_replace;

	VisReturnType replace_by(TNode* p)
	{
		EW_ASSERT(!node_replace);
		node_replace.reset(p);
		return 0;
	}

	template<typename T,typename X>
	VisReturnType visit_it(X& node,VisExtraParam visp)
	{
		EW_ASSERT(!node_replace);
		VisReturnType r=node->accept(*this,visp);
		if(!node_replace) return r;
		T* q=dynamic_cast<T*>(node_replace.get());
		if(!q) kerror("invalid replace_by node");
		node=q;
		node_replace.reset(NULL);
		return r;
	}

	VisReturnType visit(TNode_braket* node,VisExtraParam);
	VisReturnType visit(TNode_expression_dot* node,VisExtraParam);
	VisReturnType visit(TNode_expression_arr* node,VisExtraParam);
	VisReturnType visit(TNode_expression_call* node,VisExtraParam visp);
	VisReturnType visit(TNode_expression_list* node,VisExtraParam visp);

};

class DLLIMPEXP_EWA_BASE TNodeVisitorCG_AnalyzerLValue : public TNodeVisitorCG_AnalyzerUnit
{
public:
	TNodeVisitorCG_AnalyzerLValue(TNodeVisitorCG_Analyzer& cg_);

	VisReturnType visit(TNode_var* node,VisExtraParam);
	VisReturnType visit(TNode* node,VisExtraParam);


};

class DLLIMPEXP_EWA_BASE TNodeVisitorCG_AnalyzerRValue : public TNodeVisitorCG_AnalyzerUnit
{
public:
	TNodeVisitorCG_AnalyzerRValue(TNodeVisitorCG_Analyzer& cg_);

	virtual VisReturnType visit(TNode*,VisExtraParam);
	virtual VisReturnType visit(TNode_var*,VisExtraParam);

	virtual VisReturnType visit(TNode_statement_block*,VisExtraParam);
	virtual VisReturnType visit(TNode_statement_switch* node,VisExtraParam);
	virtual VisReturnType visit(TNode_val_variant*,VisExtraParam);

	virtual VisReturnType visit(TNode_expression_op_assign*,VisExtraParam);

	virtual VisReturnType visit(TNode_val_class*,VisExtraParam);
	virtual VisReturnType visit(TNode_val_function*,VisExtraParam);

	virtual VisReturnType visit(TNode_statement_try*,VisExtraParam);

	virtual VisReturnType visit(TNode_statement_loop* node,VisExtraParam);

	virtual VisReturnType visit(TNode_expression_op1*, VisExtraParam);
	virtual VisReturnType visit(TNode_expression_op2*, VisExtraParam);
	virtual VisReturnType visit(TNode_item_select*, VisExtraParam);

	virtual VisReturnType visit(TNode_statement_assignment* node,VisExtraParam);

	virtual VisReturnType visit(TNode_statement_macro* node, VisExtraParam visp);

	bool show_temp;
};



class DLLIMPEXP_EWA_BASE TNodeVisitorCG_Analyzer : public TNodeVisitorCG
{
public:
	typedef TNodeVisitorCG basetype;

	TNodeVisitorCG_AnalyzerLValue lvis;
	TNodeVisitorCG_AnalyzerRValue rvis;


	void parse(TNode* node)	{node->accept(rvis,1);}

	TNodeVisitorCG_Analyzer(CodeGen& g);

	using basetype::visit;

	void kerror(const String& s);

	StackFrameInfoAnalyzer& back(){return *stk.back();}

	void define_var(const String& v,int t=VAR_LOCAL,int d=0);	
	void define_import(const String& n, const String& m);

	void getvar(const String& v);
	void setvar(const String& v);
	bool handle_var(const String& v);

	void stackframe_enter(TNode_val_function* node);
	void stackframe_leave();

protected:
	
	arr_1t<DataPtrT<StackFrameInfoAnalyzer> > stk;
};

EW_LEAVE
#endif
