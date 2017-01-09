
#ifndef __H_EW_THREAD_MANAGER__
#define __H_EW_THREAD_MANAGER__

#include "ewa_base/threading/thread_cond.h"
#include "ewa_base/threading/thread_mutex.h"
#include "ewa_base/basic/pointer.h"
#include "ewa_base/basic/bitflags.h"
#include "ewa_base/basic/dlink.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE ThreadImpl;

class DLLIMPEXP_EWA_BASE ThreadManager : private NonCopyable
{
public:
	friend class ThreadImpl;

	void wait();
	void close(bool and_wait=false);

	int count();
	bool ok();
	static ThreadManager& current();

	typedef DLinkT<ThreadImpl> ThreadLink;


protected:

	bool ensure_free_size(size_t n);

	ThreadManager();
	~ThreadManager();

	int m_nThreadMax;
	BitFlags m_nFlags;
	AtomicInt32 m_nThreadNum;

	ThreadLink list_free, list_work;

	Condition m_cond_thrd_empty;
	Condition m_thrd_attached;
	Mutex m_thrd_mutex;

};

EW_LEAVE
#endif
