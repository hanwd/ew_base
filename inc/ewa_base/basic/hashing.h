#ifndef __H_EW_BASIC_HASHING__
#define __H_EW_BASIC_HASHING__

#include "ewa_base/config.h"

EW_ENTER


template<typename T,int N=0>
class hash_impl
{
public:
	static const int count=N;

	typedef T type1;
	typedef uintptr_t type2;

	static inline uint32_t hash(const void* p)
	{
		const type1 *pdata  = (const type1*)p;
		type2 seed1 = 0x7FED7FED;
		type2 seed2 = 0xEEEEEEEE;
		type2 seed3 = 0x3123EEEE;
		for(int i=0; i<N; i++)
		{
			type2 temp0 = (type2)*pdata++;
			seed1 = (temp0 * seed3) ^ (seed1 + seed2);
			seed2 = (temp0 + seed1) + seed2 + (seed2 << 5) + 3;
		}
		return (uint32_t)seed1;
	}

	static inline uint32_t hash(const void* p,size_t n)
	{
		const type1 *pdata=(const type1*)p;
		const type1 *p_end=pdata+n;

		type2 seed1 = 0x7FED7FED;
		type2 seed2 = 0xEEEEEEEE;
		type2 seed3 = 0x3123EEEE;

		while(pdata<p_end)
		{
			type2 temp0 = (type2)*pdata++;
			seed1 = (temp0 * seed3) ^ (seed1 + seed2);
			seed2 = (temp0 + seed1) + seed2 + (seed2 << 5) + 3;
		}
		return (uint32_t)seed1;
	}

	static inline uint32_t hash_string(const void* p)
	{
		const type1 *pdata  = (const type1*)p;
		type2 seed1 = 0x7FED7FED;
		type2 seed2 = 0xEEEEEEEE;
		type2 seed3 = 0x3123EEEE;

		while(*pdata!=0)
		{
			type2 temp0 = (type2)*pdata++;
			seed1 = (temp0 * seed3) ^ (seed1 + seed2);
			seed2 = (temp0 + seed1) + seed2 + (seed2 << 5) + 3;
		}
		return (uint32_t)seed1;
	}
};

template<int N>
class hash_raw;

template<> class hash_raw<1> : public hash_impl<uint8_t>{};
template<> class hash_raw<2> : public hash_impl<uint16_t>{};
template<> class hash_raw<4> : public hash_impl<uint32_t>{};
template<> class hash_raw<8> : public hash_impl<uint64_t>{};

template<int N,int D> class hash_base : public hash_impl<uint8_t,N>{};
template<int N> class hash_base<N,0> : public hash_impl<uint64_t,N/8>{};
template<int N> class hash_base<N,2> : public hash_impl<uint16_t,N/2>{};
template<int N> class hash_base<N,4> : public hash_impl<uint32_t,N/4>{};
template<int N> class hash_base<N,6> : public hash_impl<uint16_t,N/2>{};


template<typename T>
class hash_pod : public hash_base<sizeof(T),sizeof(T)%8>
{
public:
	typedef hash_base<sizeof(T),sizeof(T)%8> basetype;
	inline uint32_t operator()(const T& val)
	{
		return basetype::hash(&val);
	}
};

template<typename T>
class hash_origin
{
public:
	inline uint32_t operator()(const T val)
	{
		return (T)(val);
	}
};

template<> class hash_t<bool> : public hash_origin<bool> {};
template<> class hash_t<int8_t> : public hash_origin<int8_t> {};
template<> class hash_t<uint8_t> : public hash_origin<uint8_t> {};

template<> class hash_t<int16_t> : public hash_pod<int16_t> {};
template<> class hash_t<uint16_t> : public hash_pod<uint16_t> {};
template<> class hash_t<int32_t> : public hash_pod<int32_t> {};
template<> class hash_t<uint32_t> : public hash_pod<uint32_t> {};
template<> class hash_t<float32_t> : public hash_pod<float32_t> {};
template<> class hash_t<wchar_t> : public hash_pod<wchar_t> {};

template<> class hash_t<int64_t> : public hash_pod<int64_t> {};
template<> class hash_t<uint64_t> : public hash_pod<uint64_t> {};
template<> class hash_t<float64_t> : public hash_pod<float64_t> {};
template<> class hash_t<void*> : public hash_pod<void*> {};

template<typename T> class hash_t<const T> : public hash_t<T>{};

template<typename T> class hash_t<T*>
{
public:
	inline uint32_t operator()(T* val)
	{
		return (uintptr_t(val))>>4;
	}
};


template<typename T>
class hash_array
{
public:

	static uint32_t hash(const T* p,size_t n)
	{
		if(tl::is_pod<T>::value)
		{
			return hash_pod<T>::hash(p,n*hash_pod<T>::count);
		}
		else
		{
			hash_t<T> h;uint32_t v=0;
			for(size_t i=0;i<n;i++)
			{
				v^=h(p[i]);v=(v<<2)+(v>>2)*0x87654321;
			}
			return v;
		}
	}

	template<typename IT>
	static uint32_t hash(IT v1,IT v2)
	{
		hash_t<T> h;uint32_t v=0;
		for(IT it=v1;it!=v2;++it)
		{
			v^=h(*it);v=(v<<2)+(v>>2)*0x87654321;
		}
		return v;
	}

};



DLLIMPEXP_EWA_BASE uint32_t crc32(const void* p,size_t n);

class DLLIMPEXP_EWA_BASE Crc32Gen
{
public:
	Crc32Gen();

	void update(const void* p,size_t n);
	uint32_t done(){uint32_t t=~crc;crc=-1;return t;}

private:
	uint32_t crc;
};

EW_LEAVE
#endif
