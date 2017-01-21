#include "ewa_base/threading.h"
#include "thread_impl.h"
#include <csignal>
#undef new

EW_ENTER


void _g_signal_terminate(int param)
{
	Logger logger;

	System::LogTrace("signal_terminate_received(%d)",param);

	if (param == SIGTERM)
	{
		logger.LogMessage("SIGTERM Received!");
		::exit(EXIT_FAILURE);
	}

	if (ThreadImpl::sm_bReqexit)
	{
		return;
	}

	if (param == SIGINT)
	{
		logger.LogMessage("SIGINT Received!");
	}
	else
	{
		logger.LogMessage(String::Format("SIG(%d) Received", param));
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


bool ThreadManager::ensure_free_size(size_t n)
{
	while (list_free.size() < n)
	{
		ThreadImpl* p = ThreadImpl::create_one();
		if (!p) return false;
		p->link_to(&list_free);
	}
	return true;
}


EW_LEAVE
