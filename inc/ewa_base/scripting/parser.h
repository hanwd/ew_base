#ifndef __H_EW_SCRIPTING_PARSER__
#define __H_EW_SCRIPTING_PARSER__


#include "ewa_base/scripting/scanner.h"


EW_ENTER


class DLLIMPEXP_EWA_BASE TNode_statement_list;
class DLLIMPEXP_EWA_BASE tokItem;

template<typename T>
class read_node_handler;

template<int N>
class read_expr_handler;

class DLLIMPEXP_EWA_BASE Parser
{
public:

	DataPtrT<TNode_statement_list> parse(const String& p);
	DataPtrT<TNode_statement_list> parse(tokItem* t);

	String get_comment(int line);

	Parser();

	indexer_map<String,int>& op_table;

protected:

	template<typename T>
	friend class read_node_handler;

	template<int N>
	friend class read_expr_handler;

	void match(tokType type);
	void match(tokType type,const String& s);
	bool test(tokType type);
	bool test(tokType type,const String& s);

	void kerror(const String& s);
	void kexpected(const String& s);

	tokItem* pcur;
	tokItem* pbeg;

	BitFlags flags;

	Scanner scanner;
};

EW_LEAVE

#endif


