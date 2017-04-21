#ifndef __H_EW_THREAD_TIMER_QUEUE__
#define __H_EW_THREAD_TIMER_QUEUE__

#include "ewa_base/threading/thread_pool.h"


EW_ENTER


class DLLIMPEXP_EWA_BASE TimerQueue;
class DLLIMPEXP_EWA_BASE ITimerHolder;

class DLLIMPEXP_EWA_BASE ITimer : public ObjectData
{
public:

	friend class TimerQueue;
	friend class ITimerHolder;

	const TimePoint tdue;

	ITimer(ITask* ptask, const TimePoint& t);
	ITimer(ITask* ptask, const TimePoint& t, TimerQueue& q);

protected:

	TimerQueue& tque;
	DataPtrT<ITask> hjob;
	AtomicInt32 done;
	BitFlags flags;
};

class DLLIMPEXP_EWA_BASE ITimerHolder : public DataPtrT<ITimer>
{
public:
	friend class TimerQueue;

	typedef DataPtrT<ITimer> basetype;
	ITimerHolder(ITimer* q=NULL):basetype(q){}

	bool cancel();

	bool redue(const TimePoint& tp);
	bool redue(const TimeSpan& ts);

	void bind(ITask* ptask, TimerQueue& q);
	void bind(ITask* ptask);

	const TimePoint& due() const;

};


class DLLIMPEXP_EWA_BASE TimerQueue : public ThreadMulti
{
public:
	typedef ThreadMulti basetype;

	TimerQueue();
	~TimerQueue();

	ITimerHolder putq(ITask* hjob,const TimePoint& tp);

	ITimerHolder putq(ITask* hjob,const TimeSpan& ts)
	{
		return putq(hjob,Clock::now()+ts);
	}

	bool redue(ITimerHolder& h,const TimePoint& tp);

	void svc();

	void cancel();

	void clear();

	void wait_for_empty(const TimeSpan& ts=TimeSpan::Day(365));

	void set_param(int w1,int w2=25);

	static TimerQueue& current();

protected:

	ITimer* getq();

	arr_1t<ITimer*> m_aQueue;

	Mutex m_tMutex;
	Condition m_tCond_empty;
	Condition m_tCond;

	int m_nDelta;
	int m_nWait1;
	int m_nWait2;
};


EW_LEAVE
#endif
