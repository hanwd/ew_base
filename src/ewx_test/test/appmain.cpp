#include "ewa_base.h"

using namespace ew;


int main(int argc,char** argv)
{

	mp_check_leak(1);

	System::LogTrace("----  process enter   -------");
	TestMgr::current().Run(argc,argv);
	System::LogTrace("----  process leave   -------");
	
	ThreadManager::current().close();
	ThreadManager::current().wait();

	Console::Pause();
	return 0;
};
