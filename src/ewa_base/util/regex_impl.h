

#ifndef __H_EW_UTIL_REGEX_IMPL__
#define __H_EW_UTIL_REGEX_IMPL__

#include "ewa_base/basic.h"
#include "ewa_base/util/regex.h"
#include "regex_parser.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE Match;

class regex_policy_char
{
public:
	typedef const char* iterator;

	iterator it_beg,it_end,it_cur;

	BitFlags flags;

	class regex_state
	{
	public:

		regex_state():n_pos_repeat(0),n_pos_seqpos(0),curp(NULL){}

		regex_item* curp;
		iterator ipos;
		int n_pos_repeat;
		int n_pos_seqpos;
	};

	class regex_iterator_and_num
	{
	public:
		iterator pos;
		int num;
		regex_iterator_and_num():pos(),num(0){}
		regex_iterator_and_num(iterator it,int n):pos(it),num(n){}
	};

	bool not_finished(regex_state& state)
	{
		if(state.curp!=NULL) return true;	
		if(state.ipos!=it_end && !flags.get(Regex::FLAG_RE_PARTITIAL))
		{
			static regex_item item(regex_item::ITEM_TRY_FALLBACK);
			state.curp=&item;
			return true;
		}

		return false;
	}

	static void init_state(regex_state& state,iterator& it,regex_item* sq)
	{
		state.curp=sq;
		state.ipos=it;
		state.n_pos_repeat=0;
		state.n_pos_seqpos=0;
	}

	void seqenter(regex_state& state)
	{
		state.n_pos_seqpos++;
		stkSeqpos.push_back(regex_iterator_and_num(state.ipos, static_cast<regex_item_seq*>(state.curp)->index));	
	}

	void seqleave(regex_state& state)
	{
		state.n_pos_seqpos++;
		stkSeqpos.push_back(regex_iterator_and_num(state.ipos,-1));		
	}

	void fallback(regex_state& state)
	{
		stkSeqpos.resize(state.n_pos_seqpos);
	}

	static bool handle_item_id(regex_state&,const String&){return false;}


	arr_1t<regex_iterator_and_num> stkSeqpos;
};

class regex_policy_char_match_only : public regex_policy_char
{
public:
	static void seqenter(regex_state&){}
	static void seqleave(regex_state&){}
	static void fallback(regex_state&){}
};

class regex_policy_char_recursive : public regex_policy_char
{
public:
	typedef regex_policy_char basetype;

	class regex_state : public basetype::regex_state
	{
	public:

		regex_state():n_pos_curstk(0),n_seq_index(0){}
		int n_pos_curstk;
		int n_seq_index;
	};

	class curp_state
	{
	public:
		curp_state(regex_item* p=0,int n=0):curp(p),n_seq_shift(n){}
		regex_item* curp;
		int n_seq_shift;
	};

	arr_1t<curp_state> curp_stack;

	bst_map<String,regex_item*> item_map;

	bool not_finished(regex_state& state)
	{
		if(state.curp!=NULL) return true;
		state.curp=curp_stack.back().curp;
		if(!state.curp)
		{
			if(state.ipos!=it_end && !flags.get(Regex::FLAG_RE_PARTITIAL))
			{
				static regex_item item(regex_item::ITEM_TRY_FALLBACK);
				state.curp=&item;
				return true;
			}
			return false;
		}

		state.n_seq_index=curp_stack.back().n_seq_shift-1;
		state.n_pos_curstk--;
		curp_stack.pop_back();
		return true;
	}

	void seqenter(regex_state& state)
	{
		state.n_seq_index=curp_stack.back().n_seq_shift+static_cast<regex_item_seq*>(state.curp)->index;

		state.n_pos_seqpos++;
		stkSeqpos.push_back(regex_iterator_and_num(state.ipos, state.n_seq_index));	
	}

	void init_state(regex_state& state,iterator& it,regex_item* sq)
	{
		basetype::init_state(state,it,sq);
		curp_stack.resize(1);
		state.n_pos_curstk=1;
	}

	void fallback(regex_state& state)
	{
		basetype::fallback(state);
		curp_stack.resize(state.n_pos_curstk);
	}

	bool handle_item_id(regex_state& state,const String& name)
	{
		bst_map<String,regex_item*>::iterator it=item_map.find(name);
		if(it==item_map.end()) return false;

		if(state.curp->sibling)
		{
			state.n_pos_curstk++;
			curp_stack.push_back(curp_state(state.curp->sibling,state.n_seq_index+1));
		}

		state.curp=(*it).second;
		return true;
	}

};

template<typename X>
class regex_impl : public X
{
public:

	typedef typename X::iterator iterator;
	typedef typename X::regex_state regex_state;
	typedef typename X::regex_iterator_and_num regex_iterator_and_num;


	arr_1t<regex_state> arrStates;
	arr_1t<regex_iterator_and_num> stkRepeat;

	void update_match_results(Match& res);

	bool fallback(regex_state& state);

	bool match_real(iterator& it,regex_item* sq);

	bool execute(regex_item_root* seq,iterator t1,iterator t2,bool match_all);
};

EW_LEAVE
#endif
