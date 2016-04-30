#include "ewa_base.h"

int main(int argc,char** argv)
{

	using namespace ew;
	mp_check_leak(1);

	System::LogTrace("----  process enter   -------");
	TestMgr::current().Run(argc,argv);
	System::LogTrace("----  process leave   -------");
	
	ThreadManager::current().close();
	ThreadManager::current().wait();

	return 0;
};
