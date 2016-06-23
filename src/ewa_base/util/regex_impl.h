

#ifndef __H_EW_UTIL_REGEX_IMPL__
#define __H_EW_UTIL_REGEX_IMPL__

#include "ewa_base/basic.h"
#include "ewa_base/util/regex.h"
#include "regex_parser.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE Match;

class match_group_data : public ObjectData
{
public:
	virtual String value()=0;
};

class match_group : public ObjectT<match_group_data>
{
public:
	
};

class match_result_data : public ObjectData
{
public:

	virtual size_t size() const =0;
	virtual match_group group(size_t i) const=0;


};

class match_result : public ObjectT<match_result_data>
{
public:
	
};


template<typename X>
class regex_pos_and_num_t
{
public:
	X pos;
	int num;
	regex_pos_and_num_t():pos(),num(0){}
	regex_pos_and_num_t(X it,int n):pos(it),num(n){}
};

template<typename X>
class match_result_data_t : public match_result_data
{
public:
	typename arr_1t<regex_pos_and_num_t<X> >::iterator it_beg,it_end;
};


template<typename X>
class regex_policy_base
{
public:
	typedef X iterator;

	iterator it_beg,it_end,it_cur;
	typedef regex_pos_and_num_t<X> regex_iterator_and_num;

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

	static bool handle_item_id(regex_state&,const String&){return false;}

	static void seqenter(regex_state&){}
	static void seqleave(regex_state&){}
	static void fallback(regex_state&){}


};

class regex_policy_char_pointer : public regex_policy_base<const char*>
{
public:

};

class regex_policy_char_match : public regex_policy_char_pointer
{
public:

	arr_1t<regex_iterator_and_num> stkSeqpos;

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

	void update_match(Match& res);
};

class regex_policy_char_recursive : public regex_policy_char_match
{
public:
	typedef regex_policy_char_match basetype;

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

	bst_map<String,DataPtrT<ObjectData> >* p_item_map;

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
		bst_map<String,DataPtrT<ObjectData> >::iterator it=p_item_map->find(name);
		if(it==p_item_map->end()) return false;

		if(state.curp->sibling)
		{
			state.n_pos_curstk++;
			curp_stack.push_back(curp_state(state.curp->sibling,state.n_seq_index+1));
		}

		state.curp=static_cast<regex_item_root*>((*it).second.get());
		return true;
	}

};

template<typename P>
class regex_impl : public P
{
public:

	typedef P policy;
	typedef typename P::iterator iterator;
	typedef typename P::regex_state regex_state;
	typedef typename P::regex_iterator_and_num regex_iterator_and_num;


	bool execute(regex_item_root* seq,iterator t1,iterator t2,bool match_all);

private:

	arr_1t<regex_state> arrStates;
	arr_1t<regex_iterator_and_num> stkRepeat;

	bool fallback(regex_state& state);
	bool match_real(iterator& it,regex_item* sq);

};

EW_LEAVE
#endif
