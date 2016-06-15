#ifndef __H_EW_SCRIPTING_PARSER_NODE__
#define __H_EW_SCRIPTING_PARSER_NODE__

#include "ewa_base/scripting/scanner.h"
#include "ewa_base/scripting/variant.h"
#include "ewa_base/scripting/callable_table.h"
#include "ewa_base/scripting/callable_array.h"
#include "ewa_base/scripting/parser_visitor.h"

#define SYNTAX_TNODE_ACCEPT(X) \
virtual VisReturnType accept(TNodeVisitor& vis,VisExtraParam p)	\
{\
	vis.visit_enter(this);VisReturnType n=vis.visit(this,p);vis.visit_leave(this);return n;\
}

EW_ENTER

enum
{
	VAR_DEFAULT,
	VAR_LOCAL,
	VAR_GLOBAL,

	VAR_SYSTEM,
	VAR_IMPORT,
	VAR_EXPORT,
	VAR_CAPTURE,
	VAR_BREAKER,
};

class DLLIMPEXP_EWA_BASE CG_Variable : public mp_obj
{
public:

	enum
	{
		FLAG_READONLY		=1<<0,
		FLAG_CAPTURED		=1<<1,
		FLAG_INITIALIZED	=1<<3,
		FLAG_TOUCHED		=1<<4,
	};

	CG_Variable(int t,const String& s="");

	~CG_Variable();

	int type;
	BitFlags flags;
	String name;
	String module;
	int target;
	int index;
	LitePtrT<CG_Variable> next;

};


class CG_Block : public CG_Variable
{
public:

	enum
	{
		FLAG_BREAK_ABLE		=1<<5,
		FLAG_CONTINUE_ABLE	=1<<6,
		FLAG_EXCEPT			=1<<7,
	};

	CG_Block();
	~CG_Block();


	LitePtrT<CG_Block> parent;
	LitePtrT<CG_Variable> tail;


	BitFlags flags;

	arr_1t<int> m_aControlB;
	arr_1t<int> m_aControlC;

	void insert(CG_Variable* q);

private:
	CG_Block(const CG_Block&);
};


class DLLIMPEXP_EWA_BASE TNode : public ObjectData
{
public:

	enum
	{
		FLAG_LOCAL		=1<<0,
		FLAG_GLOBAL		=1<<1,

		FLAG_WITH_SEMI	=1<<3,
		FLAG_SUB_ASSIGN	=1<<4,
		FLAG_MAX		=1<<5,
	};

	TNode(){}
	TNode(const String& v):token(v){}
	TNode(const tokItem& v):token(v){}

	tokItem token;
	BitFlags flags;

	SYNTAX_TNODE_ACCEPT(TNode);
};


class DLLIMPEXP_EWA_BASE TNode_statement : public TNode
{
public:
	typedef TNode basetype;

	enum
	{
		FLAG_WITH_SEMI2	=basetype::FLAG_MAX<<0,
		FLAG_MAX		=basetype::FLAG_MAX<<1,
	};

	TNode_statement(){}
	TNode_statement(const String& v):basetype(v){}
	TNode_statement(const tokItem& v):basetype(v){}

	SYNTAX_TNODE_ACCEPT(TNode_statement);
};

class DLLIMPEXP_EWA_BASE TNode_statement_macro : public TNode_statement
{
public:
	typedef TNode_statement basetype;
	arr_1t<tokItem> param; 
	arr_1t<tokItem> extra;

	SYNTAX_TNODE_ACCEPT(TNode_statement_macro);
};

class DLLIMPEXP_EWA_BASE TNode_statement_nil : public TNode_statement
{
public:
	typedef TNode_statement basetype;
	SYNTAX_TNODE_ACCEPT(TNode_statement_nil);
};

class DLLIMPEXP_EWA_BASE TNode_var;
class DLLIMPEXP_EWA_BASE TNode_expression_list;
class DLLIMPEXP_EWA_BASE TNode_variable_list;

class DLLIMPEXP_EWA_BASE TNode_expression : public TNode
{
public:
	typedef TNode basetype;

	TNode_expression(){}
	TNode_expression(const String& v):basetype(v){}
	TNode_expression(const tokItem& v):basetype(v){}

