#include "ewa_base/scripting/parser.h"
#include "ewa_base/scripting/parser_nodes.h"

EW_ENTER


indexer_map<String, int>& get_op_table_instance()
{
	static indexer_map<String, int> m;
	if (m.empty())
	{
		m["++"] = 16;
		m["--"] = 16;
		m["~"] = 16;
		m["!"] = 16;


		m["**"] = 14;
		m[".**"] = 14;

		m[".*"] = 13;
		m["./"] = 13;
		m[".\\"] = 13;
		m["*"] = 13;
		m["/"] = 13;
		m["\\"] = 13;
		m["%"] = 12;

		m["+"] = 10;
		m["-"] = 10;

		m["&"] = 9;
		m["^"] = 8;
		m["|"] = 7;

		m["<<"] = 7;
		m[">>"] = 7;

		// 6 is reserved for colon a:b

		m[">"] = 5;
		m["<"] = 5;
		m[">="] = 5;
		m["<="] = 5;
		m["=="] = 5;
		m["==="] = 5;
		m["!="] = 5;

		m["&&"] = 4;
		m["^^"] = 3;
		m["||"] = 2;
		m[".."] = 2;

		//m["1"] = 15;
		// 1 is reserved for comma, a,b,c
		m["="] = 0;
		m["+="] = 0;
		m["-="] = 0;
		m["*="] = 0;

		m["/="] = 0;
		m["\\="] = 0;
		m["+="] = 0;
		m["&="] = 0;
		m["|="] = 0;
		m["^="] = 0;

		m["=>"] = 0;

		m["**="] = 0;
		m[">>="] = 0;
		m["<<="] = 0;

	}
	return m;
}


Parser::Parser():op_table(get_op_table_instance())
{

}


class parser_handler
{
public:

	static bool is_stmt_end(tokItem* pcur)
	{
		tokType tk=pcur[0].type;
		if(tk==TOK_KEY && pcur[0].word=="case") return true;
		if(tk==TOK_KEY && pcur[0].word=="cond") return true;
		if(tk==TOK_KEY && pcur[0].word=="default") return true;
		return 
			tk==TOK_END||
			tk==TOK_KET1||
			tk==TOK_KET2||
			tk==TOK_KET3 ||
			tk==TOK_EQGT;
	}

	static bool is_expr_end(tokItem* pcur)
	{
		tokType tk=pcur[0].type;
		return 
			tk == TOK_END || 
			tk == TOK_KET1 || 
			tk == TOK_KET2 || 
			tk == TOK_KET3 || 
			tk == TOK_SEMICOLON || 
			tk == TOK_SHARP ||
			tk == TOK_EQGT;
	}

	static bool is_ket3_end(tokItem* pcur)
	{
		tokType tk=pcur[0].type;
		return tk==TOK_END||tk==TOK_KET3;
	}
};

template<typename T>
class read_node_handler : public parser_handler
{
public:
	static DataPtrT<T> g(Parser& parser);
};

template<int N>
class read_expr_handler : public parser_handler
{
public:
	static DataPtrT<TNode_expression> g(Parser& parser);
};

template<> DataPtrT<TNode_item> read_node_handler<TNode_item>::g(Parser& parser);


template<> DataPtrT<TNode_statement> read_node_handler<TNode_statement>::g(Parser& parser);
template<> DataPtrT<TNode_statement_assignment> read_node_handler<TNode_statement_assignment>::g(Parser& parser);
template<> DataPtrT<TNode_statement_block> read_node_handler<TNode_statement_block>::g(Parser& parser);
template<> DataPtrT<TNode_statement_control> read_node_handler<TNode_statement_control>::g(Parser& parser);
template<> DataPtrT<TNode_statement_if> read_node_handler<TNode_statement_if>::g(Parser& parser);
template<> DataPtrT<TNode_statement_list> read_node_handler<TNode_statement_list>::g(Parser& parser);
template<> DataPtrT<TNode_statement_return> read_node_handler<TNode_statement_return>::g(Parser& parser);
template<> DataPtrT<TNode_statement_loop> read_node_handler<TNode_statement_loop>::g(Parser& parser);
template<> DataPtrT<TNode_statement_switch> read_node_handler<TNode_statement_switch>::g(Parser& parser);
template<> DataPtrT<TNode_expression> read_node_handler<TNode_expression>::g(Parser& parser);
template<> DataPtrT<TNode_expression_list> read_node_handler<TNode_expression_list>::g(Parser& parser);
template<> DataPtrT<TNode_val_class> read_node_handler<TNode_val_class>::g(Parser& parser);
template<> DataPtrT<TNode_val_function> read_node_handler<TNode_val_function>::g(Parser& parser);
template<> DataPtrT<TNode_variable_list> read_node_handler<TNode_variable_list>::g(Parser& parser);
template<> DataPtrT<TNode_statement_try> read_node_handler<TNode_statement_try>::g(Parser& parser);
template<> DataPtrT<TNode_statement_throw> read_node_handler<TNode_statement_throw>::g(Parser& parser);


