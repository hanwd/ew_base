
#ifndef __H_EW_THREAD_EVENT__
#define __H_EW_THREAD_EVENT__

#include "ewa_base/basic/atomic.h"
#include "ewa_base/threading/thread_cond.h"
#include "ewa_base/threading/thread_mutex.h"

#ifndef _MSC_VER
#include <pthread.h>
#else
#include <windows.h>
#endif

EW_ENTER

// Manual reset event, intitial state: not set
class DLLIMPEXP_EWA_BASE Event
{
public:

	Event();
	Event(const Event&);
	Event& operator=(const Event&)
	{
		return *this;
	}

	~Event();

	void reset();
	void set();
	void wait();
	bool wait_for(int ms);
	bool wait_for(const TimeSpan& ts);
	bool wait_until(const TimePoint& tp);

protected:

#ifdef EW_WINDOWS
	void* hEvent;
#else
	Mutex m_tMutex;
	Condition m_tCond;
	AtomicInt32 m_nValue;
#endif

};


EW_LEAVE
#endif