	enum
	{
		FLAG_MIN			=basetype::FLAG_MAX,
		FLAG_POST			=FLAG_MIN<<0,
		FLAG_WITH_DOT3		=FLAG_MIN<<1,
		FLAG_SUB_INDEX		=FLAG_MIN<<2,
		FLAG_MAX			=FLAG_MIN<<3,
	};


	virtual int pm_count(){return 1;}
	virtual bool is_const(){return false;}

	virtual bool get_value(Variant&){return false;}

	virtual int dot3_pos(){return -1;}

	virtual TNode_var* to_var(){return NULL;}
	virtual TNode_expression_list* to_exp_list(){return NULL;}
	virtual TNode_variable_list* to_var_list(){return NULL;}

	SYNTAX_TNODE_ACCEPT(TNode_expression);
};


class DLLIMPEXP_EWA_BASE TNode_item : public TNode_expression
{
public:
	typedef TNode_expression basetype;

	TNode_item(){}
	TNode_item(const String& v):basetype(v){}
	TNode_item(const tokItem& v):basetype(v){}

	SYNTAX_TNODE_ACCEPT(TNode_item);
};


class DLLIMPEXP_EWA_BASE TNode_var : public TNode_item
{
public:
	typedef TNode_item basetype;

	TNode_var(){}
	TNode_var(const String& v):basetype(v){}
	TNode_var(const tokItem& v):basetype(v){}

	virtual TNode_var* to_var(){return this;}

	SYNTAX_TNODE_ACCEPT(TNode_var);
};

template<typename T,typename B=TNode>
class DLLIMPEXP_EWA_BASE TNode_list : public B
{
public:
	typedef B basetype;
	typedef ObjectGroupT<T> nodelist;

	virtual int pm_count(){return (int)aList.size();}

	int dot3_pos()
	{
		if(aList.size()==0) return -1;
		TNode_var* q=(*aList.rbegin())->to_var();
		if(q && q->token.word=="...")
		{
			return aList.size();
		}
		return -1;
	}

	nodelist aList;
};


class DLLIMPEXP_EWA_BASE TNode_variable_list : public TNode_list<TNode_var,TNode_expression>
{
public:
	typedef TNode_list<TNode_var,TNode_expression> basetype;
	TNode_variable_list* to_var_list(){return this;}

	SYNTAX_TNODE_ACCEPT(TNode_variable_list);
};


class DLLIMPEXP_EWA_BASE TNode_expression_list : public TNode_list<TNode_expression,TNode_expression>
{
public:
	typedef TNode_list<TNode_expression,TNode_expression> basetype;
	TNode_expression_list(){}
	TNode_expression_list* to_exp_list(){return this;}

	SYNTAX_TNODE_ACCEPT(TNode_expression_list);
};

class DLLIMPEXP_EWA_BASE TNode_braket : public TNode_item
{
public:
	typedef TNode_item basetype;
	enum
	{
		FLAG_MIN			=basetype::FLAG_MAX,
		FLAG_WITH_BRAKET1	=FLAG_MIN<<1,
		FLAG_WITH_BRAKET2	=FLAG_MIN<<2,
		FLAG_MAX			=FLAG_MIN<<3,
	};

	TNode_braket(){row_size=-1;}
	TNode_braket(const String& v):basetype(v){row_size=-1;}
	TNode_braket(const tokItem& v):basetype(v){row_size=-1;}

	DataPtrT<TNode_expression_list> exp_list;
	int row_size;

	SYNTAX_TNODE_ACCEPT(TNode_braket);
};


class DLLIMPEXP_EWA_BASE TNode_val : public TNode_item
{
public:
	typedef TNode_item basetype;
	TNode_val(){}
	TNode_val(const String& v):basetype(v){}
	TNode_val(const tokItem& v):basetype(v){}

	SYNTAX_TNODE_ACCEPT(TNode_val);
};


class DLLIMPEXP_EWA_BASE TNode_val_empty : public TNode_val
{
public:
	typedef TNode_val basetype;

	SYNTAX_TNODE_ACCEPT(TNode_val_empty);
};

class DLLIMPEXP_EWA_BASE TNode_val_nil : public TNode_val
{
public:
	typedef TNode_val basetype;

	virtual bool is_const(){return true;}
	virtual bool get_value(Variant& v){v.clear();return true;}

