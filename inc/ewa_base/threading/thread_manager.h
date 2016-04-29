
#ifndef __H_EW_THREAD_MANAGER__
#define __H_EW_THREAD_MANAGER__

#include "ewa_base/threading/thread_cond.h"
#include "ewa_base/threading/thread_mutex.h"
#include "ewa_base/basic/pointer.h"
#include "ewa_base/basic/bitflags.h"


EW_ENTER

class DLLIMPEXP_EWA_BASE ThreadImpl;

class DLLIMPEXP_EWA_BASE ThreadManager : private NonCopyable
{
public:
	friend class ThreadImpl;

	void wait();
	void close();
	int count();

	bool ok();

	static ThreadManager& current();

protected:

	ThreadManager();
	~ThreadManager();

	int m_nThreadMax;
	BitFlags m_nFlags;
	AtomicInt32 m_nThreadNum;
	AtomicInt32 m_nThreadJob;

	Mutex m_thrd_mutex;
	LitePtrT<ThreadImpl> m_pThreads_free;
	Condition m_cond_thrd_empty;
	Condition m_thrd_attached;

};

EW_LEAVE
#endif
