#include "ewa_base/util/strlib.h"
#pragma warning(disable:4309)

EW_ENTER


arr_1t<String> string_lines(const String& s)
{
	arr_1t<String> a;

	const char* s1=s.c_str();
	while (1)
	{
		const char* s2 = s1;
		while (*s2 && (*s2 != '\n')) s2++;

		if (s1 == s2)
		{
			a.push_back("");
		}
		else
		{
			a.push_back(String(s1, s2[-1] == '\r'?s2-1:s2));
		}

		if (!*s2) break;		
		s1 = s2 + 1;
	}

	return EW_MOVE(a);
}
arr_1t<String> string_split(const String& s,const String& b)
{
	arr_1t<String> a;

	const char* s1=s.c_str();
	while(1)
	{
		const char* s2=::strstr(s1,b.c_str());
		if(s2==NULL)
		{
			a.push_back(String(s1));
			break;
		}
		else
		{
			a.push_back(String(s1,s2));
			s1=s2+b.size();
		}
	}

	return EW_MOVE(a);
}


String string_replace(const String& str_src,const String& str_old,const String& str_new)
{
	String tmp(str_src);
	tmp.replace(str_old,str_new);
	return EW_MOVE(tmp);
}


int string_compare_no_case(const String& lhs, const String& rhs)
{
	typedef lookup_table<lkt2lowercase> lk;
	unsigned char* p1 = (unsigned char*)lhs.c_str();
	unsigned char* p2 = (unsigned char*)rhs.c_str();

	while (1)
	{
		unsigned char c1 = lk::cmap[*p1];
		unsigned char c2 = lk::cmap[*p2];
		if (c1 > c2) return +1;
		if (c1 < c2) return -1;
		if (c1 == 0) return  0;
		p1++; p2++;
	}
	return 0;
}

String string_to_lower(const String& s)
{
	typedef lookup_table<lkt2lowercase> lk;
	StringBuffer<char> sb(s);
	for (size_t i = 0; i < sb.size(); ++i)
	{
		sb[i] = lk::cmap[sb[i]];
	}
	return sb;
}

String string_to_upper(const String& s)
{
	typedef lookup_table<lkt2uppercase> lk;
	StringBuffer<char> sb(s);
	for (size_t i = 0; i < sb.size(); ++i)
	{
		sb[i] = lk::cmap[sb[i]];
	}
	return sb;
}

String string_trim(const String& s, int flag, char ch)
{

	const char* p1 = s.c_str();
	const char* p2 = s.c_str() + ::strlen(p1);

	if (flag&TRIM_L) while (p1 != p2 && p1[ 0] == ch) p1++;
	if (flag&TRIM_R) while (p2 != p1 && p2[-1] == ch) p2--;

	if (flag&TRIM_M)
	{
		StringBuffer<char> sb;
		while (p1 != p2 && *p1 != ch) sb.append(*p1++);
		return sb;
	}
	else
	{
		return String(p1, p2);
	}
}

static int get_code(unsigned char ch)
{
	if (ch >= '0' && ch <= '9') return ch - '0';
	if (ch >= 'A' && ch <= 'F') return 10 + ch - 'A';
	if (ch >= 'a' && ch <= 'f') return 10 + ch - 'a';
	return -1;
}

String string_unescape(const String& s)
{
	StringBuffer<char> sb;
	const unsigned char* p = (const unsigned char*)s.c_str();
	for (; *p;)
	{
		if (*p == '%')
		{
			int v1 = lookup_table<lkt_number16b>::test(p[1]);
			if (v1<0)
			{
				if (p[1] != 'u')
				{
					sb.push_back(p[0]);
					p += 1;				
					continue;;
				}
	
				wchar_t vv = 0;
				for (int i = 0; i < 4; i++)
				{
					v1 = lookup_table<lkt_number16b>::test(p[i+2]);	
					if (v1 < 0)
					{
						break;
					}
					vv = (vv << 4) + v1;
				}

				StringBuffer<char> wb;
				IConv::unicode_to_utf8(wb, &vv, 1);
				sb << wb;
				p += 6;

				continue;
			
			}

			int v2=lookup_table<lkt_number16b>::test(p[2]);
			if (v2<0)
			{
				sb.push_back(p[0]);
				sb.push_back(p[1]);
				p += 2;
				continue;
			}

			sb.push_back(v1 * 16 + v2);
			p += 3;
		}
		else
		{
			sb.push_back(*p);
			p += 1;
		}
	}

	IConv::utf8_to_ansi(sb,sb.data(),sb.size());
	return sb;
}

template<unsigned N>
class lkt_need_escape
{
public:

	static const unsigned char value1 = N >= '0'&&N <= '9';
	static const unsigned char value2 = N >= 'a'&&N <= 'z';
	static const unsigned char value3 = N >= 'A'&&N <= 'Z';
	static const unsigned char value4 = N == '_' || N == '@' || N == '+' || N == '-' || N == '.' || N == '/'||N=='*';

	static const unsigned char value = !(value1 | value2 | value3 | value4);
};

template<unsigned N>
class lkt_10b_to_16b_char
{
public:
	static const unsigned char value = N < 10 ? '0' + N : (N < 16 ? 'A' + N - 10 : -1);
};

String string_escape(const String& str_src)
{
	StringBuffer<char> sb;


	//StringBuffer<wchar_t> kb;
	//IConv::ansi_to_unicode(kb,str_src.c_str(),str_src.size());



	//for (const wchar_t* p = (const wchar_t*)kb.c_str(); *p;p++)
	//{
	//	if(*p>=256)
	//	{
	//		char buf[8];
	//		sprintf(buf,"%0X",(int)*p);
	//		sb.push_back('%');
	//		sb.push_back('u');
	//		sb.push_back(buf[0]);
	//		sb.push_back(buf[1]);
	//		sb.push_back(buf[2]);
	//		sb.push_back(buf[3]);
	//	}
	//	else if (lookup_table<lkt_need_escape>::test(*p))
	//	{
	//		sb.push_back('%');
	//		sb.push_back(lookup_table<lkt_10b_to_16b_char>::test((*p) >> 4));
	//		sb.push_back(lookup_table<lkt_10b_to_16b_char>::test((*p) & 0x0F));
	//	}
	//	else
	//	{
	//		sb.push_back(*p);
	//	}
	//}
	//return sb;

	StringBuffer<char> kb;
	IConv::ansi_to_utf8(kb,str_src.c_str(),str_src.size());

	for (const unsigned char* p = (const unsigned char*)kb.c_str(); *p;p++)
	{
		if (lookup_table<lkt_need_escape>::test(*p))
		{
			sb.push_back('%');
			sb.push_back(lookup_table<lkt_10b_to_16b_char>::test((*p) >> 4));
			sb.push_back(lookup_table<lkt_10b_to_16b_char>::test((*p) & 0x0F));
		}
		else
		{
			sb.push_back(*p);
		}
	}
	return sb;
}


EW_LEAVE