	SYNTAX_TNODE_ACCEPT(TNode_expression);
};

class DLLIMPEXP_EWA_BASE TNode_val_variant : public TNode_val
{
public:
	typedef TNode_val basetype;

	TNode_val_variant(const String& v):TNode_val(v)
	{
		token.type = TOK_STRING;
	}

	TNode_val_variant(const tokItem& v):TNode_val(v){}

	Variant value;

	virtual bool is_const(){return true;}
	virtual bool get_value(Variant& v){v=value;return true;}

	virtual bool update();

	SYNTAX_TNODE_ACCEPT(TNode_val_variant);
};

class DLLIMPEXP_EWA_BASE TNode_val_integer : public TNode_val_variant
{
public:
	typedef TNode_val_variant basetype;
	TNode_val_integer(const tokItem& v):basetype(v){}

	SYNTAX_TNODE_ACCEPT(TNode_val_integer);
};

class DLLIMPEXP_EWA_BASE TNode_val_boolean : public TNode_val_variant
{
public:

	typedef TNode_val_variant basetype;
	TNode_val_boolean(const tokItem& v):basetype(v){}

	bool update();

	SYNTAX_TNODE_ACCEPT(TNode_val_boolean);
};

class DLLIMPEXP_EWA_BASE TNode_expression_op : public TNode_expression
{
public:
	typedef TNode_expression basetype;
	TNode_expression_op(){}
	TNode_expression_op(const String& v):basetype(v){}
	TNode_expression_op(const tokItem& v):basetype(v){}

	virtual int size() const{return -1;}
	virtual TNode_expression* getp(int n){return NULL;}

	SYNTAX_TNODE_ACCEPT(TNode_expression_op);
};



class DLLIMPEXP_EWA_BASE TNode_expression_opn : public TNode_expression_op
{
public:
	typedef TNode_expression_op basetype;

	TNode_expression_opn(){}
	TNode_expression_opn(const String& v) :basetype(v){}
	TNode_expression_opn(const tokItem& v) :basetype(v){}

	arr_1t<DataPtrT<TNode_expression> > param;
	virtual int size() const{ return (int)param.size(); }
	virtual TNode_expression* getp(int n){ if (unsigned(n) >= param.size()) return NULL; return param[n].get(); }

	virtual int pm_count(){ return 1; }

	SYNTAX_TNODE_ACCEPT(TNode_expression_opn);
};

template<unsigned N>
class DLLIMPEXP_EWA_BASE TNode_expression_opT : public TNode_expression_op
{
public:
	typedef TNode_expression_op basetype;

	TNode_expression_opT(){}
	TNode_expression_opT(const String& v):basetype(v){}
	TNode_expression_opT(const tokItem& v):basetype(v){}

	DataPtrT<TNode_expression> param[N];
	virtual int size() const{return N;}
	virtual TNode_expression* getp(int n){if(unsigned(n)>=N) return NULL;return param[n].get();}

	virtual int pm_count(){return 1;}

};

class DLLIMPEXP_EWA_BASE TNode_expression_op1 : public TNode_expression_opT<1>
{
public:
	typedef TNode_expression_opT<1> basetype;
	TNode_expression_op1(){}
	TNode_expression_op1(const String& v):basetype(v){}
	TNode_expression_op1(const tokItem& v):basetype(v){}

	virtual bool is_const(){return param[0]->is_const();}
	SYNTAX_TNODE_ACCEPT(TNode_expression_op1);
};

class DLLIMPEXP_EWA_BASE TNode_expression_op2 : public TNode_expression_opT<2>
{
public:
	typedef TNode_expression_opT<2> basetype;

	TNode_expression_op2(){}
	TNode_expression_op2(const String& v):basetype(v){}
	TNode_expression_op2(const tokItem& v):basetype(v){}

	virtual bool is_const(){return param[0]->is_const()&&param[1]->is_const();}

	SYNTAX_TNODE_ACCEPT(TNode_expression_op2);
};

class DLLIMPEXP_EWA_BASE TNode_expression_op_assign : public TNode_expression_op2
{
public:
	typedef TNode_expression_op2 basetype;

	TNode_expression_op_assign(){}
	TNode_expression_op_assign(const String& v):basetype(v){}
	TNode_expression_op_assign(const tokItem& v):basetype(v){}

