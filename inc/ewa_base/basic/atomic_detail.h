
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
	typedef volatile int32_t* vpointer;

	static type fetch_add(vpointer p,type v);
	static type fetch_sub(vpointer p,type v);
	static type fetch_and(vpointer p,type v);
	static type fetch_or(vpointer p,type v);
	static type fetch_xor(vpointer p,type v);
	static type exchange(vpointer p,type v);
	static type store(vpointer p,type v);
	static type load(volatile const type* p);
	static bool compare_exchange(vpointer p,type& c,type v);
};

template<>
class DLLIMPEXP_EWA_BASE AtomicHelper<8>
{
public:
	typedef int64_t type;
	typedef volatile int64_t* vpointer;

	static type fetch_add(vpointer p,type v);
	static type fetch_sub(vpointer p,type v);
	static type fetch_and(vpointer p,type v);
	static type fetch_or(vpointer p,type v);
	static type fetch_xor(vpointer p,type v);
	static type exchange(vpointer p,type v);
	static type store(vpointer p,type v);
	static type load(volatile const type* p);
	static bool compare_exchange(vpointer p,type& c,type v);
};

EW_LEAVE
#endif
