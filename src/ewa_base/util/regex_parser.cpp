#include "regex_parser.h"
#include "ewa_base/util/regex.h"

EW_ENTER

regex_item::~regex_item()
{
	while(next)
	{
		regex_item* old=next;
		next=next->next;
		old->next=NULL;
		delete old;
	}
}


void regex_item_char_map::update_case()
{
	regex_item_char_map* q=this;
	for(char ch='a';ch<='z';ch++)
	{
		if(q->get(ch))
		{
			q->set(ch-'a'+'A',true);
		}
		else if(q->get(ch-'a'+'A'))
		{
			q->set(ch,true);
		}
	}
}

void regex_item_char_map::add_chars(char ch)
{
	if(ch=='w')
	{
		for(char ch='a';ch<='z';ch++)
		{
			set(ch,true);
			set(ch-'a'+'A',true);
		}
	}
	else if(ch=='d')
	{
		for(char ch='0';ch<='9';ch++)
		{
			set(ch,true);
		}
	}
	else if(ch=='s')
	{
		set(' ',true);
		set('\t',true);
		set('\r',true);
		set('\n',true);
		set('\f',true);
		set('\v',true);
	}
	else if(ch=='S'||ch=='D'||ch=='W')
	{
		regex_item_char_map t;
		t.add_chars(ch-'A'+'a');
		for(int i=0;i<ndig;i++)
		{
			bitmap[i]|=~t.bitmap[i];
		}
	}
}


void regex_item_repeat::update(regex_item* q)
{
	real_sibling=next?next:q;
	sibling=&repeat_end;
	repeat_end.sibling=child.get();
}

void regex_item_repeat::update(regex_item_state& q)
{
	index=q.loop++;

	real_sibling=next?next:q.sibling;

	if(!child.get()) return;

	LockState<regex_item*> lock(q.sibling,&repeat_end);
	child->update(q);

	sibling=&repeat_end;
	repeat_end.sibling=child.get();

}

void regex_item_seq::update(regex_item_state& q)
{

	index=q.iseq++;
	seqend.sibling=next?next:q.sibling;

	LockState<regex_item*> lock(q.sibling,&seqend);
	for(regex_item* p=child.get();p;p=p->next)
	{
		p->update(q);
	}

	sibling=child.get();

}

void regex_item_seq::append(regex_item* p)
{
	if(!child.get())
	{
		child.reset(p);
		return;
	}
	regex_item* h=child.get();
	while(h && h->next) h=h->next;
	h->next=p;
	return;
}

void regex_item_seq::adjust()
{
	regex_item* p1=child.get();
	if(!p1) return;

	regex_item* p2=p1->next;
	while(p2)
	{
		if(p1->type==regex_item::ITEM_REPEAT&&p2->type==regex_item::ITEM_REPEAT)
		{
			regex_item_repeat* q1=static_cast<regex_item_repeat*>(p1);
			regex_item_repeat* q2=static_cast<regex_item_repeat*>(p2);
			if(q1->child->next==NULL && q2->child->next==NULL && q1->child->type==q2->child->type && q1->child->type==regex_item::ITEM_CHAR_STR)
			{
				if(static_cast<regex_item_char_str*>(q1->child.get())->value==static_cast<regex_item_char_str*>(q2->child.get())->value)
				{
					q1->nmin+=q2->nmin;
					if(q1->nmax>0 && q2->nmax>0)
					{
						q1->nmax+=q2->nmax;
					}
					else
					{
						q1->nmax=-1;
					}

					p1->next=p2->next;
					p2->next=NULL;
					delete p2;
					p2=p1->next;
					continue;
				}
			}
		}

		p1=p2;
		p2=p2->next;
	}

}


int RegexParser::read_number(const char* &p1)
{
	int n=0;
	while(*p1>='0'&&*p1<='9') n=n*10+(*p1++ - '0');
	return n;
}

