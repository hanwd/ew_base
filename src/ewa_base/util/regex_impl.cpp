#include "regex_impl.h"
#include "ewa_base/util/regex.h"
EW_ENTER

template<typename P>
bool regex_impl<P>::execute(regex_item_root* seq,iterator t1,iterator t2,bool match_mode)
{
	if(!seq) return false;

	flags=seq->flags;

	it_beg=it_cur=t1;
	it_end=t2;

	if(match_mode)
	{
		if(!match_real(it_cur,seq))
		{
			return false;
		}
		return seq->flags.get(Regex::FLAG_RE_PARTITIAL)||it_cur==it_end;
	}
	else
	{
		flags.add(Regex::FLAG_RE_PARTITIAL);
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

}


template<typename P>
bool regex_impl<P>::fallback(regex_state& state)
{

	state=arrStates.back();
	if(!state.curp) return false;
	arrStates.pop_back();

	stkRepeat.resize(state.n_pos_repeat);
	P::fallback(state);
	return true;
}

template<typename P>
bool regex_impl<P>::match_real(iterator& it,regex_item* sq)
{

	regex_state state;
	P::init_state(state,it,sq);

	arrStates.resize(1);

	while(P::not_finished(state))
	{
		switch(state.curp->type)
		{
		case regex_item::ITEM_CHAR_STR_IGNORECASE:
			{
				typedef lookup_table<lkt2lowercase> lk;

				regex_item_char_str& item(*static_cast<regex_item_char_str*>(state.curp));
				const char* p1=item.value.c_str();

				while(state.ipos !=it_end && *p1 && lk::cmap[(unsigned char)(*state.ipos)]==lk::cmap[(unsigned char)(*p1)])
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
				P::seqenter(state);
			}
			break;
		case regex_item::ITEM_SEQ_END:
			{
				P::seqleave(state);
			}
			break;
		case regex_item::ITEM_ID:
			{
				if(!P::handle_item_id(state,static_cast<regex_item_id*>(state.curp)->value))
				{
					return false;
				}
				continue;
			}
		case regex_item::ITEM_TRY_FALLBACK:
			{
				if(state.n_pos_repeat<arrStates.back().n_pos_repeat)
				{
					return false;
				}
				if(!fallback(state)) return false;
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



void regex_policy_char_match::update_match(arr_1t<Match::item_array>& res)
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
			if(q.back().num>=(int)res.size())
			{
				res.resize(q.back().num+1);
			}
			res[q.back().num].push_back(q.back().str);
			q.pop_back();
		}
	}
}

template class regex_impl<regex_policy_char_pointer>;
template class regex_impl<regex_policy_char_match>;
template class regex_impl<regex_policy_char_recursive>;

EW_LEAVE
