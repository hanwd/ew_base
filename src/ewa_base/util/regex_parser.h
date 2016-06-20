
#ifndef __H_EW_UTIL_REGEX_PARSER__
#define __H_EW_UTIL_REGEX_PARSER__

#include "ewa_base/basic.h"

EW_ENTER

class regex_item
{
public:
	enum
	{
		ITEM_NONE,
		ITEM_SEQ,
		ITEM_SEQ_END,
		ITEM_CHAR_STR,
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

	virtual void update(regex_item* q)
	{
		sibling=next?next:q;
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

	void set(const std::string& s)
	{
		const char* p=s.c_str();
		while(*p) set(*p++,true);
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
	regex_item_char_str(const std::string& s):regex_item(ITEM_CHAR_STR),value(s){}
	regex_item_char_str(char ch):regex_item(ITEM_CHAR_STR)
	{
		value.push_back(ch);
	}

	std::string value;

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

	void update(regex_item* q);

	regex_item* real_sibling;

	std::auto_ptr<regex_item> child;

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

	std::auto_ptr<regex_item> child1,child2;

	void update(regex_item* q)
	{
		regex_item::update(q);

		child1->update(sibling);
		child2->update(sibling);

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

	std::auto_ptr<regex_item> child;

	virtual void print(int n)
	{
		regex_item::print(n);
		::printf("seq\n");
		for(regex_item* q=child.get();q;q=q->next)
		{
			q->print(n+1);	
		}
	}

	void update(regex_item* q)
	{
		seqend.sibling=next?next:q;

		for(regex_item* p=child.get();p;p=p->next)
		{
			p->update(&seqend);
		}
		sibling=child.get();
	}

	void adjust();

	void append(regex_item* p)
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
};




class RegexParser
{
public:
	typedef const char* iterator;

	static int read_number(const char* &p1);

	regex_item* parse_item(const char* &p1,bool seq);

	regex_item* parse_item_ex(const char* &p1,bool seq);

	regex_item_seq* parse_seq(const char* &p1);

	regex_item_seq* parse(const String& s)
	{
		const char* p=s.c_str();
		regex_item_seq* q=parse_seq(p);
		if(q) q->update(NULL);
		return q;
	}


};

EW_LEAVE
#endif
