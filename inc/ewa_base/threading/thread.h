
#ifndef __H_EW_THREAD__
#define __H_EW_THREAD__

#include "ewa_base/basic/bitflags.h"
#include "ewa_base/threading/thread_mutex.h"
#include "ewa_base/threading/thread_cond.h"
#include "ewa_base/collection/arr_1t.h"
#include "ewa_base/threading/thread_manager.h"
#include "ewa_base/collection/lockfree_queue.h"
#include "ewa_base/basic/functor.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE ThreadManager;
class DLLIMPEXP_EWA_BASE Logger;
class DLLIMPEXP_EWA_BASE ThreadImpl;

class DLLIMPEXP_EWA_BASE Thread : public Object
{
public:
	friend class ThreadManager;
	friend class ThreadImpl;

	Thread();

	// Copy construct and assign operator, NOTE these methods do NOT really copy any running threads.
	Thread(const Thread& o);
	Thread& operator=(const Thread&);

	virtual ~Thread();

	static Thread& main_thread();
	static Thread& this_thread();


	// Return current thread rank, from 0 to count()-1
	static int rank();

	// Set current thread priority, 0 lowest 100 highest
	static void set_priority(int n);

	// Bind current thread to core n
	static void set_affinity(int n);

	// Start one thread calling entry point
	virtual bool activate();

	// Default entry point
	virtual void svc();

	// Return true if thread exit flag is set
	// Block if thread pause flag is set
	virtual bool test_canceled();

	virtual bool is_canceled();

	// Sleep for a number of milliseconds.
	static void sleep_for(int milliseconds);

	// Sleep for a timespan.
	static void sleep_for(const TimeSpan& rel_time);

	// Sleep until timepoint.
	static void sleep_until(const TimePoint& abs_time);

	// A hint to the scheduler to let another thread run.
	static void yield();

	// Wait until thread terminates.
	void wait();

	// Wait until thread terminates or timeout.
	bool wait_for(int rel_time_in_ms);

	// Wait until thread terminates or timeout.
	bool wait_for(const TimeSpan& rel_time);

	// Wait until thread terminates or timepoint reached.
	bool wait_until(const TimePoint& abs_time);

	// Set the thread exit flag, causing member function test_canceled() return true;
	virtual void cancel();

	// Set the thread pasue flag, causing member function test_canceled() block.
	void pause();

	// Unset the thread pause flag.
	void resume();

	// Called while thread throws unhandled exception. can use try{throw;}catch... to rethrow the excetpion.
	virtual void on_exception();

	enum
	{
		STATE_PAUSED	=1<<0,	// thread pause flag
		STATE_CANCEL	=1<<1,	// thread exit flag
		STATE_ANY		=STATE_PAUSED|STATE_CANCEL,
	};

	enum
	{
		FLAG_DYNAMIC	=1<<0, // can start threads when alive()==true
	};

	// Running?
	bool alive();

	// Return running thread count.
	int count();

	// Thread flags, currently only support FLAG_DYNAMIC.
	// If FLAG_DYNAMIC is set, then new threads can be activated while alive.
	BitFlags flags;

	void bind_cpu(int c1);
	void bind_cpu(const arr_1t<int>& cu);

	static uintptr_t id();


protected:

	volatile int m_nState;
	volatile int m_nAlive;

	Mutex m_thrd_mutex;
	Condition m_cond_state_changed;
	Condition m_cond_thrd_empty;
	
	arr_1t<int> m_aBindCpu;

	bool _do_activate(size_t n);
	void _init();

	virtual void on_wait_noop();

};

class DLLIMPEXP_EWA_BASE ThreadMulti : public Thread
{
public:


	// Start 1 thread calling entry point
	virtual bool activate();

	// Start n threads calling entry point
	virtual bool activate(size_t n);
};

class DLLIMPEXP_EWA_BASE ThreadEx : public Thread
{
public:


	typedef Functor<void()> factor_type;

	typedef arr_1t<factor_type> InvokerGroup;

	// Start a group of threads.
	bool activate(InvokerGroup& g);

	// Start 1 thread calling fac().
	bool activate(const factor_type& fac,size_t n=1);

};

class DLLIMPEXP_EWA_BASE ThreadCustom : public ThreadEx
{
public:
	typedef ThreadEx basetype;

	// Start a group of threads calling entry points defined in m_aThreads.
	virtual bool activate();

protected:
	InvokerGroup m_aThreads;
};


EW_LEAVE

#endif
