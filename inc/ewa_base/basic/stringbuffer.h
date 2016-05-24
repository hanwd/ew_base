#ifndef __H_EW_STRINGBUFFER__
#define __H_EW_STRINGBUFFER__

#include "ewa_base/config.h"
#include "ewa_base/basic/hashing.h"
#include "ewa_base/basic/string_detail.h"
#include "ewa_base/collection/arr_1t.h"
#include "ewa_base/memory/allocator.h"

#ifdef _MSC_VER
#pragma warning(disable:4307)
#endif

EW_ENTER

enum
{
	FILE_BINARY,
	FILE_TEXT,
	FILE_TEXT_ANSI,
	FILE_TEXT_UTF8,
	FILE_TEXT_UTF16_BE,
	FILE_TEXT_UTF16_LE,

	FILE_MASK=(1<<10)-1,
	FILE_TEXT_BOM=1<<10
};


template<typename T>
class DLLIMPEXP_EWA_BASE StringBuffer : public arr_1t<T,AllocatorN<def_allocator,1> >
{
public:
	typedef arr_1t<T,AllocatorN<def_allocator,1> > basetype;
	typedef typename basetype::size_type size_type;

	using basetype::assign;
	using basetype::resize;
	using basetype::data;
	using basetype::size;

	StringBuffer() {}

	StringBuffer(const T* p1);
	StringBuffer(const T* p1,size_t ln)
	{
		assign(p1,ln);
	}
	StringBuffer(const T* p1,const T* p2)
	{
		assign(p1,p2);
	}
	StringBuffer(const StringBuffer& o):basetype(o) {}

#if defined(EW_C11)
	StringBuffer(StringBuffer&& p)
	{
		swap(p);
	}
	const StringBuffer& operator=(StringBuffer&& p)
	{
		swap(p);
		return *this;
	}
#endif

	StringBuffer(const String& o)
	{
		(*this)=o;
	}

	StringBuffer& operator=(const String& o);
	StringBuffer& operator=(const T* p1);

	StringBuffer& operator+=(const StringBuffer& o)
	{
		this->append(o.begin(),o.end());
		return *this;
	}
	StringBuffer& operator+=(const String& o);

	bool load(const String& file,int t=FILE_TEXT);
	bool save(const String& file,int t=FILE_TEXT);

	T* c_str();
	const T* c_str() const;

	StringBuffer& operator<<(bool v);
	StringBuffer& operator<<(char v);
	StringBuffer& operator<<(int32_t v);
	StringBuffer& operator<<(int64_t v);
	StringBuffer& operator<<(uint32_t v);
	StringBuffer& operator<<(uint64_t v);
	StringBuffer& operator<<(float v);
	StringBuffer& operator<<(double v);
	StringBuffer& operator<<(const T* v);
	StringBuffer& operator<<(const void* v);
	StringBuffer& operator<<(const String& v);
	StringBuffer& operator<<(const StringBuffer& v);


	StringBuffer& operator<<(const std::basic_string<T>& v)
	{
		return (*this)<<v.c_str();
	}

protected:

	template<typename G>
	void _do_format_integer(G v);
};

DEFINE_OBJECT_NAME_T(StringBuffer,"Buffer");

template<typename T> class hash_t<StringBuffer<T> >
{
public:
	inline uint32_t operator()(const StringBuffer<T>& o)
	{
		return hash_array<T>::hash(o.data(),o.size());
	}
};

inline const char* StringParamCast::g(const StringBuffer<char>& v)
{
	return v.c_str();
}

template<template<unsigned> class P,typename T=const unsigned char>
class lookup_table
{
public:
	static inline T test(unsigned char text)
	{
		return cmap[text];
	}
	inline T operator[](unsigned char text) const
	{
		return cmap[text];
	}
	inline T operator()(unsigned char text) const
	{
		return cmap[text];
	}
	static T cmap[256];
};

template<unsigned N>
class lkt2uppercase
{
public:
	static const unsigned value=(N>='a' && N<='z')?(N+'A'-'a'):N;
};

template<unsigned N>
class lkt2lowercase
{
public:
	static const unsigned value = (N >= 'A'&&N <= 'Z') ? (N - 'A' + 'a') : N;
};

