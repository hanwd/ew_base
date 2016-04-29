#include "ewa_base/testing/test.h"
#include "ewa_base/ipc.h"
#include <iostream>

using namespace ew;

TEST_DEFINE(TEST_Shm)
{

	SharedMem sm1,sm2;

	String filetext="helloworld";

	// create a file and write some text
	TEST_ASSERT(sm1.OpenFile("shm_sample.txt",1024,FileAccess::FLAG_RW|FileAccess::FLAG_CR));
	if(sm1.data())
	{
		strcpy(sm1.data(),filetext.c_str());
	}
	sm1.Close();
	TEST_ASSERT(sm1.data()==NULL);

	// open the file and read the text
	TEST_ASSERT(sm2.OpenFile("shm_sample.txt",0,FileAccess::FLAG_RD));
	if(sm1.data() && sm2.data())
	{
		TEST_ASSERT(strcmp(sm2.data(),filetext.c_str())==0);
	}
	sm2.Close();


	// open SharedMem with a name;

	TEST_ASSERT_MSG(sm1.Open("local_shm",1024,FileAccess::FLAG_RD|FileAccess::FLAG_WR|FileAccess::FLAG_CR),"ShmOpen");
	TEST_ASSERT_MSG(sm2.Open("local_shm",1024,FileAccess::FLAG_RD|FileAccess::FLAG_WR),"ShmOpen");
	char* p1=sm1.data();
	char* p2=sm2.data();

	if(p1 && p2)
	{
		TEST_ASSERT(p1!=p2);
		int *p=(int*)p1;
		for(size_t i=0; i<sm1.size()/sizeof(int); i++)
		{
			p[i]=rand();
		}

		TEST_ASSERT(memcmp(p1,p2,1024)==0);
	}
	else
	{
		TEST_ASSERT_MSG(false,"ShmOpen");
	}

	// open SharedMem without a name;
	TEST_ASSERT_MSG(sm2.Alloc(4096*8),"ShmOpen");

	p1=sm2.data();
	TEST_ASSERT_MSG(p1!=NULL,"FileAccess::Alloc failed");
	if(p1)
	{
		memset(p1,1,4096*8);
	}

	sm1.Close();
	sm2.Close();


}