template<>
DataPtrT<TNode_item> read_node_handler<TNode_item>::g(Parser& parser)
{
	DataPtrT<TNode_item> node;

	if(parser.test(TOK_BRA1))
	{
		DataPtrT<TNode_braket> q=new TNode_braket;
		q->flags.add(TNode_braket::FLAG_WITH_BRAKET1);
		q->exp_list=read_node_handler<TNode_expression_list>::g(parser);
		parser.match(TOK_KET1);
		node=q;
	}
	else if(parser.test(TOK_BRA2))
	{
		DataPtrT<TNode_braket> q=new TNode_braket;
		q->flags.add(TNode_braket::FLAG_WITH_BRAKET2);
		q->exp_list=read_node_handler<TNode_expression_list>::g(parser);
		parser.match(TOK_KET2);
		node=q;
	}
	else if(parser.test(TOK_ID))
	{
		node.reset(new TNode_var(parser.pcur[-1]));
	}
	else if(parser.test(TOK_INTEGER))
	{
		node.reset(new TNode_val_integer(parser.pcur[-1]));
	}
	else if(parser.test(TOK_DOUBLE)||parser.test(TOK_STRING)||parser.test(TOK_IMAGPART))
	{
		node.reset(new TNode_val_variant(parser.pcur[-1]));
	}
	else if(parser.test(TOK_KEY,"true"))
	{
		node.reset(new TNode_val_boolean(parser.pcur[-1]));
	}
	else if(parser.test(TOK_KEY,"false"))
	{
		node.reset(new TNode_val_boolean(parser.pcur[-1]));
	}
	else if(parser.test(TOK_KEY,"nil"))
	{
		node.reset(new TNode_val_nil);
	}
	else if(parser.test(TOK_KEY,"begin"))
	{
		node.reset(new TNode_var(parser.pcur[-1]));
	}
	else if(parser.test(TOK_KEY,"end"))
	{
		node.reset(new TNode_var(parser.pcur[-1]));
	}
	else if(parser.pcur[0].type==TOK_KEY && (parser.pcur[0].word=="function"||parser.pcur[0].word=="def"))
	{
		node=read_node_handler<TNode_val_function>::g(parser);
	}
	else if(parser.pcur[0].type==TOK_KEY && parser.pcur[0].word=="class")
	{
		node=read_node_handler<TNode_val_class>::g(parser);
	}
	else if(parser.pcur[0].type==TOK_SEMICOLON||parser.pcur[0].type==TOK_COLON||parser.pcur[0].type==TOK_COMMA)
	{
		return new TNode_val_empty;
	}
	else if(parser.pcur[0].type==TOK_KET1||parser.pcur[0].type==TOK_KET2)
	{
		return new TNode_val_empty;
	}
	else
	{
		parser.kerror("item expected");
	}

	return node;
}




template<int N>
DataPtrT<TNode_expression> read_expr_handler<N>::g(Parser& parser)
{
	indexer_map<String,int>& op_table(parser.op_table);

	DataPtrT<TNode_expression> node(read_expr_handler<N+1>::g(parser));
	while(parser.pcur[0].type==TOK_OP)
	{
		if(op_table[parser.pcur[0].word]!=N)
		{
			break;
		}

		TNode_expression_op2 *n=new TNode_expression_op2(parser.pcur[0]);

		n->param[0]=node;
		node.reset(n);

		++parser.pcur;
		n->param[1]=read_expr_handler<N+1>::g(parser);
	}

	return node;
}


