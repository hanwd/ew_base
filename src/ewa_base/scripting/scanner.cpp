#include "ewa_base/scripting/scanner.h"
#include "ewa_base/basic/lookuptable.h"

EW_ENTER


template<unsigned N>
class lkt_is_namestart
{
public:
	static const int value=lkt_is_character<N>::value||N=='_';
};

template<unsigned N>
class lkt_is_name
{
public:
	static const int value=lkt_is_namestart<N>::value||lkt_is_number10<N>::value;
};

template<unsigned N>
class lkt_is_string
{
public:
	static const int value=N!='\0'&&N!='\"'&&N!='\\'&&N!='\'';
};

template<unsigned N>
class lkt_is_string_no_crln
{
public:
	static const int value = N != '\0'&&N != '\"'&&N != '\\'&&N != '\'' && N != '\r' && N != '\n';
};

template<unsigned N>
class lkt_is_string_raw
{
public:
	static const int value = N != '\0'&& N != '\''&& N != '\"';
};

template<unsigned N>
class lkt_is_comment2
{
public:
	static const int value=N!='\0'&&N!='/';
};

template<unsigned N>
class lkt_is_comment1
{
public:
	static const int value=N!='\0'&&N!='\n';
};


template<unsigned N>
class lkt_hex_val
{
public:
	static const int value_n = lkt_is_number10<N>::value?1:0;
	static const int value_a = (N >= 'a'&&N <= 'f')?1:0;
	static const int value_A = (N >= 'A'&&N <= 'F')?1:0;
	static const int not_num = (value_n == 0 && value_a == 0 && value_A == 0)?1:0;
	static const int value =
		value_n*(N - '0') |
		value_a*(10 + N - 'a') |
		value_A*(10 + N - 'A') |
		not_num*(-1);

};



void Scanner::read_name()
{
	mychar_ptr p1=stok.pcur;
	skip<lkt_is_name>(stok);
	mychar_ptr p2=stok.pcur;
	tokitem.word.assign(p1,p2);	
	tokitem.type=aKeyword[tokitem.word];

	add_item();
}

template<template<unsigned> class X,int N>
void Scanner::read_number_t()
{
	mychar_ptr p1=stok.pcur;
	skip<X>(stok);
	mychar_ptr p2=stok.pcur;

	uint64_t v(0);
	lookup_table<lkt_hex_val> lkt;

	for (mychar_ptr p = p1 ; p != p2; p++)
	{
		unsigned val = lkt[*p];
		if (val >=N)
		{
			kerror("invalid ch");
		}
		v = (v*N)+val;
	}


	tokitem.type=TOK_INTEGER;
	tokitem.word = "";
	tokitem.word << v;

	if(stok.pcur[0]=='i'||stok.pcur[0]=='j')
	{
		tokitem.type=TOK_IMAGPART;
		++stok;
	}

	if(lookup_table<lkt_is_name>::test(*stok.pcur))
	{
		kerror("invalid number_end");
	}

	add_item();
	return;
}

template<unsigned N>
class lkt_is_character_or_0
{
public:
	static const int value=(N>='a'&&N<='z')||(N>='A'&&N<='Z')||N=='0';
};

void Scanner::read_number()
{
	mychar_ptr p1=stok.pcur;

	if(stok.pcur[0]=='0' && stok.pcur[1]!='.')
	{
		if(stok.pcur[1]=='x'||stok.pcur[1]=='X')
		{
			++stok;
			++stok;
			read_number_t<lkt_is_number16,16>();
			return;
		}

		if(stok.pcur[1]=='b'||stok.pcur[1]=='b')
		{
			++stok;
			++stok;
			read_number_t<lkt_is_number02,2>();
			return;
		}	

		if(stok.pcur[1]>='1'||stok.pcur[1]<'8')
		{
			++stok;
			read_number_t<lkt_is_number08,8>();
			return;
		}
	}


	skip<lkt_is_number10>(stok);
	if (stok.pcur[0] == '.')
	{
		if(stok.pcur[1] >='0'&& stok.pcur[1] <='9')
		{
			tokitem.type=TOK_DOUBLE;
			++stok;
			skip<lkt_is_number10>(stok);		
		}
		else if(stok.pcur[1] =='#')
		{
			tokitem.type=TOK_DOUBLE;
			++stok;
			++stok;
			skip<lkt_is_character_or_0>(stok);	
		}
		else
		{
			tokitem.type=TOK_INTEGER;	
		}
	}
	else
	{
		tokitem.type=TOK_INTEGER;
	}

	if(stok.pcur[0]=='e')
	{
		tokitem.type = TOK_DOUBLE;

		++stok;
		if (stok.pcur[0]=='-')
		{
			++stok;
		}
		else if(stok.pcur[0]=='+')
		{
			++stok;
		}
		skip<lkt_is_number10>(stok);
	}
	mychar_ptr p2=stok.pcur;

	if(stok.pcur[0]=='i'||stok.pcur[0]=='j')
	{
		tokitem.type=TOK_IMAGPART;
		++stok;
	}


	tokitem.word.assign(p1,p2);	
	add_item();
	
}


