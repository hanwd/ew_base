#include "thread_impl.h"
#include "ewa_base/basic/lockguard.h"
#include "ewa_base/threading/thread_pool.h"
#include "ewa_base/threading/timer_queue.h"

EW_ENTER


EW_THREAD_TLS ThreadImpl* tc_impl_data;

DLLIMPEXP_EWA_BASE void tc_init();
DLLIMPEXP_EWA_BASE void tc_gc(int);
DLLIMPEXP_EWA_BASE void tc_cleanup();

bool ThreadImpl::sm_bReqexit=false;


void ThreadImpl::set_priority(int n)
{
	if(n<0) n=0;
	if(n>100) n=100;
	thrd_priority=n;
	ThreadImpl_detail::set_priority(n);
}

void ThreadImpl::set_affinity(int n)
{
	thrd_affinity=n;
	ThreadImpl_detail::set_affinity(n);
}

void thread_impl_entry_real(ThreadImpl* impl)
{
	if(impl->svc_enter())
	{
		impl->svc();
		impl->svc_leave();
	}
}



template<typename G>
static bool ThreadImpl::activate_t(Thread& thrd, G& g)
{
	size_t n = g.size();
	if (n == 0) return false;

	ThreadManager& tmgr(ThreadManager::current());

	LockGuard<Mutex> lock1(tmgr.m_thrd_mutex);
	if (tmgr.m_nFlags.get(ThreadImpl::THREADMANAGER_DISABLED))
	{
		return false;
	}

	LockGuard<Mutex> lock2(thrd.m_thrd_mutex);
	if (!thrd.flags.get(Thread::FLAG_DYNAMIC) && thrd.m_nAlive != 0)
	{
		return false;
	}

	if(!tmgr.ensure_free_size(n)) return false;

	DLinkT<ThreadImpl>::iterator it=tmgr.list_free.begin();

	for (size_t i = 0; i<n; i++)
	{
		ThreadImpl* pthrd=*it++;

		if (i<thrd.m_aBindCpu.size())
		{
			pthrd->thrd_affinity = thrd.m_aBindCpu[i];
		}
		else
		{
			pthrd->thrd_affinity = -1;
		}
		pthrd->set_thread(&thrd, g[i], i);
		pthrd->link_to(&tmgr.list_work);
	}

	thrd.m_nAlive += n;
	tmgr.m_thrd_attached.notify_all();

	return true;
}

bool ThreadImpl::activate(Thread& thrd,ThreadEx::InvokerGroup& g)
{
	return activate_t(thrd, g);
}

bool ThreadImpl::activate(Thread& thrd,size_t n)
{
	class FakeGroup
	{
	public:
		size_t sz;
		FakeGroup(int n) :sz(n){}
		size_t size(){ return sz; }
		ThreadEx::factor_type operator[](size_t){ return ThreadEx::factor_type(); }
	}g(n);
	return activate_t(thrd, g);
}

ThreadImpl::ThreadImpl()
{
	thrd_rank=-1;
	thrd_ptr=NULL;
	thrd_affinity=0;
	thrd_priority=50;
}

ThreadImpl::~ThreadImpl()
{
	link_to(NULL);

	if(thrd_created)
	{
		ThreadImpl_detail::thread_destroy(thrd_id);
	}
}

ThreadImpl* ThreadImpl::create_one()
{
	ThreadImpl* impl;
	try
	{
		impl = new ThreadImpl();
	}
	catch (...)
	{
		return NULL;
	}

	impl->thrd_created=ThreadImpl_detail::thread_create(impl->thrd_id,impl);

	if (!impl->thrd_created)
	{
		System::CheckError("unable to create thread");

		delete impl;
		return NULL;
	}

	return impl;

}


void ThreadImpl::set_thread(Thread* p,ThreadEx::factor_type v,int i)
{
	thrd_ptr=p;
	invoker=v;
	thrd_rank=i;
}


bool ThreadImpl::svc_enter()
{

	ThreadManager& tmgr(ThreadManager::current());
	{
		LockGuard<Mutex> lock(tmgr.m_thrd_mutex);
		if(!tmgr.m_nFlags.get(ThreadImpl::THREADMANAGER_DISABLED))
		{
			int nthread=++tmgr.m_nThreadNum;
			System::LogTrace("Thread created, %d threads",nthread);
		}
		else
		{
			delete this;
			return false;
		}
	}

	tc_impl_data=this;
	tc_init();

	return true;
}

void ThreadImpl::svc_leave()
{
	ThreadManager& tmgr(ThreadManager::current());

	tc_cleanup();
	tc_impl_data=NULL;

	delete this;

	{
		LockGuard<Mutex> lock1(tmgr.m_thrd_mutex);
		int nthread=--tmgr.m_nThreadNum;

		System::LogTrace("Thread destroyed, %d threads",nthread);

		if(nthread==0)
		{
			tmgr.m_cond_thrd_empty.notify_all();
		}
	}
}

