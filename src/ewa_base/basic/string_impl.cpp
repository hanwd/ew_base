#include "ewa_base/config.h"
#include "ewa_base/basic/formater.h"

EW_ENTER

const char const_empty_buffer[64]= {0};

WStringProxy::WStringProxy():m_ptr(const_empty_buffer),m_size(0)
{
}

WStringProxy::WStringProxy(const WStringProxy& o):m_ptr(const_empty_buffer),m_size(0)
{
	*this=o;
}

WStringProxy& WStringProxy::operator=(const WStringProxy& o)
{
	m_ptr=StringDetail::str_dup(o.m_ptr);
	m_size=o.m_size;
	return *this;
}

WStringProxy::WStringProxy(const wchar_t* p):m_ptr(const_empty_buffer),m_size(0)
{
	reset(p);
}

WStringProxy::WStringProxy(const wchar_t* p,size_t n):m_ptr(const_empty_buffer),m_size(0)
{
	reset(p,n);
}

void WStringProxy::reset(const wchar_t* p,size_t n)
{
	StringBuffer<char> sb;
	IConv::unicode_to_utf8(sb,p,n);
	m_ptr=StringDetail::str_dup(sb.c_str());
	m_size=sb.size();
}

void WStringProxy::reset(const wchar_t* p)
{
	reset(p,std::char_traits<wchar_t>::length(p));
}

WStringProxy::~WStringProxy()
{
	StringDetail::str_free(m_ptr);
}

EW_LEAVE