void Scanner::read_string_raw(char br)
{

	++stok;
	++stok;
	++stok;

	mychar_ptr p1 = stok.pcur;
	for (;;)
	{
		skip<lkt_is_string_raw>(stok);		

		if (stok.pcur[0] == br && stok.pcur[1]==br && stok.pcur[2]==br)
		{
			tokitem.word.assign(p1, stok.pcur);
			++stok;
			++stok;
			++stok;
			break;
		}

		if (stok.pcur[0] == '\0')
		{
			kerror("unexpected end of string");
			return;
		}

		++stok;
	}
	

	tokitem.type = TOK_STRING;
	add_item();

}


void Scanner::read_string(char br)
{
	StringBuffer<char> sb;

	while(stok.pcur[0]==br)
	{
		++stok;
		for(;;)
		{
			mychar_ptr p1=stok.pcur;
			skip<lkt_is_string_no_crln>(stok);
			sb.append(p1,stok.pcur);

			if(stok.pcur[0]==br)
			{
				++stok;
				if (lookup_table<lkt_is_name>::test(stok.pcur[0]))
				{
					kerror("syntax error");
				}
				break;
			}

			if(stok.pcur[0]=='\\')
			{
				switch(stok.pcur[1])
				{
				case 'n':
					sb.append('\n');
					break;
				case 't':
					sb.append('\t');
					break;
				case 'r':
					sb.append('\r');
					break;
				case 's':
					sb.append(' ');
					break;
				case 'f':
					sb.append(0x0C);
					break;
				case 'b':
					sb.append(0x08);
					break;
				case 'u':
					{
						lookup_table<lkt_hex_val> lkt;

						uint32_t v(0);
						for (mychar_ptr p = p1+2; p != p1+6; p++)
						{
							uint32_t val = lkt[*p];
							if (val > 15)
							{
								kerror("invalid unicode");
							}
							v = (v << 4) | val;
						}
						StringBuffer<char> sa;
						if (!IConv::unicode_to_ansi(sa, &v, 1))
						{
							kerror("invalid unicode");
						}
						sb.append(sa.begin(), sa.end());						

						++stok;	
						++stok;
						++stok;
						++stok;
						break;						
					}
				break;
				case '\\':
				case '\"':
				case '\'':
					sb.append(stok.pcur[1]);
					break;
				case '\n':
					break;
				case '\r':
					if (stok.pcur[2] == '\n')
					{
						++stok;
						break;
					}
				default:
					kerror("unkown string escape");
					break;
				}

				++stok;
				++stok;
			}
			else if (stok.pcur[0] == '\0' || stok.pcur[0] == '\r' || stok.pcur[0] == '\n')
			{
				kerror("unexpected end of string");
				return;
			}
			else
			{
				sb.append(stok.pcur[0]);
				++stok;
			}
		}
		skip<lkt_whitespace>(stok);
	}
	
	tokitem.type=TOK_STRING;
	tokitem.word=sb;
	add_item();

}

void Scanner::read_op2_a()
{
	mychar_ptr p1=stok.pcur;
	++stok;
	if(stok.pcur[0]=='=')
	{
		++stok;
		mychar_ptr p2=stok.pcur;
		gen_item(TOK_OP,p1,p2);
		return;
	}

	mychar_ptr p2=stok.pcur;
	gen_item(TOK_OP,p1,p2);	
}

void Scanner::read_op2_b()
{
	mychar_ptr p1=stok.pcur;
	if(stok.pcur[1]==stok.pcur[0]||stok.pcur[1]=='=')
	{
		++stok;
		++stok;
		mychar_ptr p2=stok.pcur;
		gen_item(TOK_OP,p1,p2);
		return;
	}

	++stok;
	mychar_ptr p2=stok.pcur;
	gen_item(TOK_OP,p1,p2);		
}