regex_item_char_map* RegexParser::parse_char_map(const char* &p1)
{
	if(*p1!='[') return NULL;

	AutoPtrT<regex_item_char_map> q(new regex_item_char_map);
	p1++;

	bool inv=false;
	if(*p1=='^')
	{
		inv=true;
		++p1;
	}

	while(*p1!=']')
	{
		if(*p1=='\\')
		{
			if(p1[1]=='r'||p1[1]=='n'||p1[1]=='t'||p1[1]=='v'||p1[1]=='f')
			{
				q->set(lookup_table<lkt_slash>::test(p1[1]),true);
			}
			else if(p1[1]=='w'||p1[1]=='d'||p1[1]=='s'||p1[1]=='W'||p1[1]=='D'||p1[1]=='S')
			{
				q->add_chars(p1[1]);
				if(flags.get(Regex::FLAG_RE_UNICODE) && p1[1]=='w')
				{
					q.reset(new regex_item_char_map_unicode(*q));
				}
			}
			else
			{
				q->set(p1[1],true);
			}

			p1+=2;
		}
		else if(p1[1]=='-')
		{
			char c1=p1[0];
			char c2=p1[2];
			if(c2==']') return NULL;
			for(char ch=c1;ch<=c2;ch++) q->set(ch,true);
			p1+=3;

		}
		else
		{
			q->set(*p1++,true);
		}
	}

	if(*p1++!=']')
	{
		return NULL;
	}

	if(flags.get(Regex::FLAG_RE_IGNORECASE))
	{
		q->update_case();
	}

	if(inv) q->inv();

	return q.release();;

}

regex_item* RegexParser::parse_item(const char* &p1,bool seq)
{
	const char* p0=p1;

	if(*p1=='(')
	{
		AutoPtrT<regex_item_seq> q(parse_seq(++p1));
		if(*p1++!=')')
		{
			return NULL;
		}
		return q.release();;
	}

	if(*p1=='[')
	{
		return parse_char_map(p1);
	}

	if(*p1=='^'||*p1=='$')
	{
		p1++;
		if(flags.get(Regex::FLAG_RE_MULTILINE))
		{
			return new regex_item(*p1=='^'?regex_item::ITEM_LINE_ENTER:regex_item::ITEM_LINE_LEAVE);
		}
		else
		{
			return new regex_item(*p1=='^'?regex_item::ITEM_EXPR_ENTER:regex_item::ITEM_EXPR_LEAVE);
		}
	}

	if(*p1=='\\')
	{
		p1++;

		char ch=*p1++;

		if(ch=='w'||ch=='d'||ch=='s'||ch=='W'||ch=='D'||ch=='S')
		{
			AutoPtrT<regex_item_char_map> q(new regex_item_char_map);

			q->add_chars(ch);
			if(flags.get(Regex::FLAG_RE_IGNORECASE))
			{
				q->update_case();
			}
			if(flags.get(Regex::FLAG_RE_UNICODE) && ch=='w')
			{
				q.reset(new regex_item_char_map_unicode(*q));
			}
			return q.release();
		}
		else if(ch=='t'||ch=='r'||ch=='n'||ch=='v'||ch=='f')
		{
			return new regex_item_char_str(lookup_table<lkt_slash>::test(ch),flags.get(Regex::FLAG_RE_IGNORECASE));
		}
		else
		{
			return new regex_item_char_str(ch,flags.get(Regex::FLAG_RE_IGNORECASE));
		}

		return NULL;
	}


	if(*p1=='.')
	{
		p1++;
		if(flags.get(Regex::FLAG_RE_DOTALL))
		{
			return new regex_item(regex_item::ITEM_CHAR_ANY);
		}
		else
		{
			AutoPtrT<regex_item_char_map> q(flags.get(Regex::FLAG_RE_UNICODE)?new regex_item_char_map_unicode:new regex_item_char_map);
			q->set('\r',true);
			q->set('\n',true);
			q->regex_item_char_map::inv();
			return q.release();
		}
	}

	if(*p1=='`')
	{
		for(p1++;*p1&&*p1!='`';p1++);
		if(*p1++!='`') return NULL;
		return new regex_item_id(String(p0+1,p1-1));
	}

	if(!seq)
	{
		return new regex_item_char_str(String(p0,++p1),flags.get(Regex::FLAG_RE_IGNORECASE));
	}
	const char* prev=p1;
	while(1)
	{

		if((*p1>='a'&&*p1<='z')||(*p1>='A'&&*p1<='Z')||(*p1>='0'&&*p1<='9')||*p1==' ')
		{
			prev=p1++;
			continue;
		}

		unsigned uc=(unsigned char)*p1;

		if(uc<0xC0)
		{
			break;
		}

		prev=p1;
		if(uc<0xE0)
		{
			p1+=2;
		}
		else if(uc<0xF0)
		{
			p1+=3;
		}
		else
		{
			p1+=4;
		}

	}

	if(p1==p0) return NULL;

	if(p1[0]!='*'&&p1[0]!='+'&&p1[0]!='?'&&p1[0]!='{'&&p1[0]!='|')
	{
		return new regex_item_char_str(String(p0,p1),flags.get(Regex::FLAG_RE_IGNORECASE));
	}
	else if(prev>p0)
	{
		p1=prev;
		return new regex_item_char_str(String(p0,prev),flags.get(Regex::FLAG_RE_IGNORECASE));
	}
	else
	{
		return new regex_item_char_str(String(p0,p1),flags.get(Regex::FLAG_RE_IGNORECASE));
	}
}

