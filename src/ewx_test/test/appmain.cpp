#include "ewa_base.h"


int main(int argc, char** argv)
{
	using namespace ew;

	mp_check_leak(1);

	System::SetLogFile("ewx_test.log",true);

	System::LogTrace("----  test enter   -------");
	TestMgr::current().Run(argc,argv);
	System::LogTrace("----  test leave   -------");

	ObjectInfo::Invoke(InvokeParam::TYPE_FINI);

	Console::Pause();
	return 0;
};