void ThreadImpl::link_to(ThreadManager::ThreadLink *p)
{
	if(ptr_link==p) return;

	if(ptr_link) ptr_link->erase(this);
	ptr_link=p;
	if(ptr_link) ptr_link->push_front(this);
}

void ThreadImpl::svc()
{
	ThreadManager& tmgr(ThreadManager::current());
	for(;;)
	{
		{
			LockGuard<Mutex> lock1(tmgr.m_thrd_mutex);
			while(thrd_ptr==NULL)
			{
				if(tmgr.m_nFlags.get(ThreadImpl::THREADMANAGER_NOCAHCING|ThreadImpl::THREADMANAGER_DISABLED))
				{
					link_to(NULL);
					return;
				}
				link_to(&tmgr.list_free);
				tmgr.m_thrd_attached.wait(lock1);
			}
			link_to(&tmgr.list_work);
		}

		set_priority(50);
		set_affinity(thrd_affinity);


		thrd_ptr->test_canceled();
		{
			try
			{
				thrd_log.Id(thrd_rank);
				thrd_log.reset(Logger::def());

				if(!invoker)
				{
					thrd_ptr->svc();
				}
				else
				{
					invoker();
				}
			}
			catch(...)
			{
				thrd_ptr->on_exception();
			}

			tc_gc(0);
		}

		{
			LockGuard<Mutex> lock1(thrd_ptr->m_thrd_mutex);
			int tc=--thrd_ptr->m_nAlive;
			if(tc==0)
			{
				thrd_ptr->m_nState=0;
				thrd_ptr->m_cond_thrd_empty.notify_all();
			}
			thrd_ptr=NULL;
			invoker.clear();

		}

		{
			LockGuard<Mutex> lock1(tmgr.m_thrd_mutex);
			if (tmgr.m_nFlags.get(ThreadImpl::THREADMANAGER_NOCAHCING | ThreadImpl::THREADMANAGER_DISABLED))
			{
				link_to(NULL);
				return;
			}
			link_to(&tmgr.list_free);
		}
	}
}


CoroutineMain& ThreadImpl::cort_main()
{
	if (!cort_ptr)
	{
		cort_ptr.reset(new CoroutineMain);
		cort_ptr->init();
	}
	return *cort_ptr;
}

class ThreadMain : public Thread
{
public:

	ThreadImpl impl;

	ThreadMain()
	{
		m_nAlive=1;
		impl.thrd_rank=0;
		impl.thrd_ptr=this;
		tc_impl_data=&impl;
	}

	~ThreadMain()
	{

	}


	bool test_canceled(){return ThreadImpl::sm_bReqexit;}
	void cancel(){ThreadManager::current().close();}
	void wait(){ThreadManager::current().wait();}
	bool activate(){return false;}
	bool alive(){return true;}

};


class ThreadDummy : public Thread
{
public:

	ThreadImpl impl;

	ThreadDummy()
	{
		impl.thrd_rank=0;
		impl.thrd_ptr=this;
	}

	bool test_canceled(){return ThreadImpl::sm_bReqexit;}
	void cancel(){}
	void wait(){}
	bool activate(){return false;}
	bool alive(){return true;}

};



class ThreadManagerImpl : public ThreadManager, private ObjectInfo
{
public:

	ThreadManagerImpl() :ObjectInfo("ThreadManager"){}

	void DoInvoke(InvokeParam& ipm)
	{
		if (ipm.type == InvokeParam::TYPE_FINI)
		{
			close(true);
		}
	}

	ThreadMain thrd_main;
	ThreadDummy thrd_dummy;

	TimerQueue thrd_timer;
	ThreadPool thrd_pool;

	static ThreadManagerImpl& current();

	~ThreadManagerImpl(){close(true);}
};


ThreadManagerImpl* _g_pThreadManager=NULL;

inline ThreadManagerImpl& ThreadManagerImpl::current()
{
	if(!_g_pThreadManager)
	{
		static ThreadManagerImpl gInstance;
		_g_pThreadManager=&gInstance;
	}
	return *_g_pThreadManager;
}

ThreadManager& ThreadManager::current()
{
	return ThreadManagerImpl::current();
}

ThreadPool& ThreadPool::current()
{
	return ThreadManagerImpl::current().thrd_pool;
}

TimerQueue& TimerQueue::current()
{
	return ThreadManagerImpl::current().thrd_timer;
}

Thread& Thread::main_thread()
{
	return ThreadManagerImpl::current().thrd_main;
}

ThreadImpl& ThreadImpl::dummy_data()
{
	if(_g_pThreadManager==NULL)
	{
		return ThreadManagerImpl::current().thrd_main.impl;
	}
	else
	{
		return ThreadManagerImpl::current().thrd_dummy.impl;
	}	
}


EW_LEAVE
