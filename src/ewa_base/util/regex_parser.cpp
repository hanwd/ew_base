#include "regex_parser.h"
#include "ewa_base/util/Regex.h"

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
				if(p1[1]=='r'||p1[1]=='n'||p1[1]=='t')
				{
					q->set(lookup_table<lkt_slash>::test(p1[1]),true);				
				}
				else if(p1[1]=='w'||p1[1]=='d'||p1[1]=='s')
				{
					q->add_chars(p1[1]);
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

		if(flags.get(Regex::FLAG_CASE_INSENSITIVE))
		{
			q->update_case();
		}

		if(inv) q->inv();

		return q.release();;
	}

	if(*p1=='^'||*p1=='$')
	{
		return new regex_item_line(*p1++);
	}
		
	if(*p1=='\\')
	{
		p1++;

		char ch=*p1++;

		if(ch=='w'||ch=='d'||ch=='s')
		{
			regex_item_char_map*q= new regex_item_char_map;	
			q->add_chars(ch);
			if(flags.get(Regex::FLAG_CASE_INSENSITIVE))
			{
				q->update_case();
			}
			return q;			
		}
		else if(ch=='t'||ch=='r'||ch=='n')
		{
			return new regex_item_char_str(lookup_table<lkt_slash>::test(ch),flags.get(Regex::FLAG_CASE_INSENSITIVE));		
		}
		else
		{
			return new regex_item_char_str(ch,flags.get(Regex::FLAG_CASE_INSENSITIVE));
		}

		return NULL;
	}


	if(*p1=='.')
	{
		p1++;
		return new regex_item_char_any();
	}

	if(!seq)
	{
		return new regex_item_char_str(String(p0,++p1),flags.get(Regex::FLAG_CASE_INSENSITIVE));
	}

	while(*p1>='a'&&*p1<='z'||*p1>='A'&&*p1<='Z'||*p1>='0'&&*p1<='9'||*p1==' ') p1++;
	if(p1==p0) return NULL;

	if(p1[0]!='*'&&p1[0]!='+'&&p1[0]!='?'&&p1[0]!='{'&&p1[0]!='|')
	{
		return new regex_item_char_str(String(p0,p1),flags.get(Regex::FLAG_CASE_INSENSITIVE));
	}
	else if(p1-p0>1)
	{
		return new regex_item_char_str(String(p0,--p1),flags.get(Regex::FLAG_CASE_INSENSITIVE));
	}
	else
	{
		return new regex_item_char_str(String(p0,p1),flags.get(Regex::FLAG_CASE_INSENSITIVE));	
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
	std::vector<regex_item*> items;

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

regex_item_seq* RegexParser::parse(const String& s)
{
	const char* p=s.c_str();
		
	regex_item_seq* q=parse_seq(p);
	if(q)
	{
		regex_item_state s;
		q->update(s);
	}
	return q;
}


EW_LEAVE
