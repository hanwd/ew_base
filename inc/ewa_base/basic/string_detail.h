#ifndef __H_EW_STRING_DETAIL_H__
#define __H_EW_STRING_DETAIL_H__

#include "ewa_base/config.h"
#include "ewa_base/basic/exception.h"
#include <cstring>
#include <cstdarg>


EW_ENTER

DLLIMPEXP_EWA_BASE void* mp_alloc(size_t n);
DLLIMPEXP_EWA_BASE void mp_free(void* p);

class DLLIMPEXP_EWA_BASE String;


template<typename T>
class DLLIMPEXP_EWA_BASE StringBuffer;

class DLLIMPEXP_EWA_BASE WStringProxy
{
public:
	WStringProxy();
	WStringProxy(const WStringProxy&);
	WStringProxy& operator=(const WStringProxy&);

	WStringProxy(const wchar_t* p);
	WStringProxy(const wchar_t* p,size_t n);
	~WStringProxy();

	void reset(const wchar_t* p);
	void reset(const wchar_t* p,size_t n);

	operator const char*() const {return m_ptr;}
	const char* c_str() const {return m_ptr;}
	size_t size() const {return m_size;}

private:
	const char* m_ptr;
	size_t m_size;
};

class FormatPolicy
{
public:
	template<typename G>
	static inline size_t width(const G& o)
	{
		return 128;
	}

	template<typename T>
	static inline size_t width(const std::basic_string<T>& o)
	{
		return width(o.c_str());
	}

	template<typename T>
	static inline size_t width(const StringBuffer<T>& o)
	{
		return width(o.c_str());
	}

	static inline size_t width(const void* o)
	{
		return 32;
	}

	static inline size_t width(const char* o)
	{
		return std::char_traits<char>::length(o)+8;
	}

	static inline size_t width(const wchar_t* o)
	{
		return std::char_traits<wchar_t>::length(o)*2+8;
	}


	template<typename G>
	static inline G cast(const G& v){return v;}

	static inline const char* cast(const char* v){return v;}
	static inline const wchar_t* cast(const wchar_t* v){return v;}

	static const char* cast(const String& v);

	template<typename T>
	static inline const T* cast(const std::basic_string<T>& v){return v.c_str();}

	template<typename T>
	static const T* cast(const StringBuffer<T>& v);

	template<typename T,typename G>
	static T* do_format_integer(T* p,G v)
	{
		typedef typename unsigned_integer_type<sizeof(G)>::type U;

		p[0]=T(0);
		if(v==0)
		{
			*--p='0';
			return p;
		}

		bool sign=v<0;		
		U u=sign?~v+1:v;
		while(u>0)
		{
			*--p='0'+(u%10);
			u=u/10;
		}

		if(sign) *--p='-';

		return p;
	}
};

template<typename B>
class BContainerPolicy1
{
public:

	typedef B container_type;

	static container_type& return_initialized_buffer(container_type& b,size_t n,char* p)
	{
		b.append(p,n);
		mp_free(p);
		return b;
	}

	static char* get_uninitialized_buffer(container_type& b,size_t n)
	{
		char* p=(char*)mp_alloc(n);
		if(!p) Exception::XBadAlloc();
		return p;
	}

	static container_type& append(container_type& b,const char* p,size_t n)
	{
		b.append(p,n);
		return b;
	}
};

template<typename B>
class BContainerPolicy2
{
public:

	typedef B container_type;

	static container_type& return_initialized_buffer(container_type& b,size_t n,char* p)
	{
		b.return_initialized_buffer(n);
		return b;
	}

	static char* get_uninitialized_buffer(container_type& b,size_t n)
	{
		return b.get_uninitialized_buffer(n);
	}

	static container_type& append(container_type& b,const char* p,size_t n)
	{
		b.append(p,n);
		return b;
	}
};


template<template<typename> class C,int N=0>
class BFormatPolicy : public FormatPolicy
{
public:

	template<typename B,typename G>
	static B& on_sprint_error(B& b,size_t n,const char* s,G v)
	{
		ew::OnNoop();
		return b;
	}

	template<typename B,typename G>
	static B& format(B& b,size_t n,const char* s,G v)
	{
		if(N>0 && n<N)
		{
			char p[N+1];
			int nd=::sprintf(p,s,v);
			if(nd>=0)
			{
				return C<B>::append(b,p,nd);
			}
			else
			{
				return on_sprint_error(b,n,s,v);
			}
		}
		else
		{
			char* p=C<B>::get_uninitialized_buffer(b,n);
			int nd=::sprintf(p,s,v);
			if(nd>=0)
			{
				C<B>::return_initialized_buffer(b,nd,p);
			}
			else
			{
				C<B>::return_initialized_buffer(b,0,p);
				on_sprint_error(b,n,s,v);
			}
			return b;
		}
	}

	template<typename B,typename G>
	static B& format_integer(B& b,G& o)
	{
		char buf[64];
		char* p2=buf+63;
		char* p1=FormatPolicy::do_format_integer(p2,o);
		return C<B>::append(b,p1,p2-p1);
	}

	template<typename B>
	static B& append(B& b,const char* p)
	{
		return C<B>::append(b,p,std::char_traits<char>::length(p));
	}

