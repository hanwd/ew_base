#pragma push_macro("new")
#undef new
#include <map>
#pragma pop_macro("new")

#include "ewa_base/threading/thread.h"
#include "ewa_base/threading/thread_rwlock.h"
#include "ewa_base/threading/thread_spin.h"
#include "ewa_base/threading/thread_mutex.h"
#include "ewa_base/threading/thread_cond.h"
#include "ewa_base/threading/thread_sem.h"
#include "ewa_base/basic/exception.h"

#include <process.h>
#include <windows.h>



EW_ENTER

void thread_impl_entry_real(ThreadImpl* arg);

class ThreadImpl_windows
{
public:

	typedef HANDLE thread_t;

	template<typename T>
	static T WINAPI thread_func_t(void* arg)
	{
		thread_impl_entry_real((ThreadImpl*)arg);
		return T();
	}

	static uintptr_t get_id()
	{
		return (uintptr_t)::GetCurrentThreadId();
	}

	static void set_priority(int n_)
	{
		int n=(n_*6)/100;
		::SetThreadPriority(::GetCurrentThread(),n);
	}

	static void set_affinity(int n)
	{
		if(n>=0)
		{
			::SetThreadAffinityMask(::GetCurrentThread(),1<<n);
		}
		else
		{
			::SetThreadAffinityMask(::GetCurrentThread(),0);
		}
	}

	static bool thread_create(thread_t& t,void* arg)
	{
		t=(thread_t)_beginthreadex(NULL,0,thread_func_t<unsigned>,arg,0,NULL);
		return t!=NULL;
	}

	static void thread_destroy(thread_t& t)
	{
		CloseHandle(t);
		t=NULL;
	}

#ifndef _MINGW

	static PSRWLOCK g(RWLock& m)
	{
		return (PSRWLOCK)&m;
	}

	static void rwlock_create(RWLock& m)
	{
		InitializeSRWLock(g(m));
	}

	static void rwlock_destroy(RWLock& m)
	{
		(void)&m;
	}

	static void rwlock_lock_r(RWLock& m)
	{
		::AcquireSRWLockShared(g(m));
	}

	static void rwlock_lock_w(RWLock& m)
	{
		AcquireSRWLockExclusive(g(m));
	}


	//static bool rwlock_trylock_r(RWLock& m)
	//{
	//	return TryAcquireSRWLockShared(g(m))!=0;
	//}

	//static bool rwlock_trylock_w(RWLock& m)
	//{
	//	return TryAcquireSRWLockExclusive(g(m))!=0;
	//}

	static void rwlock_unlock_r(RWLock& m)
	{
		ReleaseSRWLockShared(g(m));
	}

	static void rwlock_unlock_w(RWLock& m)
	{
		::ReleaseSRWLockExclusive(g(m));
	}

#endif

	static PCRITICAL_SECTION g(SpinLock& m)
	{
		return (PCRITICAL_SECTION)&m;
	}

	static void spinlock_create(SpinLock& m)
	{
		if(InitializeCriticalSectionAndSpinCount(g(m),8000)==0)
		{
			Exception::XError("spinlock_create_failed",false);
		}
	}

	static void spinlock_destroy(SpinLock& m)
	{
		DeleteCriticalSection(g(m));
	}

	static void spinlock_lock(SpinLock& m)
	{
		::EnterCriticalSection(g(m));
	}

	static bool spinlock_trylock(SpinLock& m)
	{
		return TryEnterCriticalSection(g(m))!=FALSE;
	}

	static void spinlock_unlock(SpinLock& m)
	{
		::LeaveCriticalSection(g(m));
	}


	static PCRITICAL_SECTION g(Mutex& m)
	{
		return (PCRITICAL_SECTION)&m;
	}

	static void mutex_create(Mutex& m)
	{
		InitializeCriticalSection(g(m));
	}

	static void mutex_destroy(Mutex& m)
	{
		DeleteCriticalSection(g(m));
	}

	static void mutex_lock(Mutex& m)
	{
		::EnterCriticalSection(g(m));
	}

	static bool mutex_trylock(Mutex& m)
	{
		return TryEnterCriticalSection(g(m))!=FALSE;
	}

	static void mutex_unlock(Mutex& m)
	{
		::LeaveCriticalSection(g(m));
	}


	static CONDITION_VARIABLE* g(Condition& m)
	{
		return (CONDITION_VARIABLE*)&m;
	}

	static void cond_create(Condition& m)
	{
		::InitializeConditionVariable(g(m));
	}

	static void cond_destroy(Condition& m)
	{
		(void)&m;
	}

	static void cond_signal(Condition& m)
	{
		WakeConditionVariable (g(m));
	}

	static void cond_broadcast(Condition& m)
	{
		WakeAllConditionVariable (g(m));
	}

	static void cond_wait(Condition& m,Mutex& k)
	{
		SleepConditionVariableCS (g(m), g(k), INFINITE);
	}

	static bool cond_timedwait(Condition& m,Mutex& k,int ms)
	{
		if(ms<0) return true;
		return SleepConditionVariableCS (g(m), g(k),ms)!=FALSE;
	}

	static HANDLE& g(Semaphore& m)
	{
		return *(HANDLE*)&m;
	}

	static void sem_init(Semaphore& m)
	{
		g(m)=::CreateSemaphore(NULL,0,LONG_MAX,NULL);
	}

	static void sem_post(Semaphore& m)
	{
		::ReleaseSemaphore(g(m),1,NULL);
	}

	static void sem_post(Semaphore& m,int n)
	{
		::ReleaseSemaphore(g(m),n,NULL);
	}

	static void sem_wait(Semaphore& m)
	{
		::WaitForSingleObject(g(m),INFINITE);
	}

	static bool sem_trywait(Semaphore& m)
	{
		return ::WaitForSingleObject(g(m),0)==0;
	}

	static bool sem_timedwait(Semaphore& m,int ms)
	{
		if(ms<0) return true;
		return ::WaitForSingleObject(g(m),ms)==0;
	}

	static void sem_destroy(Semaphore& m)
	{
		::CloseHandle(g(m));
	}

	typedef DWORD key_t;

	static void key_create(key_t& k)
	{
		k=TlsAlloc();
	}

	static void key_destroy(key_t& k)
	{
		TlsFree(k);
	}

	static void key_set(key_t& k,void* d)
	{
		TlsSetValue(k,d);
	}

	static void* key_get(key_t& k)
	{
		return TlsGetValue(k);
	}



};

EW_LEAVE
