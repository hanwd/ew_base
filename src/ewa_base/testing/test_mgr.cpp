#include "ewa_base/testing/test.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/threading/thread.h"
#include "ewa_base/logging/logger.h"
#include "ewa_base/threading/thread_mutex.h"
#include "ewa_base/basic/lockguard.h"
#include "ewa_base/logging/logtarget.h"

EW_ENTER

Mutex test_msg_spin;

void TestMgr::Tested(bool v,const char* msg,const char* fn,int ln)
{
	LockGuard<Mutex> lock1(test_msg_spin);
	if(!m_pCurrentTest)
	{
		return;
	}

	if(v)
	{
		m_pCurrentTest->m_nPassed+=1;
	}
	else
	{
		m_pCurrentTest->m_nFailed+=1;
		logger.LogError("%s at %s:%d",msg,fn,ln);
	}
}


TestMgr::TestMgr()
{
	m_pCurrentTest=NULL;
}

TestMgr::~TestMgr()
{

}

void TestMgr::AddTest(Test* t)
{
	if(t==NULL) return;
	m_aTests.push_back(t);
}


void TestMgr::Run(int argc_,char** argv_)
{
	argc=argc_;
	argv=argv_;

	System::LogTrace("TestMgr::Run");

	//logger.reset(new LogConsole);

	m_nUnitPassed=0;
	m_nUnitFailed=0;
	m_nTestPassed=0;
	m_nTestFailed=0;


	arr_1t<Test*> tests;
	if (argc == 1)
	{
		tests = m_aTests;
	}
	else
	{
		for (arr_1t<Test*>::iterator it = m_aTests.begin(); it != m_aTests.end(); ++it)
		{
			if ((*it)->GetName() == argv[1])
			{
				tests.push_back(*it);
				break;
			}
		}

		if (tests.empty())
		{
			logger.LogMessage("Test(%s) Not Found!", argv[1]);
			return;
		}
	}

	logger.LogMessage("TestMgr: %d Tests",tests.size());
	logger.PrintLn("---------------------------------------");

	for(arr_1t<Test*>::iterator it=tests.begin(); it!=tests.end(); ++it)
	{
		m_pCurrentTest=(*it);

		(*it)->Run();
		m_nUnitPassed+=(*it)->m_nPassed;
		m_nUnitFailed+=(*it)->m_nFailed;
		if((*it)->m_nFailed==0)
		{
			m_nTestPassed+=1;
		}
		else
		{
			m_nTestFailed+=1;
		}

		m_pCurrentTest=NULL;

		logger.PrintLn("---------------------------------------");

	}


	if(m_nTestFailed>0)
	{
		logger.LogError("%d Tests, %d Pass, %d Failed",m_nTestPassed+m_nTestFailed,m_nTestPassed,m_nTestFailed);
	}
	else
	{
		logger.LogMessage("%d Tests, %d Pass, %d Failed",m_nTestPassed+m_nTestFailed,m_nTestPassed,m_nTestFailed);
	}

	logger.reset(NULL);

}

EW_LEAVE
