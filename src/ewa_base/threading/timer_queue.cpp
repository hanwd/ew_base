#include "ewa_base/threading/thread.h"
#include "ewa_base/threading/timer_queue.h"
#include "ewa_base/basic/lockguard.h"
#include "thread_impl.h"


EW_ENTER


bool ITimerHolder::cancel()
{
	if (!m_ptr) return false;
	return m_ptr->done.exchange(1) == 0;
}

bool ITimerHolder::redue(const TimePoint& tp)
{
	if(!m_ptr) return false;
	return m_ptr->tque.redue(*this,tp);
}

bool ITimerHolder::redue(const TimeSpan& ts)
{
	return redue(Clock::now()+ts);
}

const TimePoint& ITimerHolder::due() const
{
	if(m_ptr) return m_ptr->tdue;
	static TimePoint tp;
	return tp;
}

TimerQueue::TimerQueue()
{
	set_param(30,15);
}

void TimerQueue::set_param(int w1,int w2)
{
	
	m_nDelta=1;
	if(w1<10) w1=10;
	if(w1>80) w1=80;
	if(w2<10) w2=10;
	if(w2>80) w2=80;
	if(w2>w1) w2=w1;

	m_nWait1=w1;
	m_nWait2=w2;
}

TimerQueue::~TimerQueue()
{
	clear();
}

void TimerQueue::cancel()
{
	clear();
	basetype::cancel();
}

void TimerQueue::clear()
{
	LockGuard<Mutex> lock1(m_tMutex);
	for(arr_1t<ITimer*>::iterator it=m_aQueue.begin();it!=m_aQueue.end();++it)
	{
		(*it)->DecRef();
	}

	m_aQueue.clear();

	m_tCond_empty.notify_all();
	m_tCond.notify_all();
}

void TimerQueue::wait_for_empty(const TimeSpan& ts)
{
	LockGuard<Mutex> lock1(m_tMutex);
	if(!m_aQueue.empty())
	{
		m_tCond_empty.wait_for(m_tMutex,ts);
	}
}

bool itimer_pred(ITimer* t1,ITimer* t2)
{
	return (*t1).tdue>(*t2).tdue;
}

bool TimerQueue::redue(ITimerHolder& h,const TimePoint& tp)
{
	if(!h) return false;
	ITimer& q(*h);
	if(this!=&q.tque) return false;
	h.cancel();

	h=putq(q.hjob.get(),tp);
	return true;
}

ITimerHolder TimerQueue::putq(ITask* hjob,const TimePoint& tp)
{
	if(!hjob) return NULL;

	ITimerHolder q(new ITimer(*this,hjob,tp));
	q->IncRef();

	LockGuard<Mutex> lock1(m_tMutex);
	m_aQueue.push_back(q.get());
	std::push_heap(m_aQueue.begin(),m_aQueue.end(),itimer_pred);
	m_tCond.notify_one();

	return q;
}




void TimerQueue::svc()
{
	ITimerHolder h;
	for(;;)
	{
		h.m_ptr=getq();

		if(!h.m_ptr) break;

		if(h->done.exchange(1)==0)
		{
			if(h->hjob) h->hjob->svc(h);
		}

		h.reset(NULL);		
	}
}



ITimer* TimerQueue::getq()
{

	int64_t ms;
	ITimer* q=NULL;

	for(;;)
	{
		while(1)
		{
			LockGuard<Mutex> lock1(m_tMutex);

			if(test_canceled())
			{
				return NULL;
			}

			if(m_aQueue.empty())
			{
				m_tCond_empty.notify_all();
				m_tCond.wait_for(m_tMutex,100);

				if(m_aQueue.empty())
				{
					continue;
				}
			}

			q=m_aQueue.front();
			TimePoint tnow = Clock::now();


			String s1 = q->tdue.Format();
			String s2 = tnow.Format();



			ms=(q->tdue-tnow).GetMilliSeconds();

			if(ms>=m_nWait1)
			{
				m_tCond.wait_for(m_tMutex,ms-m_nWait2);
				continue;
			}

			std::pop_heap(m_aQueue.begin(),m_aQueue.end(),itimer_pred);
			m_aQueue.pop_back();

			break;
		}


		if (q->done.get()!=0)
		{
			q->DecRef();
			continue;
		}

		while((q->tdue-Clock::now()).GetMilliSeconds()>=m_nDelta);
		return q;

	}
}

EW_LEAVE
