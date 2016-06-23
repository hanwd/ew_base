
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

class regex_item : public mp_obj
{
public:
	enum
	{
		ITEM_NONE,
		ITEM_SEQ,
		ITEM_SEQ_END,
		ITEM_CHAR_STR,
		ITEM_CHAR_STR_IGNORECASE,
		ITEM_CHAR_MAP,
		ITEM_CHAR_MAP_UNICODE,
		ITEM_CHAR_ANY,
		ITEM_REPEAT,
		ITEM_REPEAT_NEXT,
		ITEM_OR,
		ITEM_EXPR_ENTER,
		ITEM_EXPR_LEAVE,
		ITEM_LINE_ENTER,
		ITEM_LINE_LEAVE,
		ITEM_ID,
		ITEM_TRY_FALLBACK,
	};

	regex_item(int t):type(t),next(NULL),sibling(NULL)
	{

	}

	virtual regex_item* clone()
	{
		return new regex_item(*this);
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

class regex_item_id : public regex_item
{
public:

	regex_item_id(const String& s=""):regex_item(ITEM_ID),value(s)
	{
		
	}

	String value;
};

class regex_item_char_map : public regex_item
{
public:

	regex_item_char_map(int f=ITEM_CHAR_MAP):regex_item(f)
	{
		memset(bitmap,0,sizeof(bitmap));
	}

	virtual regex_item_char_map* clone()
	{
		return new regex_item_char_map(*this);
	}

	static const int ndig=256/32;
	uint32_t bitmap[ndig];
	BitFlags flags;

	void update_case();
	void add_chars(char ch);

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
		bool f=(bitmap[n] & (1<<d))!=0;
		return f;
	}

	virtual void print(int n)
	{
		regex_item::print(n);
		::printf("map\n");
	}

	virtual void inv()
	{
		for(int i=0;i<ndig;i++)
		{
			bitmap[i]=~bitmap[i];
		}
	}
};

class regex_item_char_map_unicode : public regex_item_char_map
{
public:

	enum
	{
		FLAG_MATCH_UNICODE_WORD=1<<0,
	};

	regex_item_char_map_unicode(regex_item_char_map& o)
		:regex_item_char_map(ITEM_CHAR_MAP_UNICODE)
	{
		flags.add(FLAG_MATCH_UNICODE_WORD);
		memcpy(bitmap,o.bitmap,sizeof(bitmap));
	}

	virtual regex_item_char_map_unicode* clone()
	{
		return new regex_item_char_map_unicode(*this);
	}

	regex_item_char_map_unicode():regex_item_char_map(ITEM_CHAR_MAP_UNICODE)
	{
		flags.add(FLAG_MATCH_UNICODE_WORD);
	}

	void inv()
	{
		flags.inv(FLAG_MATCH_UNICODE_WORD);
		regex_item_char_map::inv();
	}
};

class regex_item_char_str : public regex_item
{
public:
	regex_item_char_str(const String& s,bool flag):regex_item(flag?ITEM_CHAR_STR_IGNORECASE:ITEM_CHAR_STR),value(s){}
	regex_item_char_str(char ch,bool flag):regex_item(flag?ITEM_CHAR_STR_IGNORECASE:ITEM_CHAR_STR)
	{
		value.append(&ch,1);
	}

	virtual regex_item_char_str* clone()
	{
		return new regex_item_char_str(*this);
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

	virtual regex_item_repeat_next* clone()
	{
		return new regex_item_repeat_next(*this);
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

	virtual regex_item_repeat* clone()
	{
		AutoPtrT<regex_item_repeat> q(new regex_item_repeat);
		q->nmin=nmin;
		q->nmax=nmax;
		q->child.reset(child->clone());
		return q.release();
	}

	regex_item_repeat_next repeat_end;

	void update(regex_item_state& q);
	void update(regex_item* q);

	virtual void print(int n)
	{
		regex_item::print(n);
		::printf("repeat\n");
		child->print(n+1);
	}

	AutoPtrT<regex_item> child;
	int nmin,nmax;

	regex_item* real_sibling;
	int index;
};

class regex_item_or : public regex_item
{
public:
	regex_item_or():regex_item(ITEM_OR)
	{

	}

	virtual regex_item_or* clone()
	{
		AutoPtrT<regex_item_or> q(new regex_item_or);
		q->child1.reset(child1->clone());
		q->child2.reset(child2->clone());
		return q.release();
	}

	AutoPtrT<regex_item> child1,child2;

	void update(regex_item_state& q)
	{
		regex_item::update(q);

		LockState<regex_item*> lock(q.sibling,sibling);
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


class regex_item_seq : public regex_item
{
public:

	regex_item seqend;

	regex_item_seq():regex_item(ITEM_SEQ),seqend(ITEM_SEQ_END){}
	regex_item_seq(const regex_item_seq& o):regex_item(ITEM_SEQ),seqend(ITEM_SEQ_END)
	{
		for(regex_item* p=(regex_item*)o.child.get();p;p=p->next)
		{
			append(p->clone());
		}
	}


	virtual regex_item_seq* clone()
	{
		return new regex_item_seq(*this);
	}


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

class regex_item_root : public regex_item_seq , public ObjectData
{
public:
	BitFlags flags;
	String orig_str;

	virtual regex_item_root* clone()
	{
		return new regex_item_root(*this);
	}
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

	regex_item_char_map* parse_char_map(const char* &p1);

	regex_item_root* parse(const String& s);

};



EW_LEAVE
#endif