template<>
DataPtrT<TNode_expression> read_expr_handler<6>::g(Parser& parser)
{
	const int N=6;
	DataPtrT<TNode_expression> node(read_expr_handler<N+1>::g(parser));
	if(parser.test(TOK_COLON))
	{
		TNode_expression_call* n = new TNode_expression_call(parser.pcur[-1]);
		TNode_expression_list* l=new TNode_expression_list;
		n->exp_list.reset(l);
		l->aList.append(node.get());

		parser.pcur[-1].word=":";
		n->tbl.reset(new TNode_var(parser.pcur[-1]));

		node.reset(n);

		l->aList.append(read_expr_handler<N+1>::g(parser));
		if(parser.test(TOK_COLON))
		{
			l->aList.append(read_expr_handler<N+1>::g(parser));
		}
	}

	return node;
}



template<>
DataPtrT<TNode_expression> read_expr_handler<1>::g(Parser& parser)
{
	indexer_map<String,int>& op_table(parser.op_table);

	const int N=1;

	DataPtrT<TNode_expression_list> node(new TNode_expression_list);

	for (;;)
	{
		while (parser.test(TOK_COMMA))
		{
			node->aList.append(new TNode_val_nil);
		}

		if (parser.pcur[0].type == TOK_OP&&op_table[parser.pcur[0].word] < 1)
		{
			break;
		}

		if (is_expr_end(parser.pcur))
		{
			break;
		}


		DataPtrT<TNode_expression> item(read_expr_handler<N + 1>::g(parser));

		while (parser.test(TOK_QUESTION))
		{
			DataPtrT<TNode_expression_opn> opn = new TNode_expression_opn(parser.pcur[-1]);
			opn->param.resize(3);
			opn->param[0].swap(item);

			opn->param[1]=read_expr_handler<N + 1>::g(parser);
			parser.match(TOK_SHARP);
			opn->param[2]=read_expr_handler<N + 1>::g(parser);

			item = opn;
		}

		node->aList.append(item.get());

		if(!parser.test(TOK_COMMA))
		{
			break;
		}

		if(is_expr_end(parser.pcur))
		{
			node->aList.append(new TNode_val_nil);
			break;
		}
	}

	if (!node->aList.empty())
	{
		TNode_var* pvar=node->aList.back()->to_var();
		if(pvar && pvar->token.word=="...")
		{
			node->flags.add(TNode_expression::FLAG_WITH_DOT3);
		}
	}

	return node;
}



template<>
DataPtrT<TNode_expression> read_expr_handler<0>::g(Parser& parser)
{
	indexer_map<String,int>& op_table(parser.op_table);

	const int N =0;

	LockState<BitFlags> lock1(parser.flags,0);

	BitFlags var_flags;

	while(parser.pcur[0].type==TOK_KEY)
	{
		if(parser.pcur[0].word=="local")
		{
			++parser.pcur;
			var_flags.add(TNode_expression::FLAG_LOCAL);
		}
		else if(parser.pcur[0].word=="global")
		{
			++parser.pcur;
			var_flags.add(TNode_expression::FLAG_GLOBAL);
		}
		else
		{
			break;
		}
	}


	DataPtrT<TNode_expression> node;

	if(var_flags.val())
	{
		DataPtrT<TNode_variable_list> vl=read_node_handler<TNode_variable_list>::g(parser);
		for(TNode_variable_list::nodelist::iterator it=vl->aList.begin();it!=vl->aList.end();++it)
		{
			(*it)->flags.add(var_flags.val());
		}
		node=vl;
	}
	else
	{
		node=read_expr_handler<N+1>::g(parser);
	}

	if(parser.pcur[0].type!=TOK_OP||op_table[parser.pcur[0].word]!=0)
	{
		DataPtrT<TNode_expression_op_assign> nopa=new TNode_expression_op_assign;
		nopa->flags.add(var_flags.val());

		if(var_flags.val()!=0)
		{
			nopa->param[0]=node;
			nopa->param[1].reset(new TNode_expression_list);
		}
		else
		{
			nopa->param[0].reset(new TNode_variable_list);
			nopa->param[1]=node;
		}

		return nopa;
	}

	parser.flags.clr(0);

	DataPtrT<TNode_expression_op_assign> ndeq,ntmp;

	while(parser.pcur[0].type==TOK_OP)
	{
		if(op_table[parser.pcur[0].word]!=0)
		{
			break;
		}

		ntmp=new TNode_expression_op_assign(parser.pcur[0]);
		ntmp->flags.add(var_flags.val());var_flags.clr(0);

		if(ndeq)
		{
			ntmp->param[0]=ndeq->param[1];
			ndeq->param[1]=ntmp;

			ndeq->flags.add(TNode::FLAG_SUB_ASSIGN);
		}
		else
		{
			ntmp->param[0]=node;
			node=ntmp;
		}

		ndeq=ntmp;

		++parser.pcur;
		ntmp->param[1]=read_expr_handler<N+1>::g(parser);
	}

	return node;
}