template<unsigned N>
class lkt_whitespace
{
public:
	static const unsigned value = N == ' ' || N == '\t' || N == '\r' || N == '\n';
};


template<unsigned N>
class lkt_number16b
{
public:
	static const unsigned char value1 = (N >= '0'&&N <= '9') ? (N - '0') : 0xFF;
	static const unsigned char value2 = (N >= 'A'&&N <= 'Z') ? (N + 10 - 'A') : value1;
	static const unsigned char value3 = (N >= 'a'&&N <= 'z') ? (N + 10 - 'a') : value2;
	static const unsigned char value = value3;
};

template<unsigned N>
class lkt_number10b
{
public:
	static const unsigned char value = (N >= '0'&&N <= '9') ? (N - '0') : 0xFF;
};

template<unsigned N>
class lkt_not_newline
{
public:
	static const int value = N!=0||N!='\n';
};

template<template<unsigned> class P,typename T>
T lookup_table<P,T>::cmap[256]=
{
	P<0x00>::value,P<0x01>::value,P<0x02>::value,P<0x03>::value,
	P<0x04>::value,P<0x05>::value,P<0x06>::value,P<0x07>::value,
	P<0x08>::value,P<0x09>::value,P<0x0A>::value,P<0x0B>::value,
	P<0x0C>::value,P<0x0D>::value,P<0x0E>::value,P<0x0F>::value,
	P<0x10>::value,P<0x11>::value,P<0x12>::value,P<0x13>::value,
	P<0x14>::value,P<0x15>::value,P<0x16>::value,P<0x17>::value,
	P<0x18>::value,P<0x19>::value,P<0x1A>::value,P<0x1B>::value,
	P<0x1C>::value,P<0x1D>::value,P<0x1E>::value,P<0x1F>::value,
	P<0x20>::value,P<0x21>::value,P<0x22>::value,P<0x23>::value,
	P<0x24>::value,P<0x25>::value,P<0x26>::value,P<0x27>::value,
	P<0x28>::value,P<0x29>::value,P<0x2A>::value,P<0x2B>::value,
	P<0x2C>::value,P<0x2D>::value,P<0x2E>::value,P<0x2F>::value,
	P<0x30>::value,P<0x31>::value,P<0x32>::value,P<0x33>::value,
	P<0x34>::value,P<0x35>::value,P<0x36>::value,P<0x37>::value,
	P<0x38>::value,P<0x39>::value,P<0x3A>::value,P<0x3B>::value,
	P<0x3C>::value,P<0x3D>::value,P<0x3E>::value,P<0x3F>::value,
	P<0x40>::value,P<0x41>::value,P<0x42>::value,P<0x43>::value,
	P<0x44>::value,P<0x45>::value,P<0x46>::value,P<0x47>::value,
	P<0x48>::value,P<0x49>::value,P<0x4A>::value,P<0x4B>::value,
	P<0x4C>::value,P<0x4D>::value,P<0x4E>::value,P<0x4F>::value,
	P<0x50>::value,P<0x51>::value,P<0x52>::value,P<0x53>::value,
	P<0x54>::value,P<0x55>::value,P<0x56>::value,P<0x57>::value,
	P<0x58>::value,P<0x59>::value,P<0x5A>::value,P<0x5B>::value,
	P<0x5C>::value,P<0x5D>::value,P<0x5E>::value,P<0x5F>::value,
	P<0x60>::value,P<0x61>::value,P<0x62>::value,P<0x63>::value,
	P<0x64>::value,P<0x65>::value,P<0x66>::value,P<0x67>::value,
	P<0x68>::value,P<0x69>::value,P<0x6A>::value,P<0x6B>::value,
	P<0x6C>::value,P<0x6D>::value,P<0x6E>::value,P<0x6F>::value,
	P<0x70>::value,P<0x71>::value,P<0x72>::value,P<0x73>::value,
	P<0x74>::value,P<0x75>::value,P<0x76>::value,P<0x77>::value,
	P<0x78>::value,P<0x79>::value,P<0x7A>::value,P<0x7B>::value,
	P<0x7C>::value,P<0x7D>::value,P<0x7E>::value,P<0x7F>::value,
	P<0x80>::value,P<0x81>::value,P<0x82>::value,P<0x83>::value,
	P<0x84>::value,P<0x85>::value,P<0x86>::value,P<0x87>::value,
	P<0x88>::value,P<0x89>::value,P<0x8A>::value,P<0x8B>::value,
	P<0x8C>::value,P<0x8D>::value,P<0x8E>::value,P<0x8F>::value,
	P<0x90>::value,P<0x91>::value,P<0x92>::value,P<0x93>::value,
	P<0x94>::value,P<0x95>::value,P<0x96>::value,P<0x97>::value,
	P<0x98>::value,P<0x99>::value,P<0x9A>::value,P<0x9B>::value,
	P<0x9C>::value,P<0x9D>::value,P<0x9E>::value,P<0x9F>::value,
	P<0xA0>::value,P<0xA1>::value,P<0xA2>::value,P<0xA3>::value,
	P<0xA4>::value,P<0xA5>::value,P<0xA6>::value,P<0xA7>::value,
	P<0xA8>::value,P<0xA9>::value,P<0xAA>::value,P<0xAB>::value,
	P<0xAC>::value,P<0xAD>::value,P<0xAE>::value,P<0xAF>::value,
	P<0xB0>::value,P<0xB1>::value,P<0xB2>::value,P<0xB3>::value,
	P<0xB4>::value,P<0xB5>::value,P<0xB6>::value,P<0xB7>::value,
	P<0xB8>::value,P<0xB9>::value,P<0xBA>::value,P<0xBB>::value,
	P<0xBC>::value,P<0xBD>::value,P<0xBE>::value,P<0xBF>::value,
	P<0xC0>::value,P<0xC1>::value,P<0xC2>::value,P<0xC3>::value,
	P<0xC4>::value,P<0xC5>::value,P<0xC6>::value,P<0xC7>::value,
	P<0xC8>::value,P<0xC9>::value,P<0xCA>::value,P<0xCB>::value,
	P<0xCC>::value,P<0xCD>::value,P<0xCE>::value,P<0xCF>::value,
	P<0xD0>::value,P<0xD1>::value,P<0xD2>::value,P<0xD3>::value,
	P<0xD4>::value,P<0xD5>::value,P<0xD6>::value,P<0xD7>::value,
	P<0xD8>::value,P<0xD9>::value,P<0xDA>::value,P<0xDB>::value,
	P<0xDC>::value,P<0xDD>::value,P<0xDE>::value,P<0xDF>::value,
	P<0xE0>::value,P<0xE1>::value,P<0xE2>::value,P<0xE3>::value,
	P<0xE4>::value,P<0xE5>::value,P<0xE6>::value,P<0xE7>::value,
	P<0xE8>::value,P<0xE9>::value,P<0xEA>::value,P<0xEB>::value,
	P<0xEC>::value,P<0xED>::value,P<0xEE>::value,P<0xEF>::value,
	P<0xF0>::value,P<0xF1>::value,P<0xF2>::value,P<0xF3>::value,
	P<0xF4>::value,P<0xF5>::value,P<0xF6>::value,P<0xF7>::value,
	P<0xF8>::value,P<0xF9>::value,P<0xFA>::value,P<0xFB>::value,
	P<0xFC>::value,P<0xFD>::value,P<0xFE>::value,P<0xFF>::value
};


class DLLIMPEXP_EWA_BASE ParserBase : public Object
{
public:

	typedef char mychar;
	typedef const char* mychar_ptr;

	template<template<unsigned> class P> static inline void skip(mychar_ptr& p)
	{
		mychar_ptr tmp=p;
		while(lookup_table<P>::test(*tmp)) ++tmp;
		p=tmp;
	}
	template<template<unsigned> class P> static inline void skip(mychar_ptr& p,mychar_ptr d)
	{
		mychar_ptr tmp=p;
		while(lookup_table<P>::test(*tmp) && p!=d) ++tmp;
		p=tmp;
	}


};

EW_LEAVE
#endif
