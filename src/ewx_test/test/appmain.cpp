#include "ewa_base.h"
#include <cstdio>
#include <algorithm>



int main(int argc,char** argv)
{
	using namespace ew;

	mp_check_leak(1);

	System::SetLogFile("ew.log",true);

	System::LogTrace("----  process enter   -------");
	TestMgr::current().Run(argc,argv);
	System::LogTrace("----  process leave   -------");

	ThreadManager::current().close();
	ThreadManager::current().wait();

	Console::Pause();
	return 0;
};