template<>
DataPtrT<TNode_expression> read_expr_handler<12>::g(Parser& parser)
{
	indexer_map<String,int>& op_table(parser.op_table);

	const int N =12;

	typedef arr_1t<tokItem> astr_array;

	astr_array aOp;
	while(parser.pcur[0].type==TOK_OP)
	{
		String &s(parser.pcur[0].word);
		if(s=="+"||s=="-"||s=="!"||s=="~")
		{
			aOp.push_back(parser.pcur[0]);
		}
		else
		{
			break;
		}
		++parser.pcur;
	}

	DataPtrT<TNode_expression> node(read_expr_handler<N+1>::g(parser));
	while(parser.pcur[0].type==TOK_OP)
	{
		if(op_table[parser.pcur[0].word]!=N)
		{
			break;
		}

		TNode_expression_op2 *n=new TNode_expression_op2(parser.pcur[0]);

		n->param[0]=node;
		node.reset(n);

		++parser.pcur;
		n->param[1]=read_expr_handler<N+1>::g(parser);
	}


	for(astr_array::reverse_iterator it=aOp.rbegin();it!=aOp.rend();++it)
	{
		TNode_expression_op1 *n=new TNode_expression_op1(*it);
		n->param[0]=node;
		node.reset(n);
	}


	return node;
}

template<>
DataPtrT<TNode_expression> read_expr_handler<15>::g(Parser& parser)
{
	typedef arr_1t<tokItem> astr_array;
	DataPtrT<TNode_expression> node;
	astr_array aOp;
	while(parser.pcur[0].type==TOK_OP)
	{
		String &s(parser.pcur[0].word);
		if(s=="++"||s=="--")
		{
			aOp.push_back(parser.pcur[0]);
		}
		else
		{
			parser.kexpected("expression start");
		}
		++parser.pcur;
	}

	node=read_node_handler<TNode_item>::g(parser);

	for(;;)
	{
		if(parser.test(TOK_DOT))
		{
			if(!parser.test(TOK_ID)&&!parser.test(TOK_KEY))
			{
				parser.kerror("id");
			}
			TNode_expression_dot *n=new TNode_expression_dot(parser.pcur[-1]);
			n->tbl=node;

			node.reset(n);
		}
		else if(parser.test(TOK_BRA2))
		{
			TNode_expression_arr *n=new TNode_expression_arr(parser.pcur[-1]);
			n->tbl=node;
			node.reset(n);
			n->exp_list=read_node_handler<TNode_expression_list>::g(parser);
			parser.match(TOK_KET2);
		}
		else if(parser.test(TOK_BRA1))
		{
			TNode_expression_call* n=new TNode_expression_call(parser.pcur[-1]);
			n->tbl=node;node.reset(n);
			n->exp_list=read_node_handler<TNode_expression_list>::g(parser);
			parser.match(TOK_KET1);
		}
		else if(parser.test(TOK_PTR))
		{
			TNode_expression_call* n=new TNode_expression_call(tokItem());
			n->cls=node;node.reset(n);
			n->tbl=read_node_handler<TNode_item>::g(parser);
			parser.match(TOK_BRA1);
			n->token = parser.pcur[-1];
			n->exp_list=read_node_handler<TNode_expression_list>::g(parser);
			parser.match(TOK_KET1);
		}
		else
		{
			break;
		}
	}


	for(astr_array::reverse_iterator it=aOp.rbegin();it!=aOp.rend();++it)
	{
		TNode_expression_op1 *n=new TNode_expression_op1(*it);
		n->param[0]=node;
		node.reset(n);
	}

	while(parser.pcur[0].type==TOK_OP && (parser.pcur[0].word=="++"||parser.pcur[0].word=="--"))
	{
		TNode_expression_op1 *n=new TNode_expression_op1(parser.pcur[0]);
		n->param[0]=node;
		n->flags.add(TNode_expression_op1::FLAG_POST);
		node.reset(n);
		++parser.pcur;
	}


	return node;
}



template<>
DataPtrT<TNode_expression> read_node_handler<TNode_expression>::g(Parser& parser)
{
	return read_expr_handler<2>::g(parser);
}


