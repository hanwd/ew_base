
#include "ewa_base/testing/test.h"
#include "ewa_base/basic.h"
#include "ewa_base/memory.h"
#include "ewa_base/threading.h"


using namespace ew;

template<typename MP>
class ThreadTestMemPool : public ThreadMulti
{
public:

	Logger logger;

	void test(const String& msg)
	{
		int n=System::GetCpuCount();
		if(n<2) n=2;

		TimePoint tp1=Clock::now();
		if(!activate(n)) return;

		wait();
		TimePoint tp2=Clock::now();

		logger.LogMessage("%s:%g sec used",msg,(tp2-tp1)/TimeSpan::Seconds(1));

	}

	void svc()
	{
		arr_1t<void*> h;

		MP& mp(MP::current());

		size_t n=1024*64;
		h.reserve(n*10);

		size_t memmax=256;

		logger.LogMessage("thread rank[%d] allocate memory",rank());

		for(size_t i=0; i<n*4; i++)
		{
			size_t sz=((size_t)::rand())%memmax;
			h.push_back(mp.allocate(sz));
		}

		logger.LogMessage("thread rank[%d] allocate/deallocate memory randomly",rank());

		for(size_t i=0; i<n*40; i++)
		{
			if(rand()%2==0)
			{
				size_t sz=((size_t)::rand()%memmax);
				h.push_back(mp.allocate(sz));
			}
			else if(!h.empty())
			{
				size_t kk=((size_t)::rand()%h.size());
				void *p=h[kk];
				h[kk]=h.back();
				h.pop_back();
				mp.deallocate(p);
			}
		}

		logger.LogMessage("thread rank[%d] deallocate memory",rank());

		for(size_t i=0; i<h.size(); i++)
		{
			mp.deallocate(h[i]);
		}
	}
};


TEST_DEFINE(TEST_Mempool)
{

	int n=System::GetCpuCount();
	if(n<2) n=2;

	TestMgr::current().logger.LogMessage("testing MemPool, %d threads allocate/deallocate together",n);

	ThreadTestMemPool<MemPoolPaging> mp;
	mp.test("mempool");

	ThreadTestMemPool<MemPoolMalloc> ht;
	ht.test("malloc");

}


