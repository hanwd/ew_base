
#ifndef __H_EW_THREAD_MUTEX__
#define __H_EW_THREAD_MUTEX__

#include "ewa_base/config.h"
#include "ewa_base/basic/clock.h"

#ifndef _MSC_VER
#include <pthread.h>
#else
#include <windows.h>
#endif


EW_ENTER

class DLLIMPEXP_EWA_BASE Mutex
{
public:

	Mutex();
	Mutex(const Mutex&);
	Mutex& operator=(const Mutex&)
	{
		return *this;
	}

	~Mutex();

	void lock();
	void unlock();
	bool try_lock();

protected:
#ifdef EW_WINDOWS
	CRITICAL_SECTION impl;
#else
	pthread_mutex_t impl;
#endif
};

EW_LEAVE
#endif