template<>
DataPtrT<TNode_expression_list> read_node_handler<TNode_expression_list>::g(Parser& parser)
{
	if(is_expr_end(parser.pcur))
	{
		return new TNode_expression_list;
	}

	LockState<BitFlags> lock1(parser.flags,0);
	DataPtrT<TNode_expression> q=read_expr_handler<1>::g(parser);
	TNode_expression_list* exp_list=q->to_exp_list();

	if(exp_list==NULL)
	{
		parser.kerror("invalid exp_list");
	}

	return exp_list;
}

template<>
DataPtrT<TNode_statement_list> read_node_handler<TNode_statement_list>::g(Parser& parser)
{
	DataPtrT<TNode_statement_list> node(new TNode_statement_list);
	while(!is_stmt_end(parser.pcur))
	{
		DataPtrT<TNode_statement> d=read_node_handler<TNode_statement>::g(parser);
		if(!d)
		{
			continue;
		}

		node->aList.append(d.get());
	}
	return node;
}


template<>
DataPtrT<TNode_statement_block> read_node_handler<TNode_statement_block>::g(Parser& parser)
{

	parser.match(TOK_BRA3);

	DataPtrT<TNode_statement_block> node(new TNode_statement_block);
	DataPtrT<TNode_statement_list> q(read_node_handler<TNode_statement_list>::g(parser));
	if(q)
	{
		node->aList.swap(q->aList);
	}
	parser.match(TOK_KET3);

	return node;

}



template<>
DataPtrT<TNode_statement> read_node_handler<TNode_statement>::g(Parser& parser)
{
	DataPtrT<TNode_statement> q;
	if(parser.pcur[0].type==TOK_SEMICOLON)
	{
		q=new TNode_statement_nil;
	}
	else if(parser.pcur[0].type==TOK_SHARP)
	{
		DataPtrT<TNode_statement_macro> s(new TNode_statement_macro);
		q=s.get();

		int32_t nline=parser.pcur->line;
		++parser.pcur;

		if(nline!=parser.pcur->line||(!parser.test(TOK_KEY) && !parser.test(TOK_ID)))
		{
			parser.kexpected("id");
		}

		s->token=parser.pcur[-1];

		while(nline==parser.pcur->line && (parser.test(TOK_KEY)||parser.test(TOK_ID)))
		{
			s->extra.push_back(parser.pcur[-1]);
			if (nline == parser.pcur->line)
			{
				parser.test(TOK_COMMA);
			}
		}

		if (nline == parser.pcur->line && parser.test(TOK_BRA1))
		{
			for(;;)
			{
				if(parser.test(TOK_KEY)||parser.test(TOK_ID))
				{
					s->param.push_back(parser.pcur[-1]);
				}

				if(parser.test(TOK_KET1))
				{
					//if(nline!=parser.pcur->line)
					//{
					//	parser.kerror("syntax error");
					//}
					break;
				}

				if(parser.test(TOK_END)) parser.kerror("invalid end");

				if(parser.test(TOK_COMMA)||parser.test(TOK_COLON))
				{
					continue;
				}

				parser.kerror("syntax error");
			}
		}

		parser.test(TOK_COMMA);

	}	
	else if(parser.pcur[0].type==TOK_KEY)
	{
		const String& sval(parser.pcur[0].word);
		if(sval=="if")
		{
			q=read_node_handler<TNode_statement_if>::g(parser);
		}
		else if(sval=="while"||sval=="for"||sval=="do"||sval=="for_each")
		{
			q=read_node_handler<TNode_statement_loop>::g(parser);
		}
		else if(sval=="function"||sval=="global"||sval=="local"||sval=="class")
		{
			q=read_node_handler<TNode_statement_assignment>::g(parser);
		}
		else if(sval=="break"||sval=="continue"||sval=="continue2"||sval=="break2"||sval=="break3")
		{
			q=read_node_handler<TNode_statement_control>::g(parser);
		}
		else if(sval=="return")
		{
			q=read_node_handler<TNode_statement_return>::g(parser);
		}
		else if(sval=="try")
		{
			q=read_node_handler<TNode_statement_try>::g(parser);
		}
		else if(sval=="throw")
		{
			q=read_node_handler<TNode_statement_throw>::g(parser);
		}
		else if(sval=="switch")
		{
			q=read_node_handler<TNode_statement_switch>::g(parser);
		}
		else if(sval=="judge")
		{
			q = read_node_handler<TNode_statement_switch>::g(parser);
		}
		else
		{
			q=read_node_handler<TNode_statement_assignment>::g(parser);
		}
	}
	else if(is_stmt_end(parser.pcur))
	{
		q=new TNode_statement_nil;
	}
	else if(parser.pcur[0].type==TOK_BRA3)
	{
		q=read_node_handler<TNode_statement_block>::g(parser);
	}
	else
	{
		q=read_node_handler<TNode_statement_assignment>::g(parser);
	}

	if(parser.test(TOK_SEMICOLON))
	{
		q->flags.add(TNode_statement::FLAG_WITH_SEMI);
	}
	return q;
}

