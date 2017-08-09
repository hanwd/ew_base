#include "ewa_base/config.h"
#include "ewa_base/basic/formater.h"
#include "ewa_base/basic/codecvt.h"
#include "ewa_base/scripting/variant_op.h"

EW_ENTER

const char const_empty_buffer[64]= {0};

StringProxy::StringProxy():m_ptr(const_empty_buffer),m_size(0)
{
}

StringProxy::StringProxy(const StringProxy& o):m_ptr(const_empty_buffer),m_size(0)
{
	*this=o;
}

StringProxy& StringProxy::operator=(const StringProxy& o)
{
	m_ptr=StringDetail::str_dup(o.m_ptr);
	m_size=o.m_size;
	return *this;
}

StringProxy::StringProxy(const wchar_t* p):m_ptr(const_empty_buffer),m_size(0)
{
	reset(p);
}

StringProxy::StringProxy(const wchar_t* p,size_t n):m_ptr(const_empty_buffer),m_size(0)
{
	reset(p,n);
}

StringProxy::StringProxy(const Variant& v):m_ptr(const_empty_buffer),m_size(0)
{
	reset(v);
}

void StringProxy::reset(const wchar_t* p,size_t n)
{
	StringBuffer<char> sb;
	IConv::unicode_to_utf8(sb,p,n);
	m_ptr=StringDetail::str_dup(sb.c_str());
	m_size=sb.size();
}

void StringProxy::reset(const wchar_t* p)
{
	reset(p,std::char_traits<wchar_t>::length(p));
}

void StringProxy::reset(const Variant& v,const char*)
{
	String s=variant_cast<String>(v);
	m_ptr=StringDetail::str_dup(s.c_str());	
	m_size=s.size();
}

StringProxy::~StringProxy()
{
	StringDetail::str_free(m_ptr);
}

size_t FormatPolicy::width(const Variant& o)
{
	return std::max((size_t)128,variant_cast<String>(o).size());
}


int variant_snprintf(char* p,size_t n,const char* s,const Variant& v)
{
	int sz=::strlen(s);
	if(sz==0) return -1;
	char ch=s[sz-1];
	try
	{
		switch(ch)
		{
		case 'i':
		case 'd':
		case 'u':
		case 'x':
		case 'o':
		case 'p':
			return sprintf(p,s,variant_cast<int64_t>(v));
		case 'f':
		case 'g':
		case 'a':
		case 'A':
			return sprintf(p,s,variant_cast<double>(v));
		case 's':
			{
				if(sz>1&&s[-2]=='L')
				{
					String val=variant_cast<String>(v);
					StringBuffer<wchar_t> sb;
					IConv::utf8_to_unicode(sb,val.c_str(),val.size());
					return sprintf(p,s,sb.c_str());	
				}
				else
				{
					String val=variant_cast<String>(v);
					return sprintf(p,s,val.c_str());			
				}
			}
		default:
			return -1;
		}	
	}
	catch(...)
	{
		return -1;
	}

}

EW_LEAVE
