
#include "string_impl.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/basic/codecvt.h"
#include <string>

#include <stdio.h>
#include <iostream>

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

inline int String::_do_prinfv(const char* format,va_list argptr)
{
	va_list argptr_copy;
	va_copy(argptr_copy,argptr);
	int n1=vsnprintf(0,0,format,argptr_copy);
	va_end(argptr_copy);

	if(n1<0)
	{
		System::LogTrace("n1!<0 in String::_do_printfv.");
		return -1;
	}

	char* pbuf=StringDetail::str_alloc(n1);
	int n2=vsnprintf(pbuf,n1+1,format,argptr);
	StringDetail::str_free(m_ptr);
	m_ptr=pbuf;

	if(n1!=n2)
	{
		System::LogTrace("n1!=n2 in String::_do_printfv. n1=%d,n2=%d",n1,n2);
	}

	return n1;

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

	String ret;

	va_list arglist;
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

template<template<unsigned> class H>
bool string_read_uint_t(const char* &p,int64_t& v,uint32_t h)
{
	unsigned char w;
	for(v=0;(w=lookup_table<H>::test(p[0]))!=0xFF;p++)
	{
		v=v*h+w;
	}
	return true;
}

bool string_read_uint(const char* &p,int64_t& v)
{
	v=0;

	char ch=p[0];
	if(ch=='0')
	{
		ch=lookup_table<lkt2uppercase>::test(*++p);
		if(ch=='X')
		{
			return string_read_uint_t<lkt_number16b>(++p,v,16);
		}
		else if(ch=='B')
		{
			return string_read_uint_t<lkt_number02b>(++p,v,2);		
		}
		else if(ch>='1'&&ch<='9')
		{
			return string_read_uint_t<lkt_number08b>(++p,v,8);		
		}
		else if(ch=='0')
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return string_read_uint_t<lkt_number10b>(p,v,10);
	}
}

bool Scanner_read_sign(const char* &p)
{
	if(p[0]=='-')
	{
		p++;
		return true;
	}
	else if(p[0]=='+')
	{
		p++;
	}
	return false;
}

bool Scanner_read_number(const char* p,double& v)
{

	bool sign=Scanner_read_sign(p);

	int64_t ipart(0);

	if(p[0]=='0')
	{
		if(p[1]!='.')
		{
			if(string_read_uint(p,ipart))
			{
				v=sign?-double(ipart):double(ipart);
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	else if(p[0]!='.')
	{
		if(!string_read_uint_t<lkt_number10b>(p,ipart,10))
		{
			return false;
		}		
	}

	double dpart(ipart);

	if (p[0] == '.')
	{
		if(*++p=='#')
		{
			if(p[1]=='I'&&p[2]=='N'&&p[3]=='F')
			{
				v=sign?-numeric_trait_floating<double>::max_value():numeric_trait_floating<double>::max_value();
				return true;
			}
			if((p[1]=='I'&&p[2]=='N'&&p[3]=='D')||(p[1]=='N'&&p[2]=='A'&&p[3]=='N')||(p[1]!=0 && p[2]=='N'&&p[3]=='A'))
			{
				v=sign?-numeric_trait_floating<double>::nan_value():numeric_trait_floating<double>::nan_value();
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			int64_t v2(0);
			const char* p0=p;
			string_read_uint_t<lkt_number10b>(p,v2,10);
			dpart+=double(v2)*::pow(0.1,p-p0);
 		}
	}


	if(p[0]=='e'||p[0]=='E')
	{
		bool sign2=Scanner_read_sign(++p);
		int64_t v3(0);
		string_read_uint_t<lkt_number10b>(p,v3,10);
		dpart*=::pow(10.0,v3);
	}
	
	v= sign?-dpart:dpart;
	return true;
	
}

bool String::ToNumber(int64_t* val) const
{
	if(!val) return false;
	const char* p=m_ptr;
	bool sign=Scanner_read_sign(p);

	int64_t v(0);
	const char* p0=p;
	if(!string_read_uint(p,v))
	{
		return false;
	}
	if(p==p0)
	{
		return false;
	}

	*val=sign?-v:v;
	return true;

}

bool String::ToNumber(int32_t* val) const
{
	if(!val) return false;
	int64_t tmp;
	if(!ToNumber(&tmp))
	{
		return false;
	}

	if(tmp>(int64_t)numeric_trait<int32_t>::max_value())
	{
		tmp=numeric_trait<int32_t>::max_value();
	}
	else if(tmp<(int64_t)numeric_trait<int32_t>::min_value())
	{
		tmp=numeric_trait<int32_t>::min_value();
	}

	*val=tmp;
	return true;
}

bool String::ToNumber(float32_t* val) const
{
	if(!val) return false;
	const char* p=m_ptr;
	double tmp;
	if(!Scanner_read_number(p,tmp))
	{
		return false;
	}
	*val=tmp;
	return true;

}


bool String::ToNumber(float64_t* val) const
{
	if(!val) return false;
	const char* p=m_ptr;
	return Scanner_read_number(p,*val);
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
