#include "ewa_base.h"
#include "apihook.h"

class MyThread : public ew::Thread
{
public:

	void svc()
	{
		while(1)
		{
			::printf("live\n");
			sleep_for(2000);
		}
	}
};

int main(int argc, char** argv)
{
	using namespace ew;

	MyThread t;
	t.activate();
	Thread::sleep_for(5000);
	ThreadManager::current().stop_the_world();
	::printf("stop\n");
	Thread::sleep_for(5000);
	ThreadManager::current().resume_the_world();
	::printf("resume\n");

	Thread::sleep_for(5000);

	mp_check_leak(1);
	mp_alloc(1,__FILE__,__LINE__);

	Logger::def()->flags.add(LogTarget::FLAG_SHOWRANK|LogTarget::FLAG_SHOWALL);

	System::SetLogFile("ewx_test.log",true);

	System::LogTrace("----  test enter   -------");
	TestMgr::current().Run(argc,argv);
	System::LogTrace("----  test leave   -------");

	ThreadManager::current().close(true);

	Console::Pause();
	return 0;
};
