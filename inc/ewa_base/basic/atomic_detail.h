
#ifndef __H_EW_BASIC_ATOMIC_DETAIL__
#define __H_EW_BASIC_ATOMIC_DETAIL__

#include "ewa_base/config.h"


EW_ENTER


template<int N>
class DLLIMPEXP_EWA_BASE AtomicHelper;

template<>
class DLLIMPEXP_EWA_BASE AtomicHelper<4>
{
public:
	typedef int32_t type;
	static type fetch_add(volatile type* p,type v);
	static type fetch_sub(volatile type* p,type v);
	static type fetch_and(volatile type* p,type v);
	static type fetch_or(volatile type* p,type v);
	static type fetch_xor(volatile type* p,type v);
	static type exchange(volatile type* p,type v);
	static type store(volatile type* p,type v);
	static type load(volatile const type* p);
	static bool compare_exchange(volatile type* p,type& c,type v);
};

template<>
class DLLIMPEXP_EWA_BASE AtomicHelper<8>
{
public:
	typedef int64_t type;
	static type fetch_add(volatile type* p,type v);
	static type fetch_sub(volatile type* p,type v);
	static type fetch_and(volatile type* p,type v);
	static type fetch_or(volatile type* p,type v);
	static type fetch_xor(volatile type* p,type v);
	static type exchange(volatile type* p,type v);
	static type store(volatile type* p,type v);
	static type load(volatile const type* p);
	static bool compare_exchange(volatile type* p,type& c,type v);
};

EW_LEAVE
#endif
