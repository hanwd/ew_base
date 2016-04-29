#include "ewa_base/testing/test.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/threading/thread.h"
#include "ewa_base/logging/logger.h"
#include "ewa_base/threading/thread.h"


EW_ENTER


Test::Test(const String& s):m_sName(s)
{
	TestMgr::current().AddTest(this);
}

void Test::Run()
{

	m_nPassed=0;
	m_nFailed=0;

	TestMgr::current().logger.LogMessage("Running Test: %s",m_sName);

	try
	{
		RunTest();
	}
	catch(...)
	{
		TestMgr::current().logger.LogError("unexpected exception");
		m_nFailed++;
	}
	
	if(m_nFailed>0)
	{
		TestMgr::current().logger.LogError("%d units, %d passed, %d failed",m_nPassed+m_nFailed,m_nPassed,m_nFailed);
	}
	else
	{
		TestMgr::current().logger.LogMessage("%d units, %d passed, %d failed",m_nPassed+m_nFailed,m_nPassed,m_nFailed);
	}

}


EW_LEAVE
