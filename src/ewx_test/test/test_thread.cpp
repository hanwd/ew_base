#include "ewa_base/testing/test.h"
#include "ewa_base/threading.h"
#include "ewa_base/basic.h"

using namespace ew;

bool hello_run=false;
bool a_my_run=false;

void hello()
{
	hello_run=true;
}

class AAA
{
public:
	void my_run()
	{
		a_my_run=true;
	}
};

class ThreadTest2 : public ThreadMulti
{
public:

	void svc()
	{
		TEST_ASSERT(&this_thread()==this);
		while(!test_canceled())
		{
			Thread::sleep_for(100);
		}
	}
};

bool thread_a_svc=false;

class ThreadA : public Thread
{
public:
	void svc()
	{
		thread_a_svc=true;
	}
};


class ITimerTest : public ITask
{
public:

	void svc(ITimerHolder& h)
	{
		TimePoint t1=Clock::now();
		int dt=(t1-h.due()).GetMilliSeconds();
		Console::WriteLine(String::Format("dis: %d ms",dt));
	}
};


TEST_DEFINE(TEST_TIMER)
{
	Logger logger;
	logger.LogMessage("timer test enter");

	TimerQueue& tq(TimerQueue::current());

	tq.activate(1);

	DataPtrT<ITask> q(new ITimerTest);

	tq.putq(q.get(),TimeSpan::MilliSeconds(200));
	tq.putq(q.get(),TimeSpan::MilliSeconds(250));
	tq.putq(q.get(),TimeSpan::MilliSeconds(1250));
	tq.putq(q.get(),TimeSpan::MilliSeconds(3250));
	tq.putq(q.get(),TimeSpan::MilliSeconds(3260));
	tq.putq(q.get(),TimeSpan::MilliSeconds(3270));

	tq.wait_for_empty();

	logger.LogMessage("timer test leave");
}

TEST_DEFINE(TEST_Thread)
{
	Logger logger;
	logger.LogMessage("thread test enter");
	ThreadA thrd1;
	thrd1.activate();
	thrd1.wait();
	TEST_ASSERT(thread_a_svc);

	ThreadTest2 thrd2;
	thrd2.activate(10);
	TEST_ASSERT(thrd2.alive());
	TEST_ASSERT(thrd2.count()==10);
	TEST_ASSERT(!thrd2.activate(3));
	thrd2.flags().add(Thread::FLAG_DYNAMIC);
	TEST_ASSERT(thrd2.activate(3));
	TEST_ASSERT(thrd2.count()==13);

	thrd2.cancel();
	thrd2.wait();
	logger.LogMessage("thrd2 test leave");

	TEST_ASSERT(!thrd2.alive());
	TEST_ASSERT(thrd2.count()==0);

	AAA aa;
	ThreadEx thrd3;
	thrd3.flags().add(Thread::FLAG_DYNAMIC);
	thrd3.activate(&hello);

	auto fn = hbind(&AAA::my_run,&aa);
	thrd3.activate(fn);
	thrd3.wait();

	TEST_ASSERT(a_my_run);
	TEST_ASSERT(hello_run);


}


class Product : public ObjectData
{
public:
	Product(int v=0):value(v) {}
	int value;
};


class RoutineProducer : public Coroutine
{
public:

	LitePtrT<Coroutine> Consumer;

	void sink(int val)
	{
		this_logger().LogMessage("produce %d, yielding to consumer",val);
		Product* p=new Product(val);
		TEST_ASSERT(yield(Consumer,p));
		TEST_ASSERT(p->value==val*2);
	}

	int val;

	RoutineProducer()
	{
		val=1;
	}

	void svc()
	{
		TEST_ASSERT(Coroutine::main_coroutine().state()==Coroutine::STATE_PAUSED);
		TEST_ASSERT(&Coroutine::this_coroutine()==this);
		TEST_ASSERT(state()==Coroutine::STATE_RUNNING);

		sink(val++);
		sink(val++);
		sink(val++);
		sink(val++);
		sink(val++);

		return;
	}

};



class RoutineConsumer : public Coroutine
{
public:

	void svc()
	{
		for(;;)
		{
			Product* p=dynamic_cast<Product*>(extra());
			if(p)
			{
				this_logger().LogMessage("consume %d",p->value);
				p->value*=2;

				yield_last(p);
			}
			else
			{
				this_logger().LogMessage("consumer exit");
				return;
			}
		}
	}

};



TEST_DEFINE(TEST_Coroutine)
{
	RoutineProducer producer;
	RoutineConsumer consumer;

	TEST_ASSERT(producer.state()==Coroutine::STATE_STOPPED);
	TEST_ASSERT(consumer.state()==Coroutine::STATE_STOPPED);
	TEST_ASSERT(Coroutine::main_coroutine().state()==Coroutine::STATE_RUNNING);
	TEST_ASSERT(&Coroutine::main_coroutine()==&Coroutine::this_coroutine());

	producer.Consumer=&consumer;

	this_logger().LogMessage("spawn producer");
	TEST_ASSERT(Coroutine::spawn(&producer));

	this_logger().LogMessage("spawn consumer");
	TEST_ASSERT(Coroutine::spawn(&consumer));

	TEST_ASSERT(producer.state()==Coroutine::STATE_PAUSED);
	TEST_ASSERT(consumer.state()==Coroutine::STATE_PAUSED);

	this_logger().LogMessage("yielding to producer");
	Coroutine::yield(&producer);


	TEST_ASSERT(consumer.state()==Coroutine::STATE_PAUSED);
	TEST_ASSERT(producer.state()==Coroutine::STATE_STOPPED);

	TEST_ASSERT(!Coroutine::yield(&producer));

	this_logger().LogMessage("spawn producer again");
	Coroutine::spawn(&producer);

	this_logger().LogMessage("yielding to producer again");
	TEST_ASSERT(Coroutine::yield(&producer));

	this_logger().LogMessage("yielding to consumer with NULL");
	Coroutine::yield(&consumer,NULL);

	TEST_ASSERT(consumer.state()==Coroutine::STATE_STOPPED);
}


class ThreadSync : public ThreadMulti
{
public:

	size_t size;
	size_t type;
	size_t result;

	ThreadSync()
	{
		size=4;
	}

	Functor<void()> svc_handler;

	template<typename L>
	void svc_real(L& thelock)
	{
		for(size_t i=0; i<1024*1024/size; i++)
		{
			thelock.lock();
			result++;
			thelock.unlock();
		}
	}

	template<typename L>
	void test(const String& s)
	{
		L thelock;
		svc_handler.bind(&ThreadSync::svc_real<L>,this,ew::mk_ref(thelock));

		TimePoint tp1=Clock::now();
		result=0;
		this->activate(size);
		this->wait();

		TimePoint tp2=Clock::now();

		svc_handler.clear();
		Console::WriteLine(String::Format("test:%s:%g ms",s,(tp2-tp1)/TimeSpan::MilliSeconds(1)));
	}

	void svc()
	{
		svc_handler();
	}
};



TEST_DEFINE(TEST_ThreadOther)
{
	ThreadSync thrd;


	thrd.test<ew::AtomicSpin>("atomic_spin");
	TEST_ASSERT(thrd.result==1024*1024);

	thrd.test<ew::AtomicMutex>("atomic_mutex");
	TEST_ASSERT(thrd.result==1024*1024);

	thrd.test<ew::SpinLock>("spin");
	TEST_ASSERT(thrd.result==1024*1024);

	thrd.test<ew::Mutex>("mutex");
	TEST_ASSERT(thrd.result==1024*1024);


}
