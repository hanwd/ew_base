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

	template<typename G>
	static bool activate_t(Thread& thrd,G& g);

	static bool activate(Thread& thrd,size_t n);
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

	String last_error;

	CoroutineMain& cort_main();

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


	static void link_append(ThreadManager::ThreadLink& link, ThreadImpl* p);
	static void link_remove(ThreadManager::ThreadLink& link, ThreadImpl* p);
	static ThreadImpl* link_getone(ThreadManager::ThreadLink& link);
};


EW_LEAVE

