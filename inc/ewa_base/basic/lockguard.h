#ifndef __H_EW_THREAD_LOCKGUARD__
#define __H_EW_THREAD_LOCKGUARD__

#include "ewa_base/config.h"

EW_ENTER

template<typename T>
class LockPolicyDefault
{
public:

	static inline void lock(T& mtx)
	{
		mtx.lock();
	}

	static inline void unlock(T& mtx)
	{
		mtx.unlock();
	}

};


template<typename T,typename P=LockPolicyDefault<T> >
class LockGuard : public NonCopyable
{
public:

	inline LockGuard(T& mtx_):mtx(mtx_)
	{
		P::lock(mtx);
	}

	inline ~LockGuard()
	{
		P::unlock(mtx);
	}

	operator T&()
	{
		return mtx;
	}

private:
	T& mtx;
};


template<typename T>
class LockState : public NonCopyable
{
public:
	T oldvalue;
	T& value;

	LockState(T& v,const T n=T()):value(v)
	{
		oldvalue=value;
		value=n;
	}

	~LockState()
	{
		value=oldvalue;
	}

};

EW_LEAVE
#endif