template<>
DataPtrT<TNode_variable_list> read_node_handler<TNode_variable_list>::g(Parser& parser)
{
	DataPtrT<TNode_variable_list> node(new TNode_variable_list);
	while(parser.pcur[0].type==TOK_ID)
	{
		node->aList.append(new TNode_var(parser.pcur[0].word));
		++parser.pcur;

		if(parser.test(TOK_COMMA))
		{
			while(parser.test(TOK_COMMA))
			{
				node->aList.append(new TNode_var(""));
			}

			if(parser.pcur[0].type!=TOK_ID)
			{
				parser.kexpected("id");
			}
			continue;
		}

		if(node->aList[node->aList.size()-1]->token.word=="...")
		{
			node->flags.add(TNode_expression::FLAG_WITH_DOT3);
		}

		break;
	}

	return node;
}


template<>
DataPtrT<TNode_expression> read_expr_handler<-1>::g(Parser& parser)
{
	//typedef arr_1t<tokItem> astr_array;
	DataPtrT<TNode_expression> node;

	node=read_node_handler<TNode_item>::g(parser);

	for(;;)
	{
		if(parser.test(TOK_DOT))
		{
			if(!parser.test(TOK_ID))
			{
				parser.kerror("id");
			}
			TNode_expression_dot *n=new TNode_expression_dot(parser.pcur[-1]);
			n->tbl=node;

			node.reset(n);
		}
		else if(parser.test(TOK_BRA2))
		{
			TNode_expression_arr *n=new TNode_expression_arr(parser.pcur[-1]);
			n->tbl=node;
			node.reset(n);
			n->exp_list=read_node_handler<TNode_expression_list>::g(parser);
			parser.match(TOK_KET2);
		}
		else
		{
			break;
		}
	}

	return node;
}


template<>
DataPtrT<TNode_val_function> read_node_handler<TNode_val_function>::g(Parser& parser)
{

	DataPtrT<TNode_val_function> node(new TNode_val_function(*parser.pcur));

	node->m_sHelp=parser.get_comment(parser.pcur->line);

	node->tok_beg=parser.pcur[0];

	++parser.pcur;

	if(parser.pcur[0].type!=TOK_BRA1)
	{
		if(parser.pcur[0].type!=TOK_ID)
		{
			parser.kexpected("function name");
		}
		node->func_name=read_expr_handler<-1>::g(parser);
	}

	parser.match(TOK_BRA1);
	node->exp_list=read_node_handler<TNode_variable_list>::g(parser);
	parser.match(TOK_KET1);

	node->func_body=read_node_handler<TNode_statement_block>::g(parser);

	node->tok_end=parser.pcur[-1];

	return node;

}

template<>
DataPtrT<TNode_val_class> read_node_handler<TNode_val_class>::g(Parser& parser)
{

	DataPtrT<TNode_val_class> node(new TNode_val_class(*parser.pcur));

	++parser.pcur;

	if(parser.pcur[0].type!=TOK_BRA3)
	{
		node->func_name=read_expr_handler<-1>::g(parser);
	}

	node->func_body=read_node_handler<TNode_statement_block>::g(parser);

	return node;

}

template<>
DataPtrT<TNode_statement_if> read_node_handler<TNode_statement_if>::g(Parser& parser)
{
	DataPtrT<TNode_statement_if> node(new TNode_statement_if);
	++parser.pcur;
	parser.match(TOK_BRA1);
	node->expr_cond=read_node_handler<TNode_expression>::g(parser);
	parser.match(TOK_KET1);
	node->stmt_if_1=read_node_handler<TNode_statement>::g(parser);
	if(parser.test(TOK_KEY,"else"))
	{
		node->stmt_if_0=read_node_handler<TNode_statement>::g(parser);
	}
	return node;
}

