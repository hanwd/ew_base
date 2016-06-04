#ifndef __H_EW_BASIC_ATOMIC__
#define __H_EW_BASIC_ATOMIC__

#include "ewa_base/config.h"
#include "ewa_base/basic/atomic_detail.h"

EW_ENTER


class AtomicOps
{
public:

	template<typename T>
	static EW_FORCEINLINE T fetch_add(volatile T* p,T v)
	{
		typedef AtomicHelper<sizeof(T)> helper;
		return helper::fetch_add((volatile helper::type*)p,v);
	}

	template<typename T>
	static EW_FORCEINLINE T fetch_sub(volatile T* p,T v)
	{
		typedef AtomicHelper<sizeof(T)> helper;
		return helper::fetch_sub((volatile helper::type*)p,v);
	}

	template<typename T>
	static EW_FORCEINLINE T fetch_and(volatile T* p,T v)
	{
		typedef AtomicHelper<sizeof(T)> helper;
		return helper::fetch_and((volatile helper::type*)p,v);
	}

	template<typename T>
	static EW_FORCEINLINE T fetch_or(volatile T* p,T v)
	{
		typedef AtomicHelper<sizeof(T)> helper;
		return helper::fetch_or((volatile helper::type*)p,v);
	}

	template<typename T>
	static EW_FORCEINLINE T fetch_xor(volatile T* p,T v)
	{
		typedef AtomicHelper<sizeof(T)> helper;
		return helper::fetch_xor((volatile helper::type*)p,v);
	}

	template<typename T>
	static EW_FORCEINLINE T exchange(volatile T* p,T v)
	{
		typedef AtomicHelper<sizeof(T)> helper;
		return helper::exchange((volatile helper::type*)p,v);
	}

	template<typename T>
	static EW_FORCEINLINE bool compare_exchange(volatile T* p,T& expected,T v)
	{
		typedef AtomicHelper<sizeof(T)> helper;
		return helper::compare_exchange((volatile helper::type*)p,v);
	}
};

// AtomicIntT, T must be 32bit or 64bit signed/unsigned integer or pointer
template<typename T>
class DLLIMPEXP_EWA_BASE AtomicIntT
{
public:

	typedef T type;
	typedef AtomicHelper<sizeof(type)> helper;

	EW_FORCEINLINE AtomicIntT(){val=0;}

	EW_FORCEINLINE AtomicIntT(T v):val(v) {}

	// fetch_add performs value=value+v and return original value
	EW_FORCEINLINE type fetch_add(type v){return helper::fetch_add((volatile helper::type*)&val,v);}

	// fetch_sub performs value=value-v and return original value
	EW_FORCEINLINE type fetch_sub(type v){return helper::fetch_sub((volatile helper::type*)&val,v);}

	// fetch_and performs value=value&v and return original value
	EW_FORCEINLINE type fetch_and(type v){return helper::fetch_and((volatile helper::type*)&val,v);}

	// fetch_or performs value=value|v and return original value
	EW_FORCEINLINE type fetch_or(type v){return helper::fetch_or((volatile helper::type*)&val,v);}

	// fetch_xor performs value=value^v and return original value
	EW_FORCEINLINE type fetch_xor(type v){return helper::fetch_xor((volatile helper::type*)&val,v);}

	// exchange value and v and return original value
	EW_FORCEINLINE type exchange(type v){return helper::exchange((volatile helper::type*)&val,v);}

	// if value==expected then value=v return true else expected=value and return false.
	EW_FORCEINLINE bool compare_exchange(type& expected,type v){return helper::compare_exchange((volatile helper::type*)&val,*(helper::type*)&expected,v);}

	EW_FORCEINLINE T operator++(){return fetch_add(1)+1;}
	EW_FORCEINLINE T operator++(int){return fetch_add(1);}
	EW_FORCEINLINE T operator--(){return fetch_sub(1)-1;}
	EW_FORCEINLINE T operator--(int){return fetch_sub(1);}

	EW_FORCEINLINE T get() const{return val;}
	EW_FORCEINLINE void set(T v){val=v;}

	// load return value
	EW_FORCEINLINE T load() const{return helper::load((volatile helper::type*)&val);}

	// store performs value=v;
	EW_FORCEINLINE void store(type v){helper::store((volatile helper::type*)&val,v);}

	EW_FORCEINLINE AtomicIntT& operator=(T v){ val=v; return *this; }
	EW_FORCEINLINE operator T() const { return val; }

private:
	volatile T val;
};

typedef AtomicIntT<int32_t> AtomicInt32;
typedef AtomicIntT<int64_t> AtomicInt64;
typedef AtomicIntT<uint32_t> AtomicUint32;
typedef AtomicIntT<uint64_t> AtomicUint64;


template<typename T>
class DLLIMPEXP_EWA_BASE AtomicIntT<T*>
{
public:

	typedef intptr_t diff_type;

	typedef T* type;