	virtual bool is_const(){return false;}
	virtual int pm_count(){return param[0]->pm_count();}

	SYNTAX_TNODE_ACCEPT(TNode_expression_op_assign);
};


class DLLIMPEXP_EWA_BASE TNode_val_decl : public TNode_val
{
public:
	typedef TNode_val basetype;
	TNode_val_decl(){}
	TNode_val_decl(const String& v):basetype(v){}
	TNode_val_decl(const tokItem& v):basetype(v){}

	tokInfo tok_beg,tok_end;


	CG_Block tScope;

	DataPtrT<TNode_expression> func_name;
	DataPtrT<TNode_statement_list> func_body;
	DataPtrT<TNode_variable_list> exp_list;

	size_t local_max(){return nLocal;}
	void local_max(size_t n){nLocal=n;}

	size_t m_nCaptureNum;
	String m_sHelp;

	SYNTAX_TNODE_ACCEPT(TNode_val_decl);
protected:
	size_t nLocal;

};

class DLLIMPEXP_EWA_BASE TNode_val_class : public TNode_val_decl
{
public:
	typedef TNode_val_decl basetype;
	TNode_val_class(){}
	TNode_val_class(const String& v):basetype(v){}
	TNode_val_class(const tokItem& v):basetype(v){}

	DataPtrT<TNode_expression_list> base_list;

	SYNTAX_TNODE_ACCEPT(TNode_val_class);
};


class DLLIMPEXP_EWA_BASE TNode_val_function : public TNode_val_decl
{
public:
	typedef TNode_val_decl basetype;
	TNode_val_function(){}
	TNode_val_function(const String& v):basetype(v){}
	TNode_val_function(const tokItem& v):basetype(v){}

	enum
	{
		FLAG_MIN		=basetype::FLAG_MAX,
		FLAG_THIS_USED	=FLAG_MIN<<0,
		FLAG_SELF_USED	=FLAG_MIN<<1,
		FLAG_DOT3_USED	=FLAG_MIN<<2,
		FLAG_MAX		=FLAG_MIN<<3,
	};


	void param_num(size_t n){nParam=n;}
	size_t param_num(){return nParam;}
	size_t capture_num(){return m_nCaptureNum;}
	void capture_num(size_t v){m_nCaptureNum=v;}

	bool add_capture(int d);

	arr_1t<int> aCaptured;
	size_t nParam;

	SYNTAX_TNODE_ACCEPT(TNode_val_function);

};

// function call: cls->tbl(exp_list)
class DLLIMPEXP_EWA_BASE TNode_expression_call : public TNode_expression
{
public:
	typedef TNode_expression basetype;
	TNode_expression_call(const tokItem& v):basetype(v){}

	DataPtrT<TNode_expression> tbl;
	DataPtrT<TNode_expression> cls;
	DataPtrT<TNode_expression_list> exp_list;

	virtual int pm_count(){return -1;}

	SYNTAX_TNODE_ACCEPT(TNode_expression_call);

};


class DLLIMPEXP_EWA_BASE TNode_expression_dot : public TNode_expression
{
public:
	typedef TNode_expression basetype;

	TNode_expression_dot(){}
	TNode_expression_dot(const String& v):basetype(v){}
	TNode_expression_dot(const tokItem& v):basetype(v){}

	DataPtrT<TNode_expression> tbl;

	virtual int pm_count(){return 1;}

	SYNTAX_TNODE_ACCEPT(TNode_expression_dot);
};

class DLLIMPEXP_EWA_BASE TNode_expression_arr : public TNode_expression
{
public:
	typedef TNode_expression basetype;
	TNode_expression_arr(const tokItem& item=tokItem()):basetype(item){}

	CG_Block tScope;

	DataPtrT<TNode_expression> tbl;
	DataPtrT<TNode_expression_list> exp_list;
	arr_1t<int> exp_flag;

	virtual int pm_count(){return 1;}

	SYNTAX_TNODE_ACCEPT(TNode_expression_arr);
};



class DLLIMPEXP_EWA_BASE TNode_statement_list : public TNode_list<TNode_statement,TNode_statement>
{
public:
	SYNTAX_TNODE_ACCEPT(TNode_statement_list);
};


