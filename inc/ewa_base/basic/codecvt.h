#ifndef __H_EW_BASIC_CODECVT__
#define __H_EW_BASIC_CODECVT__

#include "ewa_base/config.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/basic/stringbuffer.h"

#ifdef EW_WINDOWS
#include "windows.h"
#else
#include <iconv.h>
#include <errno.h>
#endif

EW_ENTER

template<int N> class char_implN;
template<> class char_implN<1>
{
public:
	typedef unsigned char type;
};
template<> class char_implN<2>
{
public:
	typedef uint16_t type;
};
template<> class char_implN<4>
{
public:
	typedef uint32_t type;
};
template<typename T> class char_impl : public char_implN<sizeof(T)> {};


class DLLIMPEXP_EWA_BASE IConv
{
public:

	static StringBuffer<wchar_t> to_wide(const String& s)
	{
		StringBuffer<wchar_t> sb;
		utf8_to_unicode(sb,s.c_str(),s.size());
		return sb;
	}

#ifdef EW_WINDOWS
	static StringBuffer<char> to_ansi(const String& s)
	{
		StringBuffer<char> sb;
		utf8_to_ansi(sb,s.c_str(),s.size());
		return sb;
	}
	static String from_ansi(const char* s)
	{
		StringBuffer<char> sb;
		ansi_to_utf8(sb,s,::strlen(s));
		return sb.c_str();
	}

#else
	static const String& to_ansi(const String& s){return s;}
	static const String from_ansi(const char* s){return s;}

#endif

	static String from_gbk(const char* s)
	{
		StringBuffer<char> sb;
		gbk_to_utf8(sb,s,::strlen(s));
		return sb.c_str();
	}

	static String from_unknown(const char* s);

	static bool unicode_to_gbk(StringBuffer<uint8_t>& aa_,const uint16_t* pw_,size_t ln_);
	static bool unicode_to_gbk(StringBuffer<uint8_t>& aa_,const uint32_t* pw_,size_t ln_);

	static bool gbk_to_unicode(StringBuffer<uint16_t>& aw_,const uint8_t* pa_,size_t ln_);
	static bool gbk_to_unicode(StringBuffer<uint32_t>& aw_,const uint8_t* pa_,size_t ln_);

	static bool unicode_to_utf8(StringBuffer<uint8_t>& aa_,const uint16_t* pw_,size_t ln_);
	static bool unicode_to_utf8(StringBuffer<uint8_t>& aa_,const uint32_t* pw_,size_t ln_);

	static bool utf8_to_unicode(StringBuffer<uint16_t>& aw_,const uint8_t* pa_,size_t ln_);
	static bool utf8_to_unicode(StringBuffer<uint32_t>& aw_,const uint8_t* pa_,size_t ln_);

	template<typename WC>
	static bool unicode_to_gbk(StringBuffer<char>& aa_,const WC* pw_,size_t ln_)
	{
		typedef typename char_impl<WC>::type char_wcs;
		return unicode_to_gbk((StringBuffer<uint8_t>&)aa_,(const char_wcs*)pw_,ln_);
	}

	template<typename WC>
	static bool gbk_to_unicode(StringBuffer<WC>& aw_,const char* pa_,size_t ln_)
	{
		typedef typename char_impl<WC>::type char_wcs;
		return gbk_to_unicode((StringBuffer<char_wcs>&)aw_,(const uint8_t*)pa_,ln_);
	}

	template<typename WC>
	static bool unicode_to_utf8(StringBuffer<char>& aa_,const WC* pw_,size_t ln_)
	{
		typedef typename char_impl<WC>::type char_wcs;
		return unicode_to_utf8((StringBuffer<uint8_t>&)aa_,(const char_wcs*)pw_,ln_);
	}

	template<typename WC>
	static bool utf8_to_unicode(StringBuffer<WC>& aw_,const char* pa_,size_t ln_)
	{
		typedef typename char_impl<WC>::type char_wcs;
		return utf8_to_unicode((StringBuffer<char_wcs>&)aw_,(const uint8_t*)pa_,ln_);
	}


	template<typename WC>
	static bool unicode_to_ansi(StringBuffer<char>& aa_,const WC* pw_,size_t ln_)
	{
#ifdef EW_WINDOWS
		return unicode_to_gbk(aa_,pw_,ln_);
#else
		return unicode_to_utf8(aa_,pw_,ln_);
#endif
	}

	template<typename WC>
	static bool ansi_to_unicode(StringBuffer<WC>& aw_,const char* pa_,size_t ln_)
	{
#ifdef EW_WINDOWS
		return gbk_to_unicode(aw_,pa_,ln_);
#else
		return utf8_to_unicode(aw_,pa_,ln_);
#endif
	}

	static bool utf8_to_ansi(StringBuffer<char>& aa_,const char* pa_,size_t ln_);
	static bool ansi_to_utf8(StringBuffer<char>& aa_,const char* pa_,size_t ln_);
	static bool utf8_to_gbk(StringBuffer<char>& aa_,const char* pa_,size_t ln_);
	static bool gbk_to_utf8(StringBuffer<char>& aa_,const char* pa_,size_t ln_);
};



EW_LEAVE
#endif