	EW_FORCEINLINE  type exchange(type v)
	{
		return (type)impl.exchange((diff_type)v);
	}

	EW_FORCEINLINE  bool compare_exchange(type& expected,type v)
	{
		return impl.compare_exchange(*(diff_type*)&expected,(diff_type)v);
	}

	EW_FORCEINLINE  type get() const
	{
		return (type)impl.get();
	}

	EW_FORCEINLINE  void set(type v)
	{
		impl.set((diff_type)v);
	}

	EW_FORCEINLINE type load()
	{
		return (type)impl.load();
	}

	EW_FORCEINLINE void store(type v)
	{
		impl.store((diff_type)v);
	}

protected:
	AtomicIntT<diff_type> impl;
};

// non-recursive spin
class DLLIMPEXP_EWA_BASE AtomicSpin
{
public:

	static const int32_t nSpinCount=1024*1024*32;

	EW_FORCEINLINE void lock()
	{
		while(val.exchange(1)!=0)
		{
			noop();
		}
	}

	EW_FORCEINLINE bool try_lock()
	{
		return val.exchange(1)==0;
	}

	EW_FORCEINLINE void unlock()
	{
		int32_t old=val.exchange(0);

		EW_ASSERT(old==1);
		EW_UNUSED(old);
	}

	static void noop();

protected:
	AtomicInt32 val;
};


// recursive
class DLLIMPEXP_EWA_BASE AtomicMutex
{
public:
	AtomicMutex(){num=0;}

	EW_FORCEINLINE void lock()
	{
		uintptr_t _id=thread_id();
		while(val.exchange(1)!=0)
		{
			if(_id==tid)
			{
				++num;
				return;
			}
			else
			{
				AtomicSpin::noop();
			}
		}

		EW_ASSERT(num==0);

		num=1;
		tid=_id;
	}

	EW_FORCEINLINE bool try_lock()
	{
		uintptr_t _id=thread_id();
		if(val.exchange(1)!=0)
		{
			if(_id==tid)
			{
				++num;
				return true;
			}
			else
			{
				return false;
			}
		}

		EW_ASSERT(num==0);
		num=1;
		tid=_id;
		return true;
	}

	EW_FORCEINLINE void unlock()
	{
		EW_ASSERT(tid==thread_id());

		if(--num==0)
		{
			tid=uintptr_t(-1);
			int32_t old=val.exchange(0);
			EW_ASSERT(old==1);
			EW_UNUSED(old);
		}
	}

	static uintptr_t thread_id();

private:
	AtomicInt32 val;
	uint32_t num;
	uintptr_t tid;
};


template<typename T1,typename T2>
static void lock_mutex2(T1& m1,T2& m2)
{
	while(1)
	{
		m1.lock();
		if(m2.try_lock()) break;
		m1.unlock();
		AtomicSpin::noop();
	}
}

template<typename T1,typename T2>
static void unlock_mutex2(T1& m1,T2& m2)
{
	m1.unlock();
	m2.unlock();
}


template<bool C>
class ThreadSafe;

template<>
class ThreadSafe<true>
{
public:
	AtomicSpin mutex;
	inline void lock(){mutex.lock();}
	inline bool try_lock(){return mutex.try_lock();}
	inline void unlock(){mutex.unlock();}
};

template<>
class ThreadSafe<false>
{
public:
	inline void lock(){}
	inline bool try_lock(){return true;}
	inline void unlock(){}
};

class DLLIMPEXP_EWA_BASE RefCounter : public mp_obj
{
public:

	AtomicUint64 m_refcount;

	static const uint64_t WEAK_TAG = uint64_t(1) << 32;
	static const uint64_t MASK_TAG = WEAK_TAG - 1;

	EW_FORCEINLINE RefCounter() :m_refcount(0){}
	EW_FORCEINLINE RefCounter(int n) :m_refcount(n){}

	EW_FORCEINLINE bool Lock()
	{
		AtomicUint32& n = *(AtomicUint32*)&m_refcount;
		while (1)
		{
			uint32_t expected = n;
			if (expected == 0) return false;
			if (n.compare_exchange(expected, expected + 1))
			{
				return true;
			}
		}
	}

	EW_FORCEINLINE int GetUseCount()
	{
		return *(AtomicUint32*)&m_refcount;
	}

	EW_FORCEINLINE void IncUseCount()
	{
		m_refcount.fetch_add(1);
	}

	EW_FORCEINLINE bool DecUseCount()
	{
		int64_t val = m_refcount.fetch_sub(1);
		if (val == 1)
		{
			delete this;
		}
		return (val&MASK_TAG) == 1;
	}

	EW_FORCEINLINE void IncWeakCount()
	{
		m_refcount.fetch_add(WEAK_TAG);
	}

	EW_FORCEINLINE void DecWeakCount()
	{
		if (m_refcount.fetch_sub(WEAK_TAG) == WEAK_TAG)
		{
			delete this;
		}
	}
};



EW_LEAVE
#endif
