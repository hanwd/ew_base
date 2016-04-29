
#ifndef __H_EW_THREAD_COND__
#define __H_EW_THREAD_COND__

#include "ewa_base/config.h"

#ifndef _MSC_VER
#include <pthread.h>
#else
#include <windows.h>
#endif


EW_ENTER

class DLLIMPEXP_EWA_BASE Mutex;
class DLLIMPEXP_EWA_BASE Condition
{
public:
	Condition();
	Condition(const Condition& o);
	Condition& operator=(const Condition&)
	{
		return *this;
	}

	~Condition();

	void notify_one();
	void notify_all();

	void wait(Mutex& Mutex);
	bool wait_for(Mutex& Mutex,int ms);
	bool wait_for(Mutex& Mutex,const TimeSpan& ts);
	bool wait_until(Mutex& Mutex,const TimePoint& tp);

protected:
#ifdef _MSC_VER
	CONDITION_VARIABLE impl;
#else
	pthread_cond_t impl;
#endif
};


EW_LEAVE
#endif
