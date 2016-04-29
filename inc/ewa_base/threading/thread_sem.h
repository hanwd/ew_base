
#ifndef __H_EW_THREAD_SEM__
#define __H_EW_THREAD_SEM__


#include "ewa_base/config.h"

#ifndef _MSC_VER
#include <pthread.h>
#include "semaphore.h"
#else
#include <windows.h>
#endif


EW_ENTER

class DLLIMPEXP_EWA_BASE Semaphore
{
public:
	Semaphore();
	Semaphore(const Semaphore&);
	Semaphore& operator=(const Semaphore&)
	{
		return *this;
	}

	~Semaphore();

	void post();
	void post(int n);
	void wait();
	bool try_wait();

	bool wait_for(int ms);
	bool wait_for(const TimeSpan& ts);
	bool wait_until(const TimePoint& tp);

protected:
#ifdef _MSC_VER
	void* impl;
#else
	sem_t impl;
#endif

};



EW_LEAVE
#endif
