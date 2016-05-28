
#include "string_impl.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/basic/codecvt.h"
#include <string>

#include <stdio.h>

#ifdef EW_WINDOWS
#include <windows.h>
#else
#include <cerrno>
#include <iconv.h>
#endif


EW_ENTER


inline void String::_do_append(const char* p1,uintptr_t n)
{
	char* _pnewstr=StringDetail::str_cat(m_ptr,::strlen(m_ptr),p1,n);
	StringDetail::str_free(m_ptr);
	m_ptr=_pnewstr;
}

inline void String::_do_append(const wchar_t* p1,uintptr_t n)
{
	StringBuffer<char> sb;
	IConv::unicode_to_ansi(sb,p1,n);
	_do_append(sb.data(),sb.size());
}

inline void String::_do_assign(const char* p1,uintptr_t n)
{
	char* _pnewstr=StringDetail::str_dup(p1,n);
	StringDetail::str_free(m_ptr);
	m_ptr=_pnewstr;
}

inline void String::_do_assign(const wchar_t* p1,uintptr_t n)
{
	StringBuffer<char> sb;
	IConv::unicode_to_utf8(sb,p1,n);

	_do_assign(sb.data(),sb.size());
}


template<typename T>
inline void String::_do_format_integer(T v_)
{
	char buf[32];

	char* p2=buf+31;
	char* p1=StringDetail::str_format(p2,v_);

	_do_append(p1,p2-p1);
}


inline int String::_do_prinfv(const char* s,va_list vl)
{
	int _n1 = vsnprintf(NULL, 0, s, vl);
	if (_n1 < 0)
	{
		return -1;
	}

	char* _pnewstr = StringDetail::str_alloc(_n1);
	int _n2 = vsnprintf(_pnewstr, _n1, s, vl);

	EW_ASSERT(_n1 == _n2);

	StringDetail::str_free(m_ptr);
	m_ptr = _pnewstr;

	return _n2;

}


String::String(const StringBuffer<char>& o)
{
	m_ptr=StringDetail::str_dup(o.data(),o.size());
}
String::String(const StringBuffer<unsigned char>& o)
{
	m_ptr=StringDetail::str_dup((char*)o.data(),o.size());
}
String::String(const StringBuffer<wchar_t>& o)
{
	m_ptr=StringDetail::str_empty();
	_do_assign(o.c_str(),o.size());	
}

String::String(const unsigned char* p)
{
	m_ptr=StringDetail::str_dup((char*)p);
}

String::String(const unsigned char* p1,uint32_t n)
{
	m_ptr=StringDetail::str_dup((char*)p1,n);
}

String::String(const unsigned char* p1,unsigned const char* p2)
{
	m_ptr=StringDetail::str_dup((char*)p1,p2-p1);
}

void String::assign(const char* p1,uint32_t n)
{
	_do_assign(p1,n);
}

void String::assign(const char* p1,const char* p2)
{
	_do_assign(p1,p2-p1);
}

void String::assign(const unsigned char* p1,uint32_t n)
{
	_do_assign((const char*)p1,n);
}

void String::assign(const unsigned char* p1,const unsigned char* p2)
{
	assign((char*)p1,p2-p1);
}


String::String(const wchar_t* p)
{
	m_ptr=StringDetail::str_empty();
	_do_assign(p,std::char_traits<wchar_t>::length(p));
}

String::String(const wchar_t* p,uint32_t n)
{
	m_ptr=StringDetail::str_empty();
	_do_assign(p,n);
}

String::String(const wchar_t* p1,const wchar_t* p2)
{
	m_ptr=StringDetail::str_empty();
	_do_assign(p1,p2-p1);
}

size_t String::size() const
{
	return ::strlen(m_ptr);
}

size_t String::length() const
{
	return ::strlen(m_ptr);
}

String& String::PrintfV(const char* s,va_list arglist)
{
	_do_prinfv(s,arglist);
	return *this;
}

String String::FormatV(const char* s,va_list arglist)
{
	String ret;
	ret._do_prinfv(s,arglist);
	return ret;
}

String String::FormatImpl(const char* s,...)
{
	va_list arglist;
	String ret;
	va_start(arglist,s);
	ret._do_prinfv(s,arglist);
	va_end(arglist);
	return ret;
}


void String::append(const char* p,uint32_t n)
{
	_do_append(p,n);
}

void String::append(const char* p1,const char* p2)
{
	_do_append(p1,p2-p1);
}


String& String::operator=(const char* p)
{
	_do_assign(p,::strlen(p));
	return *this;
}

String& String::operator=(const String& p)
{
	_do_assign(p.c_str(),p.size());
	return *this;
}

String& String::operator=(const wchar_t* p)
{
	_do_assign(p,std::char_traits<wchar_t>::length(p));
	return *this;
}