void Scanner::read_op2_c()
{
	mychar_ptr p1=stok.pcur;
	++stok;

	if(stok.pcur[-1]==stok.pcur[0])
	{
		++stok;
	}

	if(stok.pcur[0]=='=')
	{
		++stok;
	}

	mychar_ptr p2=stok.pcur;
	gen_item(TOK_OP,p1,p2);		
}

// comment /* ... */
void Scanner::read_comment2()
{
	++stok;
	++stok;
	if(stok.pcur[0]=='\0')
	{
		kerror("unexpected end of string");
		return;
	}

	int _lastline=stok.line;

	mychar_ptr p1=stok.pcur;

	int n=1;

	++stok;
	for(;;)
	{
		skip<lkt_is_comment2>(stok);
		if(stok.pcur[0]=='\0')
		{
			kerror("unexpected end of string");
			return;
		}
		if(stok.pcur[-1]=='*')
		{
			++stok;
			if(--n==0)
			{
				break;
			}
		}
		else if(stok.pcur[1]=='*')
		{
			n++;
		}
		++stok;
	}

	mychar_ptr p2=stok.pcur-2;

	aComments.push_back(CommentItem(_lastline,stok.line+1,p1,p2));

}

// comment // ...
void Scanner::read_comment1()
{
	++stok;
	++stok;

	int _lastline=stok.line;

	mychar_ptr p1=stok.pcur;
	skip<lkt_is_comment1>(stok);
	mychar_ptr p2=stok.pcur;

	aComments.push_back(CommentItem(_lastline,stok.line+1,p1,p2));

}


void Scanner::read_dot()
{
	mychar_ptr p1=stok.pcur;
	++stok;

	switch(stok.pcur[0])
	{
	case '*':
		if(stok.pcur[1]=='*') ++stok;
	case '/':
	case '\\':
		++stok;
		tokitem.type=TOK_OP;
		if (stok.pcur[0] == '=')
		{
			++stok;
		}
		tokitem.word.assign(p1, stok.pcur);
		break;
	case '.':
		++stok;
		if(stok.pcur[0]=='.')
		{
			++stok;
			tokitem.type=TOK_ID;
			tokitem.word.assign(p1,3);
		}
		else
		{
			tokitem.type=TOK_OP;
			tokitem.word.assign(p1,2);
		}
		break;
	default:
		tokitem.type=TOK_DOT;
		tokitem.word.assign(p1,1);
		break;
	}
	add_item();

}

indexer_map<String, tokType>& scanner_keywords()
{
	static indexer_map<String, tokType> aKeyword;
	if (aKeyword.empty())
	{
		aKeyword["if"] = TOK_KEY;
		aKeyword["else"] = TOK_KEY;
		aKeyword["do"] = TOK_KEY;
		aKeyword["while"] = TOK_KEY;
		aKeyword["for"] = TOK_KEY;
		aKeyword["switch"] = TOK_KEY;
		aKeyword["for_each"] = TOK_KEY;
		aKeyword["end"] = TOK_KEY;
		aKeyword["begin"] = TOK_KEY;
		aKeyword["global"] = TOK_KEY;
		aKeyword["local"] = TOK_KEY;

		aKeyword["in"] = TOK_KEY;

		aKeyword["continue"] = TOK_KEY;
		aKeyword["continue2"] = TOK_KEY;
		aKeyword["continue3"] = TOK_KEY;
		aKeyword["break"] = TOK_KEY;
		aKeyword["break2"] = TOK_KEY;
		aKeyword["break3"] = TOK_KEY;

		aKeyword["return"] = TOK_KEY;
		aKeyword["try"] = TOK_KEY;
		aKeyword["catch"] = TOK_KEY;
		aKeyword["finally"] = TOK_KEY;
		aKeyword["throw"] = TOK_KEY;

		aKeyword["case"] = TOK_KEY;
		aKeyword["cond"] = TOK_KEY;
		aKeyword["default"] = TOK_KEY;

		aKeyword["class"] = TOK_KEY;
		aKeyword["judge"] = TOK_KEY;
		aKeyword["function"] = TOK_KEY;
		aKeyword["def"] = TOK_KEY;


		aKeyword["true"] = TOK_KEY;
		aKeyword["false"] = TOK_KEY;
		aKeyword["nil"] = TOK_KEY;


	}
	return aKeyword;
}


ScannerBase::ScannerBase(indexer_map<String, tokType>& kw) :aKeyword(kw)
{

}

Scanner::Scanner() : ScannerBase(scanner_keywords())
{
	aComments.push_back(CommentItem());
}

