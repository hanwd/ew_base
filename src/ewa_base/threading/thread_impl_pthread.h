#include "ewa_base/threading/thread.h"
#include "ewa_base/threading/thread_spin.h"
#include "ewa_base/threading/thread_mutex.h"
#include "ewa_base/threading/thread_cond.h"
#include "ewa_base/threading/thread_sem.h"
#include "ewa_base/threading/thread_rwlock.h"
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

EW_ENTER

void thread_impl_entry_real(ThreadImpl* arg);

class ThreadImpl_pthread
{
public:

	typedef pthread_t thread_t;

	static void* thread_func(void* arg)
	{
		pthread_detach(pthread_self());
		thread_impl_entry_real((ThreadImpl*)arg);
		return NULL;
	}

	static uintptr_t get_id()
	{
		return (uintptr_t)pthread_self();
	}

	static void set_priority(int n)
	{

	}

	static void set_affinity(int n)
	{
#ifndef EW_WINDOWS
		cpu_set_t cpu_info;
		CPU_ZERO(&cpu_info);
		CPU_SET(n+1, &cpu_info);
		pthread_setaffinity_np(pthread_self(),sizeof(cpu_set_t),&cpu_info);
#endif
	}

	//static pthread_attr_t* thread_attr()
	//{
	//	static pthread_attr_t s_attr;
	//	pthread_attr_init(&s_attr);
	//	pthread_attr_setdetachstate(&s_attr,PTHREAD_CREATE_DETACHED);
	//}

	static bool thread_create(thread_t& t,void* arg)
	{
		int ret= pthread_create(&t,NULL,thread_func,arg);
		if(ret!=0)
		{
			return false;
		}
		return true;
	}

	static void thread_destroy(thread_t& t)
	{
		pthread_join(t,NULL);
	}


	static pthread_rwlock_t* g(RWLock& m)
	{
		return (pthread_rwlock_t*)&m;
	}

	static void rwlock_create(RWLock& m)
	{
		pthread_rwlock_init(g(m),NULL);
	}

	static void rwlock_destroy(RWLock& m)
	{
		pthread_rwlock_destroy(g(m));
	}

	static void rwlock_lock_r(RWLock& m)
	{
		pthread_rwlock_rdlock(g(m));
	}

	static void rwlock_lock_w(RWLock& m)
	{
		pthread_rwlock_wrlock(g(m));
	}

	static bool rwlock_trylock_r(RWLock& m)
	{
		return pthread_rwlock_tryrdlock(g(m))!=0;
	}

	static bool rwlock_trylock_w(RWLock& m)
	{
		return pthread_rwlock_trywrlock (g(m))!=0;
	}

	static void rwlock_unlock_r(RWLock& m)
	{
		pthread_rwlock_unlock (g(m));
	}

	static void rwlock_unlock_w(RWLock& m)
	{
		pthread_rwlock_unlock (g(m));
	}


	static pthread_spinlock_t* g(SpinLock& m)
	{
		return (pthread_spinlock_t*)&m;
	}

	static void spinlock_create(SpinLock& m)
	{
		pthread_spin_init(g(m),0);
	}

	static void spinlock_destroy(SpinLock& m)
	{
		pthread_spin_destroy(g(m));
	}

	static void spinlock_lock(SpinLock& m)
	{
		pthread_spin_lock(g(m));
	}

	static bool spinlock_trylock(SpinLock& m)
	{
		return pthread_spin_trylock(g(m))==0;
	}

	static void spinlock_unlock(SpinLock& m)
	{
		pthread_spin_unlock(g(m));
	}

	static pthread_mutex_t* g(Mutex& m)
	{
		return (pthread_mutex_t*)&m;
	}

	static void mutex_create(Mutex& m)
	{
		pthread_mutex_init(g(m),NULL);
	}

	static void mutex_destroy(Mutex& m)
	{
		pthread_mutex_destroy(g(m));
	}

	static void mutex_lock(Mutex& m)
	{
		pthread_mutex_lock(g(m));
	}

	static bool mutex_trylock(Mutex& m)
	{
		return pthread_mutex_trylock(g(m))==0;
	}

	static void mutex_unlock(Mutex& m)
	{
		pthread_mutex_unlock(g(m));
	}


	static pthread_cond_t* g(Condition& m)
	{
		return (pthread_cond_t*)&m;
	}

	static void cond_create(Condition& m)
	{
		pthread_cond_init(g(m),NULL);
	}

	static void cond_destroy(Condition& m)
	{
		pthread_cond_destroy(g(m));
	}

	static void cond_signal(Condition& m)
	{
		pthread_cond_signal(g(m));
	}

	static void cond_broadcast(Condition& m)
	{
		pthread_cond_broadcast(g(m));
	}

	static void cond_wait(Condition& m,Mutex& k)
	{
		pthread_cond_wait(g(m),g(k));
	}

	static void relative(struct timespec& ts,int ms)
	{
		ts.tv_sec=::time(NULL)+ms/1000;
		ts.tv_nsec = (ms % 1000)*1000000;
	}

	static bool cond_timedwait(Condition& m,Mutex& k,int ms)
	{
		struct timespec ts;
		relative(ts,ms);
		return pthread_cond_timedwait(g(m),g(k),&ts)==0;
	}

	static sem_t* g(Semaphore& m)
	{
		return (sem_t*)&m;
	}

	static void sem_init(Semaphore& m)
	{
		if(::sem_init(g(m),0,0)!=0)
		{
			System::LogWarning("sem_init failed");
		}
	}

	static void sem_post(Semaphore& m)
	{
		if(::sem_post(g(m))!=0)
		{
			System::LogWarning("sem_post failed");
		}
	}

	static void sem_post(Semaphore& m,int n)
	{
		for(int i=0; i<n; i++)
		{
			if(::sem_post(g(m))!=0)
			{
				System::LogWarning("sem_post failed");
				break;
			}
		}
		//::sem_post_multiple(g(m),n);
	}

	static void sem_wait(Semaphore& m)
	{
		::sem_wait(g(m));
	}

	static bool sem_trywait(Semaphore& m)
	{
		return ::sem_trywait(g(m))==0;
	}

	static bool sem_timedwait(Semaphore& m,int ms)
	{
		struct timespec ts;
		relative(ts,ms);
		return ::sem_timedwait(g(m),&ts)==0;
	}

	static void sem_destroy(Semaphore& m)
	{
		::sem_destroy(g(m));
	}


	typedef pthread_key_t key_t;

	static void key_create(key_t& k)
	{
		pthread_key_create(&k,NULL);
	}

	static void key_destroy(key_t& k)
	{
		pthread_key_delete(k);
	}

	static void key_set(key_t& k,void* d)
	{
		pthread_setspecific(k,d);
	}

	static void* key_get(key_t& k)
	{
		return pthread_getspecific(k);
	}

};

};//namespace
