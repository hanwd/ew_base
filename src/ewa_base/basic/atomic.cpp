
#include "ewa_base/basic/atomic.h"
#include "ewa_base/basic/lockguard.h"
#include <cstdlib>

#ifdef EW_WINDOWS
#include <windows.h>
#else
#include <sched.h>
#include <pthread.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable:4146)
#include <intrin.h>
#endif

EW_ENTER




#ifdef _MSC_VER

typedef volatile long* EW_ATOMIC_INT32_POINTER;

int32_t AtomicImpl<4>::fetch_add(volatile type* p,type v)
{
	return InterlockedExchangeAdd((EW_ATOMIC_INT32_POINTER)p,v);
}

int32_t AtomicImpl<4>::fetch_sub(volatile type* p,type v)
{
	return InterlockedExchangeAdd((EW_ATOMIC_INT32_POINTER)p,-v);
}

int32_t AtomicImpl<4>::fetch_and(volatile type* p,type v)
{
	return _InterlockedAnd((EW_ATOMIC_INT32_POINTER)p,v);
}

int32_t AtomicImpl<4>::fetch_or(volatile type* p,type v)
{
	return _InterlockedOr((EW_ATOMIC_INT32_POINTER)p,v);
}

int32_t AtomicImpl<4>::fetch_xor(volatile type* p,type v)
{
	return _InterlockedXor((EW_ATOMIC_INT32_POINTER)p,v);
}

int32_t AtomicImpl<4>::exchange(volatile type* p,type v)
{
	return InterlockedExchange((EW_ATOMIC_INT32_POINTER)p,v);
}

int32_t AtomicImpl<4>::store(volatile type* p,type v)
{
	return InterlockedExchange((EW_ATOMIC_INT32_POINTER)p,v);
}

int32_t AtomicImpl<4>::load(volatile const type* p)
{
	return *p;
}

bool AtomicImpl<4>::compare_exchange(volatile type* p,type& c,type v)
{
	type t=InterlockedCompareExchange((EW_ATOMIC_INT32_POINTER)p,v,c);
	if(t!=c)
	{
		c=t;
		return false;
	}
	else
	{
		return true;
	}
}

typedef volatile LONGLONG* EW_ATOMIC_INT64_POINTER;

int64_t AtomicImpl<8>::fetch_add(volatile type* p,type v)
{
	return InterlockedExchangeAdd64((EW_ATOMIC_INT64_POINTER)p,v);
}

int64_t AtomicImpl<8>::fetch_sub(volatile type* p,type v)
{
	return InterlockedExchangeAdd64((EW_ATOMIC_INT64_POINTER)p,-v);
}

int64_t AtomicImpl<8>::fetch_and(volatile type* p,type v)
{
	return _InterlockedAnd64((EW_ATOMIC_INT64_POINTER)p,v);
}

int64_t AtomicImpl<8>::fetch_or(volatile type* p,type v)
{
	return _InterlockedOr64((EW_ATOMIC_INT64_POINTER)p,v);
}

int64_t AtomicImpl<8>::fetch_xor(volatile type* p,type v)
{
	return _InterlockedXor64((EW_ATOMIC_INT64_POINTER)p,v);
}

int64_t AtomicImpl<8>::exchange(volatile type* p,type v)
{
	return InterlockedExchange64((EW_ATOMIC_INT64_POINTER)p,v);
}

int64_t AtomicImpl<8>::store(volatile type* p,type v)
{
	return InterlockedExchange64((EW_ATOMIC_INT64_POINTER)p,v);
}

int64_t AtomicImpl<8>::load(volatile const type* p)
{
	return *p;
}

bool AtomicImpl<8>::compare_exchange(volatile type* p,type& c,type v)
{
	type t=InterlockedCompareExchange64((EW_ATOMIC_INT64_POINTER)p,v,c);
	if(t!=c)
	{
		c=t;
		return false;
	}
	else
	{
		return true;
	}
}

#else

int32_t AtomicImpl<4>::fetch_add(volatile type* p,type v)
{
	return __sync_fetch_and_add(&val,v);
}

int32_t AtomicImpl<4>::fetch_sub(volatile type* p,type v)
{
	return __sync_fetch_and_sub(&val,v);
}

int32_t AtomicImpl<4>::fetch_and(volatile type* p,type v)
{
	return __sync_fetch_and_and(&val,v);
}

int32_t AtomicImpl<4>::fetch_or(volatile type* p,type v)
{
	return __sync_fetch_and_or(&val,v);
}

int32_t AtomicImpl<4>::fetch_xor(volatile type* p,type v)
{
	return __sync_fetch_and_xor(&val,v);
}

int32_t AtomicImpl<4>::exchange(volatile type* p,type v)
{
	return __sync_lock_test_and_set(&val,v);
}

int32_t AtomicImpl<4>::store(volatile type* p,type v)
{
	return __sync_lock_test_and_set(&val,v);
}

int32_t AtomicImpl<4>::load(volatile const type* p)
{
	return *p;
}

bool AtomicImpl<4>::compare_exchange(volatile type* p,type& expected,type v)
{
	T old=__sync_val_compare_and_swap(p,expected,v);
	if(old==expected) return true;
	expected=old;
	return false;
}


int64_t AtomicImpl<8>::fetch_add(volatile type* p,type v)
{
	return __sync_fetch_and_add(&val,v);
}

int64_t AtomicImpl<8>::fetch_sub(volatile type* p,type v)
{
	return __sync_fetch_and_sub(&val,v);
}

int64_t AtomicImpl<8>::fetch_and(volatile type* p,type v)
{
	return __sync_fetch_and_and(&val,v);
}

int64_t AtomicImpl<8>::fetch_or(volatile type* p,type v)
{
	return __sync_fetch_and_or(&val,v);
}

int64_t AtomicImpl<8>::fetch_xor(volatile type* p,type v)
{
	return __sync_fetch_and_xor(&val,v);
}

int64_t AtomicImpl<8>::exchange(volatile type* p,type v)
{
	return __sync_lock_test_and_set(&val,v);
}

int64_t AtomicImpl<8>::store(volatile type* p,type v)
{
	return __sync_lock_test_and_set(&val,v);
}

int64_t AtomicImpl<8>::load(volatile const type* p)
{
	return *p;
}

bool AtomicImpl<8>::compare_exchange(volatile type* p,type& expected,type v)
{
	T old=__sync_val_compare_and_swap(p,expected,v);
	if(old==expected) return true;
	expected=old;
	return false;
}

#endif


template class AtomicImpl<4>;
template class AtomicImpl<8>;



void AtomicSpin::noop()
{
#ifdef EW_WINDOWS
	_sleep(0);
#else
	sched_yield();
#endif
}


uintptr_t AtomicMutex::thread_id()
{
#ifdef EW_WINDOWS
	return ::GetCurrentThreadId();
#else
	return (uintptr_t)pthread_self();
#endif
}

template class AtomicIntT<int32_t>;
template class AtomicIntT<uint32_t>;

template class AtomicIntT<int64_t>;
template class AtomicIntT<uint64_t>;


EW_LEAVE
