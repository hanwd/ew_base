

#ifndef __H_EW_UTIL_REGEX_IMPL__
#define __H_EW_UTIL_REGEX_IMPL__

#include "ewa_base/basic.h"
#include "regex_parser.h"

EW_ENTER


template<typename X>
class regex_base
{
public:
	typedef X iterator;

	class regex_state
	{
	public:		
		regex_item* curp;
		iterator ipos;
		int iseq;

		int n_pos_repeat;
		int n_pos_seqpos;
		regex_state():n_pos_repeat(0),n_pos_seqpos(0),curp(NULL),iseq(0){}
	};

	std::vector<regex_state> arrStates;

	class repeat_pos_and_num
	{
	public:
		iterator pos;
		int num;
		repeat_pos_and_num():pos(),num(0){}
		repeat_pos_and_num(iterator it,int n):pos(it),num(n){}
	};

	std::vector<repeat_pos_and_num> stkRepeat;
	std::vector<repeat_pos_and_num> stkSeqpos;

	bool fallback(regex_state& state);

	bool match_real(iterator& it,regex_item* sq);

	bool search(regex_item_seq* seq,iterator t1,iterator t2);
	bool match(regex_item_seq* seq,iterator t1,iterator t2);

	iterator it_beg,it_end,it_cur;
};

EW_LEAVE
#endif
