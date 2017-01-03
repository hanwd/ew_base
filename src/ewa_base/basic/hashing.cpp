#include "ewa_base/basic/hashing.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/scanner_helper.h"

EW_ENTER

template<unsigned N,unsigned M>
class crc32_table_helper
{
public:
	static const uint32_t n_1=crc32_table_helper<N,M-1>::num;
	static const uint32_t c_1=crc32_table_helper<N,M-1>::crc;

	static const uint32_t num=n_1>>1;

	static const uint32_t tag=0xEDB88320;
	static const uint32_t tmp=(c_1&1)?((c_1>>1)^tag):c_1>>1;
	static const uint32_t crc=n_1&1 ? tmp^tag : tmp;
};

template<unsigned N>
class crc32_table_helper<N,0>
{
public:
	static const uint32_t num=N;
	static const uint32_t crc=0;
};



template<unsigned N>
class crc32_table
{
public:
	static const uint32_t value=crc32_table_helper<N,8>::crc;
};

Crc32Gen::Crc32Gen(){crc=-1;}

void Crc32Gen::update(const void* p,size_t n)
{
	typedef lookup_table<crc32_table,uint32_t> t;

	uintptr_t k0=(uintptr_t)p;
	uintptr_t k3=k0+n;

	uintptr_t k1=(k0+3)&~3;
	uintptr_t k2=k3&~3;

	if(k2>k1)
	{
		for(uintptr_t k=k0;k<k1;k++)
		{
			crc=t::cmap[(crc&0xFF)^*(const unsigned char*)k]^(crc>>8);
		}

		for(uintptr_t k=k1;k<k2;k+=4)
		{
			crc ^= *(const uint32_t *)k;
			crc = t::cmap[(crc&0xFF)]^(crc>>8);
			crc = t::cmap[(crc&0xFF)]^(crc>>8);
			crc = t::cmap[(crc&0xFF)]^(crc>>8);
			crc = t::cmap[(crc&0xFF)]^(crc>>8);
		}
		for(uintptr_t k=k2;k<k3;k++)
		{
			crc=t::cmap[(crc&0xFF)^*(const unsigned char*)k]^(crc>>8);
		}
	}
	else
	{
		for(uintptr_t k=k0;k<k3;k++)
		{
			crc=t::cmap[(crc&0xFF)^*(const unsigned char*)k]^(crc>>8);
		}
	}
}

uint32_t crc32(const void* p,size_t n)
{

	typedef lookup_table<crc32_table,uint32_t> t;

	uintptr_t k0=(uintptr_t)p;
	uintptr_t k3=k0+n;

	uintptr_t k1=(k0+3)&~3;
	uintptr_t k2=k3&~3;

	uint32_t crc=-1;

	if(k2>k1)
	{
		for(uintptr_t k=k0;k<k1;k++)
		{
			crc=t::cmap[(crc&0xFF)^*(const unsigned char*)k]^(crc>>8);
		}

		for(uintptr_t k=k1;k<k2;k+=4)
		{
			crc ^= *(const uint32_t *)k;
			crc = t::cmap[(crc&0xFF)]^(crc>>8);
			crc = t::cmap[(crc&0xFF)]^(crc>>8);
			crc = t::cmap[(crc&0xFF)]^(crc>>8);
			crc = t::cmap[(crc&0xFF)]^(crc>>8);
		}
		for(uintptr_t k=k2;k<k3;k++)
		{
			crc=t::cmap[(crc&0xFF)^*(const unsigned char*)k]^(crc>>8);
		}

		return ~crc;
	}
	else
	{
		for(uintptr_t k=k0;k<k3;k++)
		{
			crc=t::cmap[(crc&0xFF)^*(const unsigned char*)k]^(crc>>8);
		}

		return ~crc;
	}
}


EW_LEAVE