	template<typename B>
	static B& append(B& b,const char* p,size_t n)
	{
		return C<B>::append(b,p,n);
	}

	template<typename B>
	static B& append(B& b,const WStringProxy& o)
	{
		return C<B>::append(b,o.c_str(),o.size());
	}
};

class FormatPolicy1 : public BFormatPolicy<BContainerPolicy1,1024>
{
public:

};

class FormatPolicy2 : public BFormatPolicy<BContainerPolicy2>
{
public:

};


template<typename B,typename P=FormatPolicy1>
class FormatHelper
{
public:

	typedef P Policy;
	B& operator<<(bool v){return P::append(_fmt_container(),v?"true":"false");}
	B& operator<<(char v){	return P::append(_fmt_container(),&v,1);}
	B& operator<<(wchar_t v){return P::append(_fmt_container(),WStringProxy(&v,1));}
	B& operator<<(int32_t v){return P::format_integer(_fmt_container(),v);}
	B& operator<<(int64_t v){return P::format_integer(_fmt_container(),v);}
	B& operator<<(uint32_t v){return P::format_integer(_fmt_container(),v);}
	B& operator<<(uint64_t v){return P::format_integer(_fmt_container(),v);}
	B& operator<<(float v){return P::format(_fmt_container(),64,"%g",double(v));}
	B& operator<<(double v){return P::format(_fmt_container(),64,"%g",v);}
	B& operator<<(long double v){return P::format(_fmt_container(),64,"%Lg",v);}
	B& operator<<(const char* v){return P::append(_fmt_container(),v);}
	B& operator<<(const unsigned char* v){return P::append(_fmt_container(),(const char*)v);}
	B& operator<<(const wchar_t* v){return P::append(_fmt_container(),WStringProxy(v));}
	B& operator<<(const void* v){return P::format(_fmt_container(),16,"%p",v);}
	B& operator<<(const String& v){return P::append(_fmt_container(),P::cast(v));}
	B& operator<<(const StringBuffer<char>& v){return P::append(_fmt_container(),P::cast(v));}
	B& operator<<(const std::basic_string<char>& v){return P::append(_fmt_container(),P::cast(v));}

private:
	B& _fmt_container(){return static_cast<B&>(*this);}
};


class StringDetail
{
public:

	typedef char char_type;

	static inline char_type* str_empty()
	{
		return (char_type*)const_empty_buffer;
	}

	static inline char_type* str_alloc(size_t s)
	{
		char_type* p=(char_type*)mp_alloc(s+1);
		if(p==NULL) Exception::XBadAlloc();
		p[s]=0;return p;
	}

	static inline void str_free(const char_type* p)
	{
		if(p!=const_empty_buffer)
		{
			mp_free((void*)p);
		}
	}

	static inline char_type* str_dup(const char_type* s)
	{
		size_t n=std::char_traits<char_type>::length(s);
		return str_dup(s,n);
	}

	static inline char_type* str_dup(const char_type* s,size_t n)
	{
		if(n==0)
		{
			return str_empty();
		}
		char_type* dst=str_alloc(n);
		memcpy(dst,s,sizeof(char_type)*n);
		return dst;
	}

	static inline char_type* str_cat(const char_type* p1,const char_type* p2)
	{
		size_t n1=std::char_traits<char_type>::length(p1);
		size_t n2=std::char_traits<char_type>::length(p2);
		return str_cat(p1,n1,p2,n2);
	}

	static inline char_type* str_cat(const char_type* p1,size_t n1,const char_type* p2,size_t n2)
	{
		size_t n=n1+n2;
		char_type* dst=str_alloc(n);
		memcpy(dst,p1,sizeof(char_type)*n1);
		memcpy(dst+n1,p2,sizeof(char_type)*n2);
		return dst;
	}

};


#define STRING_FORMATER_FORMAT_FUNCS_4(X,Y,Z,D)\
template<\
	typename T0,\
	typename T1,\
	typename T2=tl::nulltype,\
	typename T3=tl::nulltype,\
	typename T4=tl::nulltype,\
	typename T5=tl::nulltype,\
	typename T6=tl::nulltype,\
	typename T7=tl::nulltype,\
	typename T8=tl::nulltype,\
	typename T9=tl::nulltype\
>\
X(\
	D\
	const T0& v0,\
	const T1& v1,\
	const T2& v2=T2(),\
	const T3& v3=T3(),\
	const T4& v4=T4(),\
	const T5& v5=T5(),\
	const T6& v6=T6(),\
	const T7& v7=T7(),\
	const T8& v8=T8(),\
	const T9& v9=T9()\
)\
{\
	Y fb(FormatPolicy::cast(v0));\
	StringFormater::Format(fb,v1,v2,v3,v4,v5,v6,v7,v8,v9);\
	Z;\
}\
template<typename T0> X(D const T0& v0)\
{\
	Y fb(FormatPolicy::cast(v0));\
	Z;\
}

#define STRING_FORMATER_FORMAT_FUNCS_SB(X,Z) STRING_FORMATER_FORMAT_FUNCS_4(X,FormatStateSb,Z,)


EW_LEAVE
#endif
