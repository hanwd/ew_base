#include "regex_impl.h"
#include "ewa_base/util/regex.h"
EW_ENTER

template<typename X>
bool regex_impl<X>::search(regex_item_root* seq,iterator t1,iterator t2)
{
	if(!seq) return false;

	it_beg=it_cur=t1;
	it_end=t2;

	regex_item_repeat repeat;

	repeat.child.reset(new regex_item(regex_item::ITEM_CHAR_ANY));
	repeat.repeat_end.match_as_much_as_possible=false;
	repeat.child->sibling=&repeat.repeat_end;
	repeat.update(seq);

	if(!match_real(it_cur,&repeat))
	{
		return false;
	}
	return true;
}

template<typename X>
bool regex_impl<X>::match(regex_item_root* seq,iterator t1,iterator t2)
{
	if(!seq) return false;

	it_beg=it_cur=t1;
	it_end=t2;

	if(!match_real(it_cur,seq))
	{
		return false;
	}

	return seq->flags.get(Regex::FLAG_RE_PARTITIAL)||it_cur==it_end;
}

template<typename X>
bool regex_impl<X>::fallback(regex_state& state)
{
	::printf("fallback\n");

	if(arrStates.empty())
	{
		return false;
	}

	state=arrStates.back();
	stkRepeat.resize(state.n_pos_repeat);

	X::fallback(state);

	arrStates.pop_back();
	return true;
}