void Scanner::skip_whitespace()
{
	skip<lkt_whitespace>(stok);
	tokitem.line=stok.line;
	tokitem.cpos=stok.cpos;
}

template<unsigned N>
class lk_sm_start
{
public:

	static const unsigned N1=lkt_is_namestart<N>::value?1:0;
	static const unsigned N2=N1>0?N1:(lkt_is_number10<N>::value?2:0);
	static const unsigned N3=N2>0?N2:((N>0&&N<5)?3:0);
	static const unsigned value=N3>0?N3:N;
};


bool Scanner::parse(const String& s_)
{

	pbeg=(mychar_ptr)s_.c_str();
	pend=(mychar_ptr)pbeg+s_.size();

	aTokens.clear();
	stok.pcur=pbeg;
	stok.line=1;
	stok.cpos=1;
	last=TOK_END;

	for(;;)
	{
		skip_whitespace();
		unsigned char ch=lookup_table<lk_sm_start>::test(stok.pcur[0]);	
		switch(ch)
		{
		case 0:
			tokitem.type=TOK_END;
			tokitem.word="eof";
			add_item();
			return true;
		case 1:
			read_name();
			break;
		case 2:
			read_number();
			break;
		case '\'':
		case '\"':
			if (stok.pcur[1] == ch && stok.pcur[2] == ch)
			{
				read_string_raw(ch);
				break;
			}
			read_string(ch);
			break;
		case '/':
			if(stok.pcur[1]=='/')
			{
				read_comment1();
				break;
			}
			else if(stok.pcur[1]=='*')
			{
				read_comment2();
				break;
			}
		case '\\':
		case '%':
		case '!':
		//case '>':
		//case '<':
		//case '*':
			read_op2_a();
			break;
		case '=':
			if(stok.pcur[1]=='=')
			{
				mychar_ptr p1=stok.pcur;
				++stok;
				++stok;
				if(stok.pcur[0]=='=')
				{
					++stok;
				}
				gen_item(TOK_OP,p1,stok.pcur);

			}
			else if (stok.pcur[1] == '>')
			{
				mychar_ptr p1=stok.pcur;
				++stok;
				++stok;
				gen_item(TOK_EQGT, p1, stok.pcur);
			}
			else
			{
				new_item(TOK_OP);
			}
			break;
		case '-':
			if(stok.pcur[1]=='>')
			{
				mychar_ptr p1=stok.pcur;
				++stok;++stok;
				gen_item(TOK_PTR,p1,stok.pcur);
				break;
			}
		case '+':
			read_op2_b();
			break;
		case '|':
		case '&':
		case '^':
		case '*':
		case '>':
		case '<':
			read_op2_c();
			break;
		case '?':
			new_item(TOK_QUESTION);
			break;
		case '~':
			new_item(TOK_OP);
			break;
		case '.':
			read_dot();
			break;
		case '(':
			new_item(TOK_BRA1);
			break;
		case '[':
			new_item(TOK_BRA2);
			break;
		case '{':
			new_item(TOK_BRA3);
			break;
		case ')':
			new_item(TOK_KET1);
			break;
		case ']':
			new_item(TOK_KET2);
			break;
		case '}':
			new_item(TOK_KET3);
			break;
		case ':':
			new_item(TOK_COLON);
			break;
		case ';':
			new_item(TOK_SEMICOLON);
			break;
		case '#':
			new_item(TOK_SHARP);
			break;
		case '@':
			new_item(TOK_KEY,"def");
			break;
		case ',':
			new_item(TOK_COMMA);
			break;
		default:
			kerror("unexpected character");
		}//switch
	}//for

	return true;
}


void Scanner::add_item()
{
	aTokens.push_back(tokitem);
	tokitem.tags = 0;
}

void Scanner::gen_item(tokType t,mychar_ptr p1,mychar_ptr p2)
{
	tokitem.type=t;
	tokitem.word.assign(p1,p2);	
	add_item();
}

void Scanner::new_item(tokType t)
{
	tokitem.type=t;
	tokitem.word.assign(stok.pcur,1);
	++stok;
	add_item();
}
void Scanner::new_item(tokType t, const String& s)
{
	tokitem.type=t;
	tokitem.word=s;
	++stok;
	add_item();
}

void Scanner::kerror(const String& s)
{
	Exception::XError(String::Format("Scanner:%s at line:%d pos:%d",s,(int)stok.line,(int)stok.cpos));
}

EW_LEAVE
