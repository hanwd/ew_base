#ifndef __H_EW_STRING__
#define __H_EW_STRING__

#include "ewa_base/config.h"
#include "ewa_base/basic/hashing.h"
#include "ewa_base/basic/string_detail.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/formater.h"

EW_ENTER

template<typename T>
class DLLIMPEXP_EWA_BASE StringBuffer;


class DLLIMPEXP_EWA_BASE String : public FormatHelper<String>
{
public:

	String();
	String(const String& o);

	String(const StringBuffer<char>& o);
	String(const StringBuffer<unsigned char>& o);
	String(const StringBuffer<wchar_t>& o);

	String(const char* p);
	String(const char* p,size_t n);
	String(const char* p1,const char* p2);

	String(const unsigned char* p);
	String(const unsigned char* p,size_t n);
	String(const unsigned char* p1,unsigned const char* p2);

	String(const wchar_t* p);
	String(const wchar_t* p,size_t n);
	String(const wchar_t* p1,const wchar_t* p2);

	void assign(const char* p,size_t n);
	void assign(const char* p1,const char* p2);

	void assign(const unsigned char* p,size_t n);
	void assign(const unsigned char* p1,const unsigned char* p2);

	void append(const char* p,size_t n);
	void append(const char* p1,const char* p2);
	void append(const wchar_t* p,size_t n);
	void append(const wchar_t* p1,const wchar_t* p2);

	template<typename T>
	String(const std::basic_string<T>& o);

#if defined(EW_C11)
	String(String&& p){m_ptr=p.m_ptr;p.m_ptr=const_empty_buffer;}
	String& operator=(String&& p){this->swap(p);return *this;}
#endif

	~String();

	void swap(String& s1);

	const char* c_str() const;
	size_t size() const;
	size_t length() const;

	bool empty() const;

	String& operator=(const String& p);
	String& operator=(const char* p);
	String& operator=(const wchar_t* p);

	template<typename T>
	String& operator=(const std::basic_string<T>& o)
	{
		return (*this)=o.c_str();
	}

	String& operator+=(const String& p);
	String& operator+=(const char* p);
	String& operator+=(const wchar_t* p);

	template<typename T>
	String& operator+=(const std::basic_string<T>& o)
	{
		return (*this)+=o.c_str();
	}


	friend DLLIMPEXP_EWA_BASE std::ostream& operator<<(std::ostream&o,const String& s);
	friend DLLIMPEXP_EWA_BASE std::istream& operator>>(std::istream&o,String& s);

	bool ToNumber(int64_t* val) const;
	bool ToNumber(int32_t* val) const;
	bool ToNumber(float32_t* val) const;
	bool ToNumber(float64_t* val) const;

	static const size_t npos=(size_t)(-1);

	String substr(size_t pos,size_t len=npos) const;

	int replace(char c1,char c2);
	int replace(const String& s1,const String& c2);

	int find(char c, int pos = 0) const;
	int find(const String& s, int pos = 0) const;


	STRING_FORMATER_FORMAT_FUNCS_SB(static String Format,return fb;)
	STRING_FORMATER_FORMAT_FUNCS_SB(String Printf,*this=fb;return *this;)

private:

	static String FormatImpl(const char* s,...);

	const char* m_ptr;

	void _do_append(const char* p1,size_t n);
	void _do_append(const wchar_t* p1,size_t n);
	void _do_assign(const char* p1,size_t n);
	void _do_assign(const wchar_t* p1,size_t n);

};


inline const char* String::c_str() const
{
	return m_ptr;
}

inline bool String::empty() const
{
	return m_ptr[0]=='\0';
}

inline String::~String()
{
	StringDetail::str_free(m_ptr);
}

inline String::String()
{
	m_ptr=StringDetail::str_empty();
}

inline String::String(const String& o)
{
	m_ptr=StringDetail::str_dup(o.m_ptr);
}

inline String::String(const char* p)
{
	m_ptr=StringDetail::str_dup(p);
}

inline String::String(const char* p1,size_t n)
{
	m_ptr=StringDetail::str_dup(p1,n);
}

inline String::String(const char* p1,const char* p2)
{
	m_ptr=StringDetail::str_dup(p1,p2-p1);
}

inline void String::swap(String& s1)
{
	std::swap(m_ptr,s1.m_ptr);
}

DLLIMPEXP_EWA_BASE std::ostream& operator<<(std::ostream&o,const String& s);
DLLIMPEXP_EWA_BASE std::istream& operator>>(std::istream&o,String& s);

template<typename T>
String::String(const std::basic_string<T>& o)
{
	m_ptr=StringDetail::str_dup(o.c_str());
}

String DLLIMPEXP_EWA_BASE operator+(const String& lhs,const String& rhs);
String DLLIMPEXP_EWA_BASE operator+(const char* lhs,const String& rhs);
String DLLIMPEXP_EWA_BASE operator+(const String& lhs,const char* rhs);

template<typename T>
String operator+(const std::basic_string<T>& lhs,const String& rhs)
{
	String ret(lhs.c_str());
	ret+=rhs;
	return ret;
}

template<typename T>
String operator+(const String& lhs,const std::basic_string<T>& rhs)
{
	String ret(lhs);
	ret+=rhs.c_str();
	return ret;
}

#define STRING_REL_OP2(X)	\
	inline bool operator X (const String& lhs,const String& rhs){return ::strcmp(FormatPolicy::cast(lhs),FormatPolicy::cast(rhs)) X 0;}\
	inline bool operator X (const char* lhs,const String& rhs){return ::strcmp(FormatPolicy::cast(lhs),FormatPolicy::cast(rhs)) X 0;}\
	inline bool operator X (const String& lhs,const char* rhs){return ::strcmp(FormatPolicy::cast(lhs),FormatPolicy::cast(rhs)) X 0;}\

STRING_REL_OP2(==)
STRING_REL_OP2(!=)
STRING_REL_OP2(>=)
STRING_REL_OP2(<=)
STRING_REL_OP2(<)
STRING_REL_OP2(>)

template<> class hash_t<char*>
{
public:
	inline uint32_t operator()(const char* v)
	{
		return hash_raw<1>::hash_string(v);
	}
};


template<> class hash_t<String>
{
public:
	inline uint32_t operator()(const String& v)
	{
		return hash_raw<1>::hash_string(v.c_str());
	}
};


inline const char* FormatPolicy::cast(const String& v)
{
	return v.c_str();
}



EW_LEAVE


#endif
