
#include "ewa_base/threading/thread_mutex.h"
#include "ewa_base/threading/thread_event.h"
#include "ewa_base/basic/lockguard.h"
#include "thread_impl_detail.h"

EW_ENTER

RWLock::RWLock()
{
	ThreadImpl_detail::rwlock_create(*this);
}

RWLock::RWLock(const RWLock&)
{
	ThreadImpl_detail::rwlock_create(*this);
}

RWLock::~RWLock()
{
	ThreadImpl_detail::rwlock_destroy(*this);
}

void RWLock::lock_r()
{
	ThreadImpl_detail::rwlock_lock_r(*this);
}

void RWLock::lock_w()
{
	ThreadImpl_detail::rwlock_lock_w(*this);
}


//bool RWLock::trylock_r()
//{
//	return ThreadImpl_detail::rwlock_trylock_r(*this);
//}
//
//bool RWLock::trylock_w()
//{
//	return ThreadImpl_detail::rwlock_trylock_w(*this);
//}


void RWLock::unlock_r()
{
	ThreadImpl_detail::rwlock_unlock_r(*this);
}

void RWLock::unlock_w()
{
	ThreadImpl_detail::rwlock_unlock_w(*this);
}

Mutex::Mutex()
{
	ThreadImpl_detail::mutex_create(*this);
}

Mutex::Mutex(const Mutex&)
{
	ThreadImpl_detail::mutex_create(*this);
}

Mutex::~Mutex()
{
	ThreadImpl_detail::mutex_destroy(*this);
}

void Mutex::lock()
{
	ThreadImpl_detail::mutex_lock(*this);
}

bool Mutex::try_lock()
{
	return ThreadImpl_detail::mutex_trylock(*this);
}

void Mutex::unlock()
{
	ThreadImpl_detail::mutex_unlock(*this);
}

SpinLock::SpinLock()
{
	ThreadImpl_detail::spinlock_create(*this);
}

SpinLock::SpinLock(const SpinLock&)
{
	ThreadImpl_detail::spinlock_create(*this);
}

SpinLock::~SpinLock()
{
	ThreadImpl_detail::spinlock_destroy(*this);
}

void SpinLock::lock()
{
	ThreadImpl_detail::spinlock_lock(*this);
}

bool SpinLock::try_lock()
{
	return ThreadImpl_detail::spinlock_trylock(*this);
}


void SpinLock::unlock()
{
	ThreadImpl_detail::spinlock_unlock(*this);
}


Condition::Condition()
{
	ThreadImpl_detail::cond_create(*this);
}

Condition::Condition(const Condition&)
{
	ThreadImpl_detail::cond_create(*this);
}

Condition::~Condition()
{
	ThreadImpl_detail::cond_destroy(*this);
}


void Condition::notify_one()
{
	ThreadImpl_detail::cond_signal(*this);
}

void Condition::notify_all()
{
	ThreadImpl_detail::cond_broadcast(*this);
}

void Condition::wait(Mutex& Mutex)
{
	ThreadImpl_detail::cond_wait(*this,Mutex);
}

bool Condition::wait_for(Mutex& Mutex,int ms)
{
	return ThreadImpl_detail::cond_timedwait(*this,Mutex,ms);
}

bool Condition::wait_for(Mutex& Mutex,const TimeSpan& ts)
{
	return ThreadImpl_detail::cond_timedwait(*this,Mutex,ts.GetMilliSeconds());
}

bool Condition::wait_until(Mutex& Mutex,const TimePoint& tp)
{
	return ThreadImpl_detail::cond_timedwait(*this,Mutex,(tp-Clock::now()).GetMilliSeconds());
}

Semaphore::Semaphore()
{
	ThreadImpl_detail::sem_init(*this);
}

Semaphore::Semaphore(const Semaphore&)
{
	ThreadImpl_detail::sem_init(*this);
}

Semaphore::~Semaphore()
{
	ThreadImpl_detail::sem_destroy(*this);
}

void Semaphore::post(int n)
{
	ThreadImpl_detail::sem_post(*this,n);
}

void Semaphore::post()
{
	ThreadImpl_detail::sem_post(*this);
}

void Semaphore::wait()
{
	ThreadImpl_detail::sem_wait(*this);
}

bool Semaphore::try_wait()
{
	return ThreadImpl_detail::sem_trywait(*this);
}

bool Semaphore::wait_for(int n)
{
	return ThreadImpl_detail::sem_timedwait(*this,n);
}

bool Semaphore::wait_for(const TimeSpan& ts)
{
	return ThreadImpl_detail::sem_timedwait(*this,ts.GetMilliSeconds());
}

bool Semaphore::wait_until(const TimePoint& tp)
{
	return ThreadImpl_detail::sem_timedwait(*this,(tp-Clock::now()).GetMilliSeconds());
}

Event::Event()
{
#ifdef EW_WINDOWS
	hEvent=::CreateEvent(NULL,TRUE,FALSE,NULL);
#endif
}

Event::Event(const Event&)
{
#ifdef EW_WINDOWS
	hEvent=::CreateEvent(NULL,TRUE,FALSE,NULL);
#endif
}


void Event::reset()
{
#ifdef EW_WINDOWS
	::ResetEvent(hEvent);
#else
	m_nValue.store(0);
#endif

}

void Event::set()
{
#ifdef EW_WINDOWS
	::SetEvent(hEvent);
#else

	if(m_nValue.get()!=0) return;
	LockGuard<Mutex> lock1(m_tMutex);
	m_nValue.store(1);
	m_tCond.notify_all();
#endif
}

void Event::wait()
{
#ifdef EW_WINDOWS
	::WaitForSingleObject(hEvent,INFINITE);
#else

	if(m_nValue.get()!=0) return;
	LockGuard<Mutex> lock1(m_tMutex);
	if(m_nValue.get()==0)
	{
		m_tCond.wait(m_tMutex);
	}
#endif
}

bool Event::wait_for(int ms)
{
#ifdef EW_WINDOWS

	DWORD rc=::WaitForSingleObject(hEvent,ms);
	if(rc==WAIT_TIMEOUT)
	{
		return false;
	}
	return true;

#else
	if(m_nValue.get()!=0) return true;
	LockGuard<Mutex> lock1(m_tMutex);
	if(m_nValue.get()==0)
	{
		m_tCond.wait_for(m_tMutex,ms);
	}
	return m_nValue.get()!=0;
#endif

}

bool Event::wait_for(const TimeSpan& ts)
{
#ifdef EW_WINDOWS
	return wait_for(ts.GetMilliSeconds());
#else
	if(m_nValue.get()!=0) return true;
	LockGuard<Mutex> lock1(m_tMutex);
	if(m_nValue.get()==0)
	{
		m_tCond.wait_for(m_tMutex,ts);
	}
	return m_nValue.get()!=0;
#endif
}

bool Event::wait_until(const TimePoint& tp)
{
#ifdef EW_WINDOWS
	return wait_for(tp-Clock::now());
#else
	if(m_nValue.get()!=0) return true;
	LockGuard<Mutex> lock1(m_tMutex);
	if(m_nValue.get()==0)
	{
		m_tCond.wait_until(m_tMutex,tp);
	}
	return m_nValue.get()!=0;
#endif
}

Event::~Event()
{
#ifdef EW_WINDOWS
	CloseHandle(hEvent);
#endif
}

EW_LEAVE