class DLLIMPEXP_EWA_BASE TNode_statement_block : public TNode_statement_list
{
public:
	typedef TNode_statement_list basetype;
	CG_Block tScope;

	SYNTAX_TNODE_ACCEPT(TNode_statement_block);
};



class DLLIMPEXP_EWA_BASE TNode_statement_switch : public TNode_statement
{
public:
	typedef TNode_statement basetype;
	TNode_statement_switch(){}
	TNode_statement_switch(const String& v):basetype(v){}
	TNode_statement_switch(const tokItem& v):basetype(v){}

	CG_Block tScope;

	class case_item : public ObjectData
	{
	public:
		//ObjectGroupT<TNode_item> case_value;
		DataPtrT<TNode_expression> case_value;
		DataPtrT<TNode_statement_list> case_stmt;
	};

	typedef ObjectGroupT<case_item>::iterator case_iterator;
	//typedef ObjectGroupT<TNode_item>::iterator item_iterator;

	DataPtrT<TNode_expression> expr_cond;
	ObjectGroupT<case_item> case_list;

	SYNTAX_TNODE_ACCEPT(TNode_statement_switch);
};

class DLLIMPEXP_EWA_BASE TNode_statement_assignment : public TNode_statement
{
public:
	typedef TNode_statement basetype;

	DataPtrT<TNode_expression> value;
	SYNTAX_TNODE_ACCEPT(TNode_statement_assignment);
};

class DLLIMPEXP_EWA_BASE TNode_statement_if : public TNode_statement
{
public:
	typedef TNode_statement basetype;

	TNode_statement_if()
	{
		kep = 0;
	}

	DataPtrT<TNode_expression> expr_cond;
	DataPtrT<TNode> stmt_if_1;
	DataPtrT<TNode> stmt_if_0;
	int kep;

	SYNTAX_TNODE_ACCEPT(TNode_statement_if);
};

class DLLIMPEXP_EWA_BASE TNode_statement_control : public TNode_statement
{
public:
	typedef TNode_statement basetype;

	TNode_statement_control(){}
	TNode_statement_control(const String& v):basetype(v){}
	TNode_statement_control(const tokItem& v):basetype(v){}

	SYNTAX_TNODE_ACCEPT(TNode_statement_control);
};

class DLLIMPEXP_EWA_BASE TNode_statement_return : public TNode_statement
{
public:
	typedef TNode_statement basetype;
	DataPtrT<TNode_expression> value;

	SYNTAX_TNODE_ACCEPT(TNode_statement_return);
};

class DLLIMPEXP_EWA_BASE TNode_statement_throw : public TNode_statement
{
public:
	typedef TNode_statement basetype;
	TNode_statement_throw(){}
	TNode_statement_throw(const tokItem& v):basetype(v){}

	DataPtrT<TNode_expression_list> value;

	SYNTAX_TNODE_ACCEPT(TNode_statement_throw);
};

class DLLIMPEXP_EWA_BASE TNode_statement_try : public TNode_statement
{
public:
	typedef TNode_statement basetype;

	CG_Block tScope_try;
	DataPtrT<TNode_statement> pBlock_try;

	CG_Block tScope_catch;
	DataPtrT<TNode_variable_list> exp_list;
	DataPtrT<TNode_statement> pBlock_catch;


	CG_Block tScope_finally;
	DataPtrT<TNode_statement> pBlock_finally;

	SYNTAX_TNODE_ACCEPT(TNode_statement_try);
};

class DLLIMPEXP_EWA_BASE TNode_statement_loop : public TNode_statement
{
public:
	typedef TNode_statement basetype;

	TNode_statement_loop(){}
	TNode_statement_loop(const String& v):basetype(v){}
	TNode_statement_loop(const tokItem& v):basetype(v){}

	CG_Block tScope;

	DataPtrT<TNode_statement> stmt_init;
	DataPtrT<TNode_expression> expr_cond;
	DataPtrT<TNode_statement> stmt_fini;
	DataPtrT<TNode_statement> stmt_body;

	DataPtrT<TNode_expression> fe_container;
	DataPtrT<TNode_variable_list> fe_variable;

	SYNTAX_TNODE_ACCEPT(TNode_statement_loop);
};


EW_LEAVE
#endif
