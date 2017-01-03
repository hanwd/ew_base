#include "ewa_base/threading/thread.h"
#include "ewa_base/threading/thread_pool.h"
#include "ewa_base/basic/lockguard.h"
#include "thread_impl.h"


EW_ENTER

ThreadPool::ThreadPool()
{
	flags.add(Thread::FLAG_DYNAMIC);
	m_nWorkerMin=1;
	m_nWorkerMax=4;
	m_nWorkerNum=0;
	m_nWorkerWait=0;
	m_nTaskHint=20;
}

void ThreadPool::set_worker_min(int n)
{
	if(n<1) return;
	m_nWorkerMin=n;
}

void ThreadPool::set_worker_max(int n)
{
	if(n<m_nWorkerMin) return;
	m_nWorkerMax=n;
}

int ThreadPool::get_worker_min()
{
	return m_nWorkerMin;
}

int ThreadPool::get_worker_max()
{
	return m_nWorkerMax;
}

int ThreadPool::get_worker_num()
{
	return m_nWorkerNum;
}

bool ThreadPool::empty()
{
	LockGuard<Mutex> lock1(m_tMutex);
	return m_tTaskQueue.empty();

}

void ThreadPool::wait_for_empty(const TimeSpan& ts)
{
	LockGuard<Mutex> lock1(m_tMutex);
	if (!m_tTaskQueue.empty())
	{
		m_tEmpty.wait_for(m_tMutex,ts);
	}
}

void ThreadPool::putq(ITask* q)
{
	size_t qs;

	LockGuard<Mutex> lock1(m_tMutex);
	m_tTaskQueue.append(new TaskItem(q));
	qs=m_tTaskQueue.size();

	if(m_nWorkerWait>0)
	{
		m_tCond.notify_one();
	}

	if(qs%m_nTaskHint==0)
	{	
		if(m_nWorkerNum<m_nWorkerMax)
		{
			this_logger().LogMessage("Too many tasks, creating new worker");
			if(basetype::activate())
			{
				m_nWorkerNum++;
			}
		}	
	}
}

void ThreadPool::cancel()
{
	LockGuard<Mutex> lock1(m_tMutex);
	m_tTaskQueue.clear();

	basetype::cancel();
	m_tCond.notify_all();
}

bool ThreadPool::activate()
{
	LockGuard<Mutex> lock1(m_tMutex);
	if(m_nWorkerNum!=0)
	{
		return false;
	}

	if(!ThreadImpl::activate(*this,m_nWorkerMin))
	{
		return false;
	}

	m_nWorkerNum=m_nWorkerMin;
	return true;

}


ThreadPool::TaskItem* ThreadPool::getq()
{
	LockGuard<Mutex> lock1(m_tMutex);
	for(;;)
	{
		if(test_canceled())
		{
			m_nWorkerNum--;
			return NULL;
		}

		TaskItem* q=m_tTaskQueue.pop_front();
		if(q!=NULL)
		{
			return q;
		}

		if(m_nWorkerWait>0 && m_nWorkerNum>m_nWorkerMin)
		{
			this_logger().LogMessage("too many workers, leaving");
			m_nWorkerNum--;
			return NULL;
		}

		m_nWorkerWait++;
		if (m_nWorkerWait == m_nWorkerNum)
		{
			m_tEmpty.notify_all();
		}

		m_tCond.wait(m_tMutex);
		m_nWorkerWait--;
	}
}

void ThreadPool::svc()
{
	this_logger().LogMessage("WorkerThread enter");

	TaskItem* q=NULL;
	for(;;)
	{
		q=getq();
		if(!q)	break;
		q->hjob->svc(*this);
		delete q;
	}

	this_logger().LogMessage("WorkerThread leave");
}

EW_LEAVE

