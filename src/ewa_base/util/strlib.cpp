#include "ewa_base/util/strlib.h"
#include "ewa_base/basic/lookuptable.h"
#include "ewa_base/basic/codecvt.h"
#include "ewa_base/basic/system.h"

#ifdef EW_MSVC
#pragma warning(disable:4309)
#endif // EW_MSVC

EW_ENTER


arr_1t<String> string_words(const String& s)
{

	const char* p=s.c_str();

	bool last_is_not_white=!::isspace(p[0]);
	arr_1t<size_t> pos;

	size_t n=s.size();

	for(size_t i=0;i<n;i++)
	{
		if(::isspace(p[i]))
		{
			last_is_not_white=true;
		}
		else if(last_is_not_white)
		{
			pos.push_back(i);
			last_is_not_white=false;
		}
	}

	if(!last_is_not_white) pos.push_back(n);

	arr_1t<String> columns;
	for(size_t i=1;i<pos.size();i++)
	{
		columns.push_back(string_trim(String(p+pos[i-1],p+pos[i])));
	}

	return columns;

}

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
		sb[i] = lk::cmap[(unsigned char)sb[i]];
	}
	return sb;
}

String string_to_upper(const String& s)
{
	typedef lookup_table<lkt2uppercase> lk;
	StringBuffer<char> sb(s);
	for (size_t i = 0; i < sb.size(); ++i)
	{
		sb[i] = lk::cmap[(unsigned char)sb[i]];
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




String string_unescape(const String& s)
{
	StringBuffer<char> sb;
	const unsigned char* p = (const unsigned char*)s.c_str();
	while(*p)
	{
		if (*p != '%')
		{
			sb.push_back(*p++);
			continue;
		}

		int v1 = lookup_table<lkt_number16b>::test(p[1]);
		if (v1<16)
		{
			int v2=lookup_table<lkt_number16b>::test(p[2]);
			if(v2<16)
			{
				sb.push_back(v1 * 16 + v2);
				p += 3;
			}
			else
			{
				System::LogTrace("??? escape: "+String(p,3));

				sb.push_back(p[0]);
				sb.push_back(p[1]);
				p += 2;
			}
		}
		else if (p[1] != 'u')
		{
			System::LogTrace("??? escape: "+String(p,2));

			sb.push_back(p[0]);
			p += 1;
			continue;;
		}
		else
		{

			wchar_t vv = 0;
			for (int i = 0; i < 4; i++)
			{
				v1 = lookup_table<lkt_number16b>::test(p[i+2]);
				if (v1 >=16)
				{
					System::LogTrace("??? escape: "+String(p,3+i));
					break;
				}
				vv = (vv << 4) + v1;
			}

			StringBuffer<char> wb;
			IConv::unicode_to_utf8(wb, &vv, 1);
			sb << wb;
			p += 6;
		}
	}

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
	static const unsigned char value = (unsigned char) (N < 10 ? '0' + N : (N < 16 ? 'A' + N - 10 : -1));
};


String string_escape(const String& str_src)
{
	StringBuffer<wchar_t> wb(str_src);
	StringBuffer<char> sb;
	for (const wchar_t* p = wb.c_str(); *p;p++)
	{
		if(*p>=256)
		{
			sb.push_back('%');
			sb.push_back('u');
			sb.push_back(lookup_table<lkt_10b_to_16b_char>::test(((*p)&0xF000)>>12));
			sb.push_back(lookup_table<lkt_10b_to_16b_char>::test(((*p)&0x0F00)>>8));
			sb.push_back(lookup_table<lkt_10b_to_16b_char>::test(((*p)&0x00F0)>>4));
			sb.push_back(lookup_table<lkt_10b_to_16b_char>::test(((*p)&0x000F)>>0));
		}
		else if (lookup_table<lkt_need_escape>::test(*p))
		{
			sb.push_back('%');
			sb.push_back(lookup_table<lkt_10b_to_16b_char>::test(((*p)&0x00F0)>>4));
			sb.push_back(lookup_table<lkt_10b_to_16b_char>::test(((*p)&0x000F)>>0));
		}
		else
		{
			sb.push_back(*p);
		}
	}
	return sb;
}

String string_urlencode(const String& str_src)
{
	StringBuffer<char> sb;
	for (const unsigned char* p = (const unsigned char*)str_src.c_str(); *p;p++)
	{
		if (lookup_table<lkt_need_escape>::test(*p))
		{
			sb.push_back('%');
			sb.push_back(lookup_table<lkt_10b_to_16b_char>::test(((*p)&0x00F0)>>4));
			sb.push_back(lookup_table<lkt_10b_to_16b_char>::test(((*p)&0x000F)>>0));
		}
		else
		{
			sb.push_back(*p);
		}
	}
	return sb;
}


String string_urldecode(const String& s)
{
	StringBuffer<char> sb;
	const unsigned char* p = (const unsigned char*)s.c_str();
	while(*p)
	{
		if (*p != '%')
		{
			sb.push_back(*p++);
			continue;
		}

		int v1 = lookup_table<lkt_number16b>::test(p[1]);
		if (v1<16)
		{
			int v2=lookup_table<lkt_number16b>::test(p[2]);
			if(v2<16)
			{
				sb.push_back(v1 * 16 + v2);
				p += 3;
			}
			else
			{
				System::LogTrace("??? urldecode: "+String(p,3));

				sb.push_back(p[0]);
				sb.push_back(p[1]);
				p += 2;
			}
		}
		else if (p[1] != 'u')
		{
			System::LogTrace("??? urldecode: "+String(p,2));

			sb.push_back(p[0]);
			p += 1;
			continue;;
		}
		else
		{

			wchar_t vv = 0;
			for (int i = 0; i < 4; i++)
			{
				v1 = lookup_table<lkt_number16b>::test(p[i+2]);
				if (v1 >=16)
				{
					System::LogTrace("??? urldecode: "+String(p,3+i));
					break;
				}
				vv = (vv << 4) + v1;
			}

			StringBuffer<char> wb;
			IConv::unicode_to_utf8(wb, &vv, 1);
			sb << wb;
			p += 6;
		}
	}

	return sb;
}

String string_strencode(const String& str_src)
{
	StringBuffer<char> sb;

	const char* p1 = str_src.c_str();
	const char* p2 = p1;
	while (1)
	{
		char ch = *p2;
		if (ch == 0)
		{
			sb.append(p1, p2 - p1);
			break;			
		}
		else if (ch == '\"'||ch=='\\')
		{
			sb.append(p1, p2 - p1);
			sb.append("\\", 1);
			sb.append(ch);
			p1 = ++p2;
		}
		else if (ch == '\t')
		{
			sb.append(p1, p2 - p1);
			sb.append("\\\t", 2);
			p1 = ++p2;
		}
		else if (ch == '\r')
		{
			sb.append(p1, p2 - p1);
			sb.append("\\\r", 2);
			p1 = ++p2;
		}
		else if (ch == '\n')
		{
			sb.append(p1, p2 - p1);
			sb.append("\\\n", 2);
			p1 = ++p2;
		}
		else
		{
			++p2;
		}
	}

	return sb;
}

EW_LEAVE
