#ifndef __H_EW_BASIC_UTILS__
#define __H_EW_BASIC_UTILS__

#include "ewa_base/basic/string.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/collection/arr_1t.h"

EW_ENTER

template<typename T>
String string_join(T t1,T t2,const String& br)
{
	if(t1==t2) return "";

	StringBuffer<char> sb;

	while(1)
	{
		sb<<(*t1++);
		if(t1==t2) break;
		sb<<br;
	}

	return sb;
}

DLLIMPEXP_EWA_BASE arr_1t<String> string_split(const String& s, const String& b);
DLLIMPEXP_EWA_BASE arr_1t<String> string_lines(const String& s);
DLLIMPEXP_EWA_BASE arr_1t<String> string_words(const String& s);

DLLIMPEXP_EWA_BASE String string_replace(const String& str_src, const String& str_old, const String& str_new);

DLLIMPEXP_EWA_BASE String string_unescape(const String& str_src);
DLLIMPEXP_EWA_BASE String string_escape(const String& str_src);

DLLIMPEXP_EWA_BASE String string_urlencode(const String& str_src);
DLLIMPEXP_EWA_BASE String string_urldecode(const String& str_src);

DLLIMPEXP_EWA_BASE String string_strencode(const String& str_src);

DLLIMPEXP_EWA_BASE int string_compare_no_case(const String& lhs, const String& rhs);

enum
{
	TRIM_L=1<<0,
	TRIM_R=1<<1,
	TRIM_M=1<<2,
	TRIM_B=TRIM_L|TRIM_R,
	TRIM_A=TRIM_B|TRIM_M,
};

DLLIMPEXP_EWA_BASE String string_trim(const String& s, int flag = TRIM_B, char ch = ' ');

DLLIMPEXP_EWA_BASE String string_to_lower(const String& s);
DLLIMPEXP_EWA_BASE String string_to_upper(const String& s);

EW_LEAVE
#endif
