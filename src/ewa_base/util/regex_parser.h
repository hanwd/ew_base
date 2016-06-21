
#ifndef __H_EW_UTIL_REGEX_PARSER__
#define __H_EW_UTIL_REGEX_PARSER__

#include "ewa_base/basic.h"

EW_ENTER

class regex_item;
class regex_item_state
{
public:
	regex_item_state():sibling(0),iseq(0),loop(0){}
	regex_item* sibling;
	int iseq;
	int loop;
};

class regex_item
{
public:
	enum
	{
		ITEM_NONE,
		ITEM_SEQ,
		ITEM_SEQ_END,
		ITEM_CHAR_STR,
		ITEM_CHAR_STR_NOCASE,
		ITEM_CHAR_ANY,
		ITEM_CHAR_MAP,
		ITEM_REPEAT,
		ITEM_REPEAT_NEXT,
		ITEM_LINE,
		ITEM_OR,
	};

	regex_item(int t):type(t),next(NULL),sibling(NULL)
	{

	}

	virtual ~regex_item();

	virtual void update(regex_item_state& q)
	{
		sibling=next?next:q.sibling;
	}

	virtual void print(int n)
	{
		for(int i=0;i<n;i++) ::printf(" ");
	}

	const int type;
	regex_item* next;

	regex_item* sibling;
};

class regex_item_line : public regex_item
{
public:
	char value;
	regex_item_line(char ch):regex_item(ITEM_LINE),value(ch){}
	virtual void print(int n)
	{
		regex_item::print(n);
		::printf("line %c\n",value);
	}
};

class regex_item_char_any : public regex_item
{
public:
	regex_item_char_any():regex_item(ITEM_CHAR_ANY){}
	virtual void print(int n)
	{
		regex_item::print(n);
		::printf("any\n");
	}
};

class regex_item_char_map : public regex_item
{
public:
	regex_item_char_map():regex_item(ITEM_CHAR_MAP)
	{
		memset(bitmap,0,sizeof(bitmap));
	}

	static const int ndig=256/32;
	unsigned bitmap[ndig];

	void update_case()
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

	void add_chars(char ch)
	{
		regex_item_char_map* q=this;
		if(ch=='w')
		{
			for(char ch='a';ch<='z';ch++)
			{
				q->set(ch,true);
				q->set(ch-'a'+'A',true);
			}
		}
		else if(ch=='d')
		{
			for(char ch='0';ch<='9';ch++)
			{
				q->set(ch,true);
			}
		}
		else if(ch=='s')
		{
			q->set(' ',true);
			q->set('\t',true);	
		}	
	}

	void set(char ch,bool f)
	{
		int n=((unsigned char)ch)/32;
		int d=((unsigned char)ch)%32;
		if(f) bitmap[n]|=1<<d;
		else bitmap[n]&=~(1<<d);
	}

	bool get(char ch)
	{
		int n=((unsigned char)ch)/32;
		int d=((unsigned char)ch)%32;
		return (bitmap[n] & (1<<d))!=0;
	}

	virtual void print(int n)
	{
		regex_item::print(n);
		::printf("map\n");
	}

	void inv()
	{
		for(int i=0;i<ndig;i++)
		{
			bitmap[i]=~bitmap[i];
		}
	}
};


class regex_item_char_str : public regex_item
{
public:
	regex_item_char_str(const String& s,bool flag):regex_item(flag?ITEM_CHAR_STR_NOCASE:ITEM_CHAR_STR),value(s){}
	regex_item_char_str(char ch,bool flag):regex_item(flag?ITEM_CHAR_STR_NOCASE:ITEM_CHAR_STR)
	{
		value.append(&ch,1);
	}

	String value;

	virtual void print(int n)
	{
		regex_item::print(n);
		::printf("str: %s\n",value.c_str());
	}

};

class regex_item_repeat;
class regex_item_repeat_next : public regex_item
{
public:
	regex_item_repeat& node;
	bool match_as_much_as_possible;

	regex_item_repeat_next(regex_item_repeat& p):regex_item(ITEM_REPEAT_NEXT),node(p)
	{
		match_as_much_as_possible=true;
	}
};

class regex_item_repeat : public regex_item
{
public:
	regex_item_repeat():regex_item(ITEM_REPEAT),repeat_end(*this)
	{
		nmin= 0;
		nmax=-1;
	}

	regex_item_repeat_next repeat_end;

	void update(regex_item_state& q);
	void update(regex_item* q);

	regex_item* real_sibling;
	int index;

	AutoPtrT<regex_item> child;

	virtual void print(int n)
	{
		regex_item::print(n);
		::printf("repeat\n");
		child->print(n+1);
	}


	int nmin,nmax;
};

class regex_item_or : public regex_item
{
public:
	regex_item_or():regex_item(ITEM_OR)
	{

	}

	AutoPtrT<regex_item> child1,child2;

	void update(regex_item_state& q)
	{
		regex_item::update(q);

		child1->update(q);
		child2->update(q);

		sibling=child1.get();
	}

	virtual void print(int n)
	{
		regex_item::print(n);
		::printf("or\n");
		child1->print(n+1);
		child2->print(n+1);
	}
};


class regex_item_seq : public regex_item, public ObjectData
{
public:

	regex_item seqend;

	regex_item_seq():regex_item(ITEM_SEQ),seqend(ITEM_SEQ_END){}

	AutoPtrT<regex_item> child;

	virtual void print(int n)
	{
		regex_item::print(n);
		::printf("seq\n");
		for(regex_item* q=child.get();q;q=q->next)
		{
			q->print(n+1);	
		}
	}

	int index;

	void update(regex_item_state& q);

	void adjust();

	void append(regex_item* p);

};




class RegexParser
{
public:
	typedef const char* iterator;

	RegexParser(int f):flags(f){}

	BitFlags flags;

	static int read_number(const char* &p1);

	regex_item* parse_item(const char* &p1,bool seq);

	regex_item* parse_item_ex(const char* &p1,bool seq);

	regex_item_seq* parse_seq(const char* &p1);

	regex_item_seq* parse(const String& s);


};

EW_LEAVE
#endif
