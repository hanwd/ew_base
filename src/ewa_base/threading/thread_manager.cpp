#include "ewa_base/threading.h"
#include "thread_impl.h"
#include <csignal>
#undef new

EW_ENTER


void _g_signal_terminate(int param)
{

	System::LogTrace("signal_terminate_received(%d)",param);

	if (param == SIGTERM)
	{
		this_logger().LogMessage("SIGTERM Received!");
		::exit(EXIT_FAILURE);
	}

	if (ThreadImpl::sm_bReqexit)
	{
		return;
	}

	if (param == SIGINT)
	{
		this_logger().LogMessage("SIGINT Received!");
	}
	else
	{
		this_logger().LogMessage(String::Format("SIG(%d) Received", param));
	}

	ThreadImpl::sm_bReqexit = true;
}



ThreadManager::ThreadManager()
{
	signal(SIGTERM, _g_signal_terminate);
	signal(SIGINT, _g_signal_terminate);
	m_nThreadMax=10;
	m_nThreadNum=0;
}


ThreadManager::~ThreadManager()
{

}


int ThreadManager::count()
{
	return m_nThreadNum.get();
}

void ThreadManager::close(bool and_wait)
{
	{
		LockGuard<Mutex> lock1(m_thrd_mutex);
		ThreadImpl::sm_bReqexit=1;
		m_nFlags.add(ThreadImpl::THREADMANAGER_DISABLED);
		m_thrd_attached.notify_all();
	}

	if (and_wait)
	{
		wait();
	}
}

void ThreadManager::wait()
{
	BitFlags oldflag=m_nFlags;
	LockGuard<Mutex> lock1(m_thrd_mutex);
	m_nFlags.add(ThreadImpl::THREADMANAGER_DISABLED);
	m_thrd_attached.notify_all();
	while(m_nThreadNum.get()!=0)
	{
		m_cond_thrd_empty.wait_for(lock1,1000);
	}
	m_nFlags=oldflag;
}

void ThreadManager::ThreadLink::append(ThreadImpl* p)
{
	EW_ASSERT(p->ptr_link == NULL);

	p->ptr_link=this;
	if (tail)
	{
		p->ptr_prev = tail;
		p->ptr_next = NULL;
		tail->ptr_next = p;
		tail = p;
	}
	else
	{
		p->ptr_prev = NULL;
		p->ptr_next = NULL;
		head = tail= p;
	}
	++size;
}

void ThreadManager::ThreadLink::remove(ThreadImpl* p)
{
	if (p->ptr_link != this)
	{
		EW_ASSERT(false);
		return;
	}
	p->ptr_link=NULL;

	if (p->ptr_prev)
	{
		p->ptr_prev->ptr_next = p->ptr_next;
	}
	else
	{
		head = p->ptr_next;
	}

	if (p->ptr_next)
	{
		p->ptr_next->ptr_prev = p->ptr_prev;
	}
	else
	{
		tail = p->ptr_prev;
	}
	--size;
}

ThreadImpl* ThreadManager::ThreadLink::getnum(size_t n)
{
	while (size < n)
	{
		ThreadImpl* p = ThreadImpl::create_one();
		if (!p) return NULL;
		append(p);
	}
	ThreadImpl* p1 = head;
	for (size_t i = 0; i < n; i++)
	{
		head->ptr_link = NULL;
		head = head->ptr_next;
	}

	if (head)
	{
		head->ptr_prev->ptr_next = NULL;
		head->ptr_prev = NULL;
	}
	else
	{
		tail = NULL;
	}

	size -= n;

	return p1;
}


EW_LEAVE
