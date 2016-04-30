#include "ewa_base/threading/thread.h"
#include "ewa_base/logging/logger.h"
#include "ewa_base/threading/coroutine.h"
#include "thread_impl_detail.h"

EW_ENTER

extern EW_THREAD_TLS ThreadImpl* tc_impl_data;


class DLLIMPEXP_EWA_BASE MpAllocCachedNoLock;

class DLLIMPEXP_EWA_BASE ThreadImpl : public mp_obj, private NonCopyable
{
public:

	enum
	{
		THREADMANAGER_DISABLED	=1<<0,
		THREADMANAGER_NOCAHCING	=1<<1,
	};

	static ThreadImpl* get_thread();
	static bool put_thread(ThreadImpl* impl);

	static bool activate(Thread& thrd,int n);
	static bool activate(Thread& thrd,ThreadEx::InvokerGroup& g);

	ThreadImpl();
	~ThreadImpl();

	void set_thread(Thread* p,ThreadEx::factor_type v,int i);

	void svc();
	bool svc_enter();
	void svc_leave();
	bool create();

	void set_priority(int n);
	void set_affinity(int n);


	LitePtrT<Thread> thrd_ptr;
	AutoPtrT<CoroutineMain> cort_ptr;
	Logger thrd_log;

	CoroutineMain& cort_main()
	{
		if(!cort_ptr)
		{
			cort_ptr.reset(new CoroutineMain);
			cort_ptr->init();
		}
		return *cort_ptr;
	}

	int thrd_rank;
	int thrd_priority;
	int thrd_affinity;

	bool thrd_created;

	ThreadImpl_detail::thread_t thrd_id;
	ThreadEx::factor_type invoker;


	LitePtrT<ThreadImpl> pNext;
	LitePtrT<ThreadImpl> pPrev;

	static bool sm_bReqexit;

	static ThreadImpl& dummy_data();

	static inline ThreadImpl& this_data()
	{
		if(!tc_impl_data)
		{
			ThreadManager::current();
			if(tc_impl_data) return *tc_impl_data;

			tc_impl_data=&dummy_data();
			System::LogTrace("unknown thread(%p) ThreadImpl::this_data()",(void*)Thread::id());
		}
		return *tc_impl_data;
	}
};



EW_LEAVE

