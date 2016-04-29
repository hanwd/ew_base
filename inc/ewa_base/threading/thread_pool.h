#ifndef __H_EW_THREAD_POOL__
#define __H_EW_THREAD_POOL__

#include "ewa_base/collection/intrusive_list.h"
#include "ewa_base/threading/thread.h"


EW_ENTER

class DLLIMPEXP_EWA_BASE ThreadPool;
class DLLIMPEXP_EWA_BASE ITimerHolder;

class DLLIMPEXP_EWA_BASE ITask : public ObjectData
{
public:
	virtual void svc(ThreadPool&){}
	virtual void svc(ITimerHolder&){}
};


class DLLIMPEXP_EWA_BASE ThreadPool : public Thread
{
public:
	typedef Thread basetype;

	ThreadPool();

	void putq(ITask* q);

	bool empty();
	void wait_for_empty(const TimeSpan& ts = TimeSpan::Day(365));

	bool activate();

	void reqexit();

	void set_worker_min(int n);
	void set_worker_max(int n);

	int get_worker_min();
	int get_worker_max();
	int get_worker_num();

	static ThreadPool& current();

protected:

	class TaskItem : public NonCopyable
	{
	public:
		TaskItem():next(NULL) {}
		TaskItem(ITask* q):hjob(q),next(NULL) {}

		DataPtrT<ITask> hjob;
		TaskItem* next;
	};

	Mutex m_tMutex;
	Condition m_tCond;
	Condition m_tEmpty;
	intrusive_list<TaskItem> m_tTaskQueue;

	int m_nWorkerWait;
	int m_nWorkerNum;
	int m_nWorkerMin;
	int m_nWorkerMax;
	int m_nTaskHint;

	TaskItem* getq();

	void svc();


};


EW_LEAVE
#endif