template<typename X>
bool regex_impl<X>::match_real(iterator& it,regex_item* sq)
{

	arrStates.clear();
	stkRepeat.clear();

	if(X::flag_store_result)
	{
		stkSeqpos.clear();
	}

	regex_state state;

	X::init_state(state,it,sq);
	
	while(X::not_finished(state))
	{
		switch(state.curp->type)
		{
		case regex_item::ITEM_CHAR_STR_IGNORECASE:
			{
				typedef lookup_table<lkt2lowercase> lk;

				regex_item_char_str& item(*static_cast<regex_item_char_str*>(state.curp));
				const char* p1=item.value.c_str();

				while(state.ipos !=it_end && *p1 && lk::cmap[unsigned char(*state.ipos)]==lk::cmap[unsigned char(*p1)])
				{
					state.ipos++;p1++;
				}

				if(*p1)
				{
					if(!fallback(state))
					{
						return false;
					}
					continue;
				}
			}
			break;
		case regex_item::ITEM_CHAR_STR:
			{
				regex_item_char_str& item(*static_cast<regex_item_char_str*>(state.curp));
				const char* p1=item.value.c_str();

				while(state.ipos !=it_end && *p1 && *state.ipos==*p1)
				{
					state.ipos++;p1++;
				}

				if(*p1)
				{
					if(!fallback(state))
					{
						return false;
					}
					continue;
				}
			}
			break;
		case regex_item::ITEM_CHAR_ANY:
			{
				regex_item& item(*static_cast<regex_item*>(state.curp));
				if(state.ipos!=it_end)
				{
					++state.ipos;						
				}
				else
				{
					if(!fallback(state))
					{
						return false;
					}
					continue;
				}
			}
			break;
		case regex_item::ITEM_CHAR_MAP_UNICODE:
			{
				regex_item_char_map& item(*static_cast<regex_item_char_map*>(state.curp));

				unsigned char uc=*state.ipos;

				if(uc>=0xC0)
				{
					if(uc<0xE0)
					{
						state.ipos+=2;
						if(state.ipos>it_end){state.ipos=it_end;}
					}
					else if(uc<0xF0)
					{
						state.ipos+=3;
						if(state.ipos>it_end){state.ipos=it_end;}
					}
					else
					{
						state.ipos+=4;
						if(state.ipos>it_end){state.ipos=it_end;}
					}
				}
				else if(item.get(*state.ipos))
				{
					++state.ipos;
				}
				else
				{
					if(!fallback(state))
					{
						return false;
					}
					continue;
				}
			}
			break;
		case regex_item::ITEM_CHAR_MAP:
			{
				regex_item_char_map& item(*static_cast<regex_item_char_map*>(state.curp));
				if(item.get(*state.ipos))
				{
					++state.ipos;
				}
				else
				{
					if(!fallback(state))
					{
						return false;
					}
					continue;
				}
			}
			break;
		case regex_item::ITEM_REPEAT_NEXT:
			{
				::printf("repeat_next\n");

				regex_item_repeat_next& item(*static_cast<regex_item_repeat_next*>(state.curp));

				int n=stkRepeat.back().num++;
				if(n<item.node.nmin)
				{
						
				}
				else if(n==item.node.nmax||state.ipos==it_end)
				{							
					state.curp=item.node.real_sibling;
					continue;
				}
				else if(item.match_as_much_as_possible)
				{
					if(n==item.node.nmin||state.ipos!=stkRepeat.back().pos)
					{
						regex_state state2(state);
						state2.curp=item.node.real_sibling;
						state2.n_pos_repeat=state.n_pos_repeat-1;
						arrStates.push_back(state2);
					}
					else
					{
						if(!fallback(state))
						{
							return false;
						}
						continue;
					}						
				}
				else
				{
					if(n==item.node.nmin||state.ipos!=stkRepeat.back().pos)
					{
						regex_state state2(state);
						state2.curp=state.curp->sibling;
						arrStates.push_back(state2);
						state.curp=item.node.real_sibling;
						continue;
					}
					else
					{
						if(!fallback(state))
						{
							return false;
						}
						continue;
					}				
				}

				stkRepeat.back().pos=state.ipos;
			}
			break;
		case regex_item::ITEM_REPEAT:
			{
				::printf("repeat\n");

				stkRepeat.push_back(regex_iterator_and_num(state.ipos,0));
				state.n_pos_repeat=stkRepeat.size();
			}
			break;
		case regex_item::ITEM_OR:
			{
				regex_state state2(state);
				state2.curp=static_cast<regex_item_or*>(state.curp)->child2.get();
				arrStates.push_back(state2);		
			}
			break;
		case regex_item::ITEM_EXPR_ENTER:
			{
				if(state.ipos!=it_beg)
				{
					if(!fallback(state))
					{
						return false;
					}
					continue;
				}
			}
			break;
		case regex_item::ITEM_EXPR_LEAVE:
			{
				if(state.ipos!=it_end)
				{
					if(!fallback(state))
					{
						return false;
					}
					continue;
				}
			}
			break;
		case regex_item::ITEM_LINE_ENTER:
			{
				iterator ipos(state.ipos);
				if(ipos!=it_beg&&*--ipos!='\n')
				{
					if(!fallback(state))
					{
						return false;
					}
					continue;
				}

			}
			break;
		case regex_item::ITEM_LINE_LEAVE:
			{
				iterator ipos(state.ipos);
				for(;ipos!=it_end && *ipos=='\r';ipos++);

				if(ipos!=it_end && *ipos++!='\n')
				{
					if(!fallback(state))
					{
						return false;
					}
					continue;
				}
				state.ipos=ipos;
			}
				
			break;
		case regex_item::ITEM_SEQ:
			{
				X::seqenter(state);
			}
			break;
		case regex_item::ITEM_SEQ_END:
			{
				X::seqleave(state);
			}
			break;
		case regex_item::ITEM_ID:
			{
				if(!X::handle_item_id(state,static_cast<regex_item_id*>(state.curp)->value))
				{
					return false;
				}
				continue;
			}
		default:
			return false;				
		}

		state.curp=state.curp->sibling;
	};

	it=state.ipos;
	return true;
		
}



template<typename X>
void regex_impl<X>::update_match_results(Match& res)
{

	class item
	{
	public:

		item(){}
		item(const char* p,int n):str(p,p),num(n){}

		Match::item str;
		int num;
	};

	 arr_1t<item> q;

	for(size_t i=0;i<stkSeqpos.size();i++)
	{
		int n=stkSeqpos[i].num;
		if(n>=0)
		{
			q.push_back(item(stkSeqpos[i].pos,stkSeqpos[i].num));
		}
		else if(n<0)
		{
			q.back().str.it_end=stkSeqpos[i].pos;
			if(q.back().num>=(int)res.matchs.size())
			{
				res.matchs.resize(q.back().num+1);
			}
			res.matchs[q.back().num].push_back(q.back().str);
			q.pop_back();
		}
	}
}

template class regex_impl<regex_policy_char>;
template class regex_impl<regex_policy_char_match_only>;
template class regex_impl<regex_policy_char_recursive>;

EW_LEAVE
