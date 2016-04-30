#ifndef __H_EW_SCRIPTING_SCANNER__
#define __H_EW_SCRIPTING_SCANNER__

#include "ewa_base/basic.h"
#include "ewa_base/collection/indexer_map.h"

EW_ENTER


enum tokType
{
	TOK_ID,				// variable name
	TOK_KEY,			// keyword
	TOK_OP,
	TOK_INTEGER,
	TOK_DOUBLE,
	TOK_IMAGPART,
	TOK_STRING,
	TOK_COMMENT,
	TOK_DOT,
	TOK_PTR,
	TOK_COMMA,
	TOK_COLON,
	TOK_SEMICOLON,
	TOK_QUESTION,
	TOK_SHARP,
	TOK_AT,
	TOK_EQGT,
	TOK_BRA1,
	TOK_KET1,
	TOK_BRA2,
	TOK_KET2,
	TOK_BRA3,
	TOK_KET3,
	TOK_END,
};

class DLLIMPEXP_EWA_BASE tokInfo
{
public:
	tokInfo():line(-1),cpos(-1){}

	int32_t line;
	int32_t cpos;
};

class DLLIMPEXP_EWA_BASE tokItem : public tokInfo
{
public:	
	tokItem():type(TOK_ID),tags(0){}
	tokItem(const String& s):type(TOK_ID),tags(0),word(s){}

	tokType type;
	int tags;
	String word;

};


class DLLIMPEXP_EWA_BASE tokState
{
public:
	typedef char mychar;
	typedef const mychar* mychar_ptr;

	mychar_ptr pcur; //current position
	size_t line;
	size_t cpos;
	int last;

	void inc();

};

class DLLIMPEXP_EWA_BASE Parser;

class DLLIMPEXP_EWA_BASE ScannerBase : public Object
{
public:

	typedef tokState::mychar mychar;
	typedef tokState::mychar_ptr mychar_ptr;

protected:

	template<template<unsigned> class P> 
	static void skip(tokState& p);

	StringBuffer<mychar> tempbuf;

	mychar_ptr pbeg; 
	mychar_ptr pend;
	tokState stok;

	tokItem tokitem;

	arr_1t<tokItem> aTokens;
	indexer_map<String,tokType>& aKeyword;

	ScannerBase(indexer_map<String, tokType>&);

};


class DLLIMPEXP_EWA_BASE Scanner : public ScannerBase
{
public:

	friend class Parser;

	Scanner();

	bool parse(const String& p);

protected:

	void add_item();
	void gen_item(tokType t,mychar_ptr p1,mychar_ptr p2);
	void new_item(tokType t);
	void new_item(tokType t,const String& s);

	void read_name();

	void read_number();

	template<template<unsigned> class X,int N>
	void read_number_t();

	void read_string_raw(char br);
	void read_string(char br);

	void read_op2_a();

	void read_op2_b();

	void read_op2_c();

	void read_dot();

	// comment // ...
	void read_comment1();

	// comment /* ... */
	void read_comment2();

	void kerror(const String& s);

	void skip_whitespace();

	class CommentItem
	{
	public:
		CommentItem(int ln1=-1,int ln2=-1,mychar_ptr p1=NULL,mychar_ptr p2=NULL)
		:line1(ln1)
		,line2(ln2)
		,desc(p1,p2)
		{}

		int line1;
		int line2;
		String desc;
	};

	arr_1t<CommentItem> aComments;
};


template<template<unsigned> class P> 
void ScannerBase::skip(tokState& p)
{
	if(P<'\n'>::value)
	{
		while(lookup_table<P>::test(*p.pcur))
		{			
			if(*p.pcur++=='\n')
			{
				++p.line;
				p.cpos=1;
			}
			else
			{
				++p.cpos;
			}
		}
	}
	else
	{
		while(lookup_table<P>::test(*p.pcur))
		{
			++p.pcur;			
			++p.cpos;
		}
	}
}

EW_LEAVE

namespace tl
{
	template<> struct is_pod<ew::tokInfo> : public value_type<true>{};
};

#endif