template<>
DataPtrT<TNode_statement_switch> read_node_handler<TNode_statement_switch>::g(Parser& parser)
{
	DataPtrT<TNode_statement_switch> node(new TNode_statement_switch(*parser.pcur));
	++parser.pcur;
	parser.match(TOK_BRA1);

	if (node->token.word == "switch")
	{
		node->expr_cond=read_node_handler<TNode_expression_list>::g(parser);
	}
	else if (node->token.word == "judge")
	{
		node->expr_cond=read_node_handler<TNode_variable_list>::g(parser);
	}
	else
	{
		parser.kerror("unknown decl");
	}

	parser.match(TOK_KET1);
	parser.match(TOK_BRA3);

	if (node->token.word == "switch") for(;;)
	{

		if(parser.test(TOK_KEY,"case"))
		{
			DataPtrT<TNode_statement_switch::case_item> item(new TNode_statement_switch::case_item);
			DataPtrT<TNode_expression_list> elist(new TNode_expression_list);
			item->case_value = elist;

			for(;;)
			{
				DataPtrT<TNode_item> n=read_node_handler<TNode_item>::g(parser);
				elist->aList.append(n);
				//item->case_value.append(n);

				if(parser.test(TOK_COLON))
				{
					break;
				}
				if(parser.test(TOK_COMMA))
				{
					continue;
				}
				parser.kerror("invalid case");
				break;

			}
			//parser.match(TOK_COLON);
			item->case_stmt=read_node_handler<TNode_statement_list>::g(parser);
			node->case_list.append(item);

		}
		else if(parser.test(TOK_KEY,"default"))
		{
			DataPtrT<TNode_statement_switch::case_item> item(new TNode_statement_switch::case_item);
			parser.match(TOK_COLON);
			item->case_stmt=read_node_handler<TNode_statement_list>::g(parser);
			node->case_list.append(item);
		}
		else
		{
			break;
		}

	}
	else if (node->token.word == "judge") for (;;)
	{
		if (parser.test(TOK_KEY, "cond"))
		{
			DataPtrT<TNode_statement_switch::case_item> item(new TNode_statement_switch::case_item);
			item->case_value = read_node_handler<TNode_expression>::g(parser);
			parser.match(TOK_EQGT);
			item->case_stmt = read_node_handler<TNode_statement_list>::g(parser);
			node->case_list.append(item);
		}
		else if (parser.test(TOK_KEY, "default"))
		{
			DataPtrT<TNode_statement_switch::case_item> item(new TNode_statement_switch::case_item);
			parser.match(TOK_EQGT);
			item->case_stmt = read_node_handler<TNode_statement_list>::g(parser);
			node->case_list.append(item);
			break;
		}
		else
		{
			break;
		}
	}

	parser.match(TOK_KET3);
	return node;

}

template<>
DataPtrT<TNode_statement_try> read_node_handler<TNode_statement_try>::g(Parser& parser)
{
	DataPtrT<TNode_statement_try> node(new TNode_statement_try);
	node->token.word=parser.pcur[0].word;

	++parser.pcur;
	node->pBlock_try=read_node_handler<TNode_statement>::g(parser);
	parser.match(TOK_KEY,"catch");
	parser.match(TOK_BRA1);
	node->exp_list=read_node_handler<TNode_variable_list>::g(parser);
	parser.match(TOK_KET1);
	node->pBlock_catch=read_node_handler<TNode_statement>::g(parser);
	if(parser.test(TOK_KEY,"finally"))
	{
		node->pBlock_finally=read_node_handler<TNode_statement>::g(parser);
	}

	return node;
}

template<>
DataPtrT<TNode_statement_loop> read_node_handler<TNode_statement_loop>::g(Parser& parser)
{
	DataPtrT<TNode_statement_loop> node(new TNode_statement_loop);
	node->token.word=parser.pcur[0].word;

	if(node->token.word=="do")
	{
		++parser.pcur;
		node->stmt_body=read_node_handler<TNode_statement>::g(parser);
		parser.match(TOK_KEY,"while");
		parser.match(TOK_BRA1);
		if(parser.pcur[0].type!=TOK_KET1)
		{
			node->expr_cond=read_node_handler<TNode_expression>::g(parser);
		}
		parser.match(TOK_KET1);

		return node;
	}
	else if(node->token.word=="for_each")
	{
		++parser.pcur;
		parser.match(TOK_BRA1);
		node->fe_variable=read_node_handler<TNode_variable_list>::g(parser);
		parser.match(TOK_KEY,"in");
		node->fe_container=read_node_handler<TNode_expression>::g(parser);
		parser.match(TOK_KET1);
		node->stmt_body=read_node_handler<TNode_statement>::g(parser);
		return node;
	}

	++parser.pcur;
	parser.match(TOK_BRA1);

	if(node->token.word=="for")
	{
		node->stmt_init=(read_node_handler<TNode_statement>::g(parser));
		if(!parser.test(TOK_SEMICOLON))
		{
			node->expr_cond=read_node_handler<TNode_expression>::g(parser);
			parser.match(TOK_SEMICOLON);
		}
		node->stmt_fini=(read_node_handler<TNode_statement>::g(parser));
		node->stmt_fini->flags.add(TNode_statement::FLAG_WITH_SEMI2);
	}
	else if(parser.pcur[0].type!=TOK_KET1)
	{
		node->expr_cond=(read_node_handler<TNode_expression>::g(parser));
	}

	parser.match(TOK_KET1);
	node->stmt_body=read_node_handler<TNode_statement>::g(parser);

	return node;
}

