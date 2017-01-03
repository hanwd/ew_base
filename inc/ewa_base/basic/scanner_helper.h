#ifndef __H_EW_BASIC_SCANNER_BASE__
#define __H_EW_BASIC_SCANNER_BASE__

#include "ewa_base/config.h"
#include "ewa_base/basic/exception.h"
#include "ewa_base/basic/lookuptable.h"

EW_ENTER


template<typename T>
class char_pos_t
{
public:

	typedef T type;
	typedef const T* pointer;

	char_pos_t(pointer p = NULL){ reset(p); }

	void advance()
	{
		if (pcur[0] == '\n')
		{
			++line;
			cpos = 1;
		}
		else
		{
			++cpos;
		}
		++pcur;
	}

	void advance(int n)
	{
		if (n < 0)
		{
			Exception::XError("invalid operation");
		}

		for (int i = 0; i < n; i++)
		{
			advance();
		}
	}

	void reset(pointer p){pcur = p;line = 1;cpos = 1;}

	char_pos_t& operator=(pointer v){ reset(v); return *this; }
	char_pos_t& operator++(){ advance(); return *this; }
	char_pos_t& operator+=(int n){ advance(n); return *this; }
	pointer operator++(int){ pointer p = pcur; advance(); return p; }

	operator pointer(){ return pcur; }

	size_t line;
	size_t cpos;
	pointer pcur;
};



template<typename B>
class ScannerHelperBase
{
public:
	typedef typename B::type type;
	typedef typename B::pointer pointer;
};

template<typename T>
class ScannerHelperBase<const T*>
{
public:
	typedef T type;
	typedef const T* pointer;
};


template<typename B>
class DLLIMPEXP_EWA_BASE ScannerHelper : public ScannerHelperBase<B>
{
public:

	typedef ScannerHelperBase<B> basetype;
	typedef typename basetype::type type;
	typedef typename basetype::pointer pointer;

	template<template<unsigned> class H, unsigned U>
	class rebind
	{
	public:
		typedef typename tl::meta_if<sizeof(type) == 1, lookup_table<H>, lookup_table_u<H, U> >::type type;
	};


	template<template<unsigned> class H,unsigned U=0> 
	static inline void skip(B& p)
	{
		typedef typename rebind<H, U>::type lktable;
		while (lktable::test(*p)) ++p;
	}

	template<template<unsigned> class H,unsigned N=0> 
	static inline void skip(B& p, pointer d)
	{
		typedef typename rebind<H, U>::type lktable;
		while (lookup_table<P>::test(*p) && p != d) ++p;
	}

	template<int N>
	static bool read_uint_t(B &p, int64_t& v)
	{
		typedef typename rebind<lkt_number16b,16>::type lktable;
		unsigned char w;
		for (v = 0; (w = lktable::test(p[0])) <N; ++p)
		{
			v = v*N + w;
		}
		return true;
	}

	static bool read_uint(B &p, int64_t& v);
	static bool read_sign(B &p);

	static bool read_number(B &p, double& v);
	static bool read_number(B &p, int64_t& v);

};


EW_LEAVE
#endif
