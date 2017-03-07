#ifndef __H_EW_THREAD_LOCKGUARD__
#define __H_EW_THREAD_LOCKGUARD__

#include "ewa_base/config.h"

EW_ENTER

template<typename T>
class LockPolicyDefault
{
public:

	static inline void lock(T* mtx)
	{
		mtx->lock();
	}

	static inline void unlock(T* mtx)
	{
		mtx->unlock();
	}

	static inline bool try_lock(T* mtx)
	{
		return mtx->try_lock();
	}

};


template<typename T,typename P=LockPolicyDefault<T> >
class LockGuard : public NonCopyable
{
public:

	inline LockGuard(T& mtx_,bool already_locked):mtx(mtx_)
	{
		if (!already_locked) P::lock(&mtx);
	}

	inline LockGuard(T& mtx_):mtx(mtx_)
	{
		P::lock(&mtx);
	}

	inline ~LockGuard()
	{
		P::unlock(&mtx);
	}

	operator T&()
	{
		return mtx;
	}

private:
	T& mtx;
};


class DLLIMPEXP_EWA_BASE LockGuard2 : public NonCopyable
{
public:
	typedef void(*unlock_func)(void*);

	inline LockGuard2() :pmtx(NULL){}

	template<typename T>
	inline LockGuard2(T& v) : pmtx(NULL){ try_lock(v); }

	inline ~LockGuard2()
	{
		if (pmtx) func(pmtx);
	}


	template<typename T,typename P = LockPolicyDefault<T> >
	bool try_lock(T& v)
	{
		if (!P::try_lock(&v)) return false;
		if (pmtx) func(pmtx);
		pmtx = &v;
		func = (unlock_func)P::unlock;		
		return true;
	}

	void release()
	{ 
		if (!pmtx) return;
		func(pmtx);
		pmtx = NULL;
	}

	inline operator bool(){ return pmtx!=NULL; }

private:
	void* pmtx;
	unlock_func func;
};


template<typename T>
class LockState : public NonCopyable
{
public:
	T oldvalue;
	T& value;

	inline LockState(T& v,const T n=T()):value(v)
	{
		oldvalue=value;
		value=n;
	}

	inline ~LockState()
	{
		value=oldvalue;
	}

};


class DLLIMPEXP_EWA_BASE AtomicMutex;
class DLLIMPEXP_EWA_BASE StaticMutex : public NonCopyableAndNonNewable
{
public:
	operator AtomicMutex&(){ return *(AtomicMutex*)this;}
private:
	int64_t val[4];
};


template<>
class DLLIMPEXP_EWA_BASE LockPolicyDefault<StaticMutex>
{
public:
	static void lock(StaticMutex* mtx);
	static void unlock(StaticMutex* mtx);
};




EW_LEAVE
#endif