String& String::operator+=(const char* p)
{
	_do_append(p,::strlen(p));
	return *this;
}

String& String::operator+=(const String& p)
{
	_do_append(p.c_str(),p.size());
	return *this;
}

String& String::operator+=(const wchar_t* p)
{
	_do_append(p,std::char_traits<wchar_t>::length(p));
	return *this;

}

bool String::ToNumber(int64_t* val) const
{
	if(!val) return false;
	int n=sscanf(m_ptr, "%lld", val);
	return n>0;
}

bool String::ToNumber(int32_t* val) const
{
	if(!val) return false;
	int n=sscanf(m_ptr, "%d", val);
	return n>0;
}

bool String::ToNumber(float32_t* val) const
{
	if(!val) return false;
	double tmp(*val);
	int n=sscanf(m_ptr, "%lf", &tmp);
	*val = tmp;
	return n>0;
}

bool String::ToNumber(float64_t* val) const
{
	if(!val) return false;
	int n=sscanf(m_ptr, "%lf", val);
	return n>0;
}


std::ostream& operator<<(std::ostream&o,const String& s)
{
	std::string stds(s.c_str());
	o<<stds;
	return o;
}

std::istream& operator>>(std::istream&o,String& s)
{
	std::string tmp;
	o>>tmp;
	s=tmp.c_str();
	return o;
}

String operator+(const String& lhs,const String& rhs)
{
	String tmp(lhs);
	tmp+=rhs;
	return tmp;
}

String operator+(const char* lhs,const String& rhs)
{
	String tmp(lhs);
	tmp+=rhs;
	return tmp;
}

String operator+(const String& lhs,const char* rhs)
{
	String tmp(lhs);
	tmp+=rhs;
	return tmp;
}

String& String::operator<<(bool v)
{
	if(v)
	{
		_do_append("true",4);
	}
	else
	{
		_do_append("false",5);
	}

	return *this;
}

String& String::operator<<(char v)
{
	_do_append(&v,1);
	return *this;
}

String& String::operator<<(int32_t v)
{
	_do_format_integer(v);
	return *this;
}

String& String::operator<<(int64_t v)
{
	_do_format_integer(v);
	return *this;
}

String& String::operator<<(uint32_t v)
{
	_do_format_integer(v);
	return *this;
}

String& String::operator<<(uint64_t v)
{
	_do_format_integer(v);
	return *this;
}

String& String::operator<<(float v)
{
	(*this)+=String::Format("%g",v);
	return *this;
}

String& String::operator<<(double v)
{
	(*this)+=String::Format("%g",v);
	return *this;
}

String& String::operator<<(const char* v)
{
	append(v,::strlen(v));
	return *this;
}

String& String::operator<<(const wchar_t* p)
{
	_do_append(p,std::char_traits<wchar_t>::length(p));
	return *this;
}

String& String::operator<<(const String& v)
{
	_do_append(v.c_str(),v.size());
	return *this;
}

String& String::operator<<(const StringBuffer<char>& o)
{
	_do_append(o.data(),o.size());
	return *this;
}

String& String::operator<<(const StringBuffer<unsigned char>& o)
{
	_do_append((char*)o.data(),o.size());
	return *this;
}

String String::substr(size_t pos,size_t len) const
{
	size_t n = size();
	if(pos>=n) return "";
	return String(m_ptr+pos,std::min(len,n-pos));
}

int String::find (char c, int pos) const
{
	size_t n=size();
	for(size_t i=pos; i<n; i++)
	{
		if(m_ptr[i]==c) return i;
	}
	return -1;
}


int String::find (const String& s, int pos) const
{
	size_t n=size();
	if (unsigned(pos) >= n) return -1;
	const char* p = ::strstr(m_ptr + pos, s.c_str());	
	return p == NULL ? -1 : p - m_ptr;
}

int String::replace(char c1,char c2)
{
	int n=0;
	StringBuffer<char> sb(*this);
	for(size_t i=0; i<sb.size(); i++)
	{
		if(sb[i]==c1)
		{
			sb[i]=c2;
			n++;
		}
	}

	(*this)=sb;
	return n;
}

int String::replace(const String& c1,const String& c2)
{
	int n=0;

	StringBuffer<char> sb;

	const char* _pOld=c1.c_str();
	size_t _nOld=c1.size();

	const char* _pNew=c2.c_str();
	size_t _nNew=c2.size();

	const char* p1=m_ptr;
	const char* p2=m_ptr+size();
	for(;;)
	{
		const char* pt=::strstr(p1,_pOld);
		if(pt==NULL)
		{
			sb.append(p1,p2);
			break;
		}
		sb.append(p1,pt);
		sb.append(_pNew,_nNew);
		p1=pt+_nOld;
		n++;
	}
	(*this)=sb;
	return n;
}



EW_LEAVE
