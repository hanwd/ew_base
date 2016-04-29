#include "stdio.h"


__declspec(dllexport) void dll_test(int argc, char** argv);


void dll_test(int argc, char** argv)
{
	

	printf(argv[0]);

};
