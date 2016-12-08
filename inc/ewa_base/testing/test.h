
#ifndef __H_EW_TESTING_TEST__
#define __H_EW_TESTING_TEST__


#include "ewa_base/logging/logger.h"
#include "ewa_base/collection/arr_1t.h"

#define TEST_ASSERT_MSG(X,Y) \
	try\
	{\
		ew::TestMgr::current().Tested((X),Y,__FILE__,__LINE__);\
	}\
	catch(...)\
	{\
		ew::TestMgr::current().Tested(false,Y,__FILE__,__LINE__);\
	}

#define TEST_ASSERT(X) TEST_ASSERT_MSG(X,#X)

#define TEST_ASSERT_THROW(X,Y) \
	for(;;)\
	{\
		try{(X);}\
		catch(Y&)\
		{\
			TEST_ASSERT_MSG(true,#X);\
			break;\
		} \
		catch(...){}\
		TEST_ASSERT_MSG(false,#X);\
		break;\
	}\

#define TEST_ASSERT_THROW_ANY(X) \
	for(;;){ \
		try{(X);}\
		catch(...)\
		{\
			TEST_ASSERT_MSG(true,#X);\
			break;\
		} \
		TEST_ASSERT_MSG(false,#X);\
		break;\
	}\

#define TEST_ASSERT_EQ(X,Y) TEST_ASSERT_MSG((X)==(Y),#X"=="#Y" is FALSE")
#define TEST_ASSERT_NE(X,Y) TEST_ASSERT_MSG((X)!=(Y),#X"!="#Y" is FALSE")



#define TEST_DEFINE(X) \
	class Test##X : public ew::Test\
	{\
	protected:\
		Test##X():Test(#X){}\
	public:\
		virtual void RunTest();\
		static Test##X impl;\
	};\
	Test##X Test##X::impl;\
	void Test##X::RunTest()



EW_ENTER


//TEST_DEFINE(TEST_Sample)
//{
//	TEST_ASSERT(what);
//	TEST_ASSERT_MSG(what,message);
//  ...
//}

class DLLIMPEXP_EWA_BASE TestMgr;

class DLLIMPEXP_EWA_BASE Test : private NonCopyable
{
protected:
	Test(const String& s);
public:

	friend class TestMgr;
	virtual ~Test() {}

	virtual void Run();
	virtual void RunTest()=0;

protected:
	String m_sName;
	int m_nPassed;
	int m_nFailed;
};

class DLLIMPEXP_EWA_BASE TestMgr : private NonCopyable
{
	TestMgr();
	~TestMgr();
public:
	static TestMgr& current()
	{
		static TestMgr g;
		return g;
	}

	void AddTest(Test* t);
	void Run(int argc,char** argv);

	void Tested(bool v,const char* msg,const char* file,int line);

	Logger logger;

	int argc;
	char** argv;

protected:
	int m_nUnitPassed;
	int m_nUnitFailed;
	int m_nTestPassed;
	int m_nTestFailed;
	arr_1t<Test*> tests;
	Test* m_pCurrentTest;

};

EW_LEAVE

#endif