regex_item* RegexParser::parse_item_ex(const char* &p1,bool seq)
{
	AutoPtrT<regex_item> q(parse_item(p1,seq));
	if(!q.get()) return NULL;

	regex_item_repeat* q1=NULL;
	for(;*p1=='*'||*p1=='+'||*p1=='?'||*p1=='{';p1++)
	{
		AutoPtrT<regex_item_repeat> q2(new regex_item_repeat);
		if(*p1=='*')
		{

		}
		else if(*p1=='+')
		{
			q2->nmin=1;
		}
		else if(*p1=='?')
		{
			q2->nmin=0;
			q2->nmax=1;
		}
		else if(*p1=='{')
		{
			q2->nmin=q2->nmax=read_number(++p1);
			if(*p1==',')
			{
				q2->nmax=read_number(++p1);
			}
			if(*p1!='}')
			{
				return NULL;
			}
		}
		else
		{
			break;
		}

		if(p1[1]=='?')
		{
			p1++;
			q2->repeat_end.match_as_much_as_possible=false;
		}

		if(q1 && q1->repeat_end.match_as_much_as_possible==q2->repeat_end.match_as_much_as_possible)
		{
			if(q1->nmax==-1||q1->nmax==1)
			{
				q1->nmin*=q2->nmin;
				q1->nmax=(q1->nmax==-1||q2->nmax==-1)?-1:q2->nmax;
				continue;
			}
			else if(q2->nmax==1)
			{
				q1->nmin*=q2->nmin;
				continue;
			}
		}


		q2->child.reset(q.release());
		q1=q2.get();
		q.reset(q2.release());

	}

	return q.release();
}

regex_item_seq* RegexParser::parse_seq(const char* &p1)
{
	arr_1t<regex_item*> items;

	while(*p1&&*p1!=')')
	{
		AutoPtrT<regex_item> q(parse_item_ex(p1,true));
		if(!q.get()) return NULL;

		while(p1[0]=='|')
		{
			AutoPtrT<regex_item_or> q2(new regex_item_or);
			q2->child1.reset(q.release());
			q2->child2.reset(parse_item_ex(++p1,false));



			if(!q2->child2.get())
			{
				return NULL;
			}

			q.reset(q2.release());


		}

		items.push_back(q.release());
	}

	regex_item_seq* pitem(new regex_item_seq);
	for(size_t i=1;i<items.size();i++) items[i-1]->next=items[i];
	if(!items.empty()) pitem->child.reset(items[0]);

	pitem->adjust();
	return pitem;
}

regex_item_root* RegexParser::parse(const String& s)
{
	AutoPtrT<regex_item_root> q(new regex_item_root);
	q->orig_str=s;
	q->flags=flags;

	const char* p=q->orig_str.c_str();

	AutoPtrT<regex_item_seq> seq(parse_seq(p));
	if(!seq||*p!=0) return NULL;

	q->child.swap(seq->child);
	regex_item_state rstate;
	q->update(rstate);

	return q.release();
}


EW_LEAVE