template<>
DataPtrT<TNode_statement_control> read_node_handler<TNode_statement_control>::g(Parser& parser)
{
	DataPtrT<TNode_statement_control> node(new TNode_statement_control);
	node->token.word=parser.pcur[0].word;
	++parser.pcur;
	return node;
}

template<>
DataPtrT<TNode_statement_return> read_node_handler<TNode_statement_return>::g(Parser& parser)
{
	DataPtrT<TNode_statement_return> node(new TNode_statement_return);
	++parser.pcur;
	node->value=(read_node_handler<TNode_expression_list>::g(parser));
	return node;
}

template<>
DataPtrT<TNode_statement_throw> read_node_handler<TNode_statement_throw>::g(Parser& parser)
{
	DataPtrT<TNode_statement_throw> node(new TNode_statement_throw);
	++parser.pcur;
	node->value=(read_node_handler<TNode_expression_list>::g(parser));
	return node;
}

template<>
DataPtrT<TNode_statement_assignment> read_node_handler<TNode_statement_assignment>::g(Parser& parser)
{
	DataPtrT<TNode_statement_assignment> node(new TNode_statement_assignment);
	node->value=(read_expr_handler<0>::g(parser));
	return node;
}

void Parser::kerror(const String& s)
{

	if(pcur && pcur->line>0)
	{
		tokItem item(*pcur);
		Exception::XError(String::Format("Parser:%s at line:%d, pos:%d, token is %s.",s,item.line,item.cpos,item.word));
		return;
	}

	Exception::XError(String::Format("Parser:%s",s));
}

void Parser::kexpected(const String& s)
{
	kerror(s+" expected");
}

void Parser::match(tokType type)
{
	if(pcur[0].type==type)
	{
		++pcur;
	}
	else
	{
		kerror("unexpected token type");
	}
}

void Parser::match(tokType type,const String& s)
{
	if(pcur[0].type==type && pcur[0].word==s)
	{
		++pcur;
	}
	else
	{
		kerror("unexpected token type");
	}
}

bool Parser::test(tokType type)
{
	if(pcur[0].type==type)
	{
		++pcur;
		return true;
	}
	else
	{
		return false;
	}
}

bool Parser::test(tokType type,const String& s)
{
	if(pcur[0].type==type && pcur[0].word==s)
	{
		++pcur;
		return true;
	}
	else
	{
		return false;
	}
}

DataPtrT<TNode_statement_list> Parser::parse(const String& p)
{
	if(!scanner.parse(p))
	{
		return NULL;
	}
	return parse(scanner.aTokens.data());
}


DataPtrT<TNode_statement_list> Parser::parse(tokItem* t)
{
	pbeg=pcur=t;
	if(!pcur)
	{
		return NULL;
	}

	return read_node_handler<TNode_statement_list>::g(*this);
}


 String Parser::get_comment(int line)
{
	arr_1t<Scanner::CommentItem>::const_iterator it1,it2;
	int _lastline2=-1;

	for(it1=it2=scanner.aComments.begin();it2!=scanner.aComments.end();++it2)
	{
		const Scanner::CommentItem& item(*it2);
		if(item.line2>line)
		{
			break;
		}

		if(item.line2==line)
		{
			StringBuffer<char> result;
			while(it1<=it2) result << (*it1++).desc<<'\n';
			return result;
		}

		if((*it2).line1!=_lastline2)
		{
			_lastline2=(*it2).line2;
			it1=it2;
		}
	}

	return "";
}

EW_LEAVE
