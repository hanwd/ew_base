
#include "ewa_base/testing/test.h"
#include "ewa_base/basic.h"
#include "ewa_base/threading.h"
#include "ewa_base/math/math_def.h"
#include "ewa_base/basic/scanner_helper.h"
#include "ewa_base/scripting/variant_op.h"
#include "ewa_base/basic/formater.h"
#include "ewa_base/basic/misc.h"

#include <time.h>

using namespace ew;


TEST_DEFINE(TEST_String)
{

	String s1;
	String s2("hello");

	TEST_ASSERT(s1=="");
	TEST_ASSERT(s1.empty());
	TEST_ASSERT(s1.size()==0);
	TEST_ASSERT(s1.c_str()[0]==0);

	TEST_ASSERT(s2=="hello");
	TEST_ASSERT(!s2.empty());
	TEST_ASSERT(s2.size()==5);
	TEST_ASSERT(s1.c_str()[5]==0);

	TEST_ASSERT(s1!=s2);

	s1=s2;
	TEST_ASSERT(s1==s2);

	String s3="world";
	String s4="helloworld";

	TEST_ASSERT(s4.substr(0,5)=="hello");
	TEST_ASSERT(s4.substr(5)=="world");
	TEST_ASSERT(s4.substr(2,2)=="ll");

	TEST_ASSERT(s3.size()==5);

	String s5(s2);
	s5+=s3;
	String s6;
	s6<<s2<<s3;

	String::Format("%s%s",s2,s3);

	String s7=String::Format("%s%s",s2,s3);


	TEST_ASSERT(s2+s3==s4);
	TEST_ASSERT(s5==s4);
	TEST_ASSERT(s6==s4);
	TEST_ASSERT(s7==s4);

	String s8;

	s8="";
	s8<<'A';
	TEST_ASSERT(s8=="A");

	s8="";
	s8<<"hello";
	TEST_ASSERT(s8=="hello");

	s8<<"world";
	TEST_ASSERT(s8=="helloworld");

	s8.replace('l','x');
	TEST_ASSERT(s8=="hexxoworxd");

	s8.replace("x","yy");
	TEST_ASSERT(s8=="heyyyyoworyyd");


	int32_t i32=12345678;
	s8="";
	s8<<i32;
	TEST_ASSERT(s8=="12345678");

	s8="";
	s8<<-i32;
	TEST_ASSERT(s8=="-12345678");

	int64_t i64=123456890123456890;
	s8="";
	s8<<i64;
	TEST_ASSERT(s8=="123456890123456890");
	s8="";
	s8<<-i64;
	TEST_ASSERT(s8=="-123456890123456890");


	uint32_t u32=12345678;
	s8="";
	s8<<u32;
	TEST_ASSERT(s8=="12345678");

	uint64_t u64=123456890123456890;
	s8="";
	s8<<u64;
	TEST_ASSERT(s8=="123456890123456890");

	float32_t f32=1.25,f32_2;
	s8="";
	s8<<f32;
	TEST_ASSERT(s8.ToNumber(&f32_2) && f32_2==f32);

	float64_t f64=2.125,f64_2;
	s8="";
	s8<<f64;
	TEST_ASSERT(s8.ToNumber(&f64_2) && f64_2==f64);

}


TEST_DEFINE(TEST_StringFormatEx)
{
	TEST_ASSERT(String::Format("%{2}%{1}","hello","world")=="worldhello");
	TEST_ASSERT(String::Format("%{1, *d}",3,9)=="  9");
	TEST_ASSERT(String::Format("%{1}%{3}%{1}%{2}",1,2,3,4)=="1312");
	TEST_ASSERT(String::Format("%d %ccs32342")=="%d %ccs32342");
	TEST_ASSERT(String::Format("%d %ccs32342",1)=="1 %ccs32342");
	TEST_ASSERT(String::Format(L"%{1}%{2}",1,2)=="12");
	TEST_ASSERT(String::Format(L"%{1}%{2}","1",L"2")=="12");
	TEST_ASSERT(String::Format("%{1}%{2}",L"1",L"2")=="12");

	int32_t v1,v2;
	String s1=String::Format("hello%{2,n}%{1,d}%{3,n}world",123,&v1,&v2);
	TEST_ASSERT(v1==5);
	TEST_ASSERT(v2==8);

}

TEST_DEFINE(TEST_StringBuffer)
{

	StringBuffer<char> s1;
	StringBuffer<char> s2("hello");
	TEST_ASSERT(s1=="");
	TEST_ASSERT(s1.size()==0);
	TEST_ASSERT(s1!=s2);
	TEST_ASSERT(s2.size()==5);
	s1=s2;
	TEST_ASSERT(s1==s2);

	StringBuffer<char> s3="world";
	StringBuffer<char> s4="helloworld";


	TEST_ASSERT(s3.size()==5);

	StringBuffer<char> s5(s2);
	s5+=s3;
	StringBuffer<char> s6;
	s6<<s2<<s3;

	StringBuffer<char> s7=String::Format("%s%s",s2,s3);

	TEST_ASSERT(s2+s3==s4);
	TEST_ASSERT(s5==s4);
	TEST_ASSERT(s6==s4);
	TEST_ASSERT(s7==s4);

	StringBuffer<char> s8;

	s8="";
	s8<<'A';
	TEST_ASSERT(s8=="A");

	s8="";
	s8<<"hello";
	TEST_ASSERT(s8=="hello");

	s8<<"world";
	TEST_ASSERT(s8=="helloworld");

	int32_t i32=12345678;
	s8="";
	s8<<i32;
	TEST_ASSERT(s8=="12345678");

	int64_t i64=123456890123456890;
	s8="";
	s8<<i64;
	TEST_ASSERT(s8=="123456890123456890");

	uint32_t u32=12345678;
	s8="";
	s8<<u32;
	TEST_ASSERT(s8=="12345678");

	uint64_t u64=123456890123456890;
	s8="";
	s8<<u64;
	TEST_ASSERT(s8=="123456890123456890");

	float32_t f32=1.25;
	s8="";
	s8<<f32;

	float64_t f64=2.125;
	s8="";
	s8<<f64;


}


TEST_DEFINE(TEST_BitFlags)
{

	enum
	{
		FLAG1=1<<0,
		FLAG2=1<<1,
		FLAG3=1<<2
	};

	BitFlags flags;
	TEST_ASSERT(flags.val()==0);

	flags.add(FLAG1);
	TEST_ASSERT(flags.val()==FLAG1);
	TEST_ASSERT(flags.get(FLAG1));
	TEST_ASSERT(!flags.get(FLAG2));

	flags.set(FLAG2,true);
	TEST_ASSERT(flags.get(FLAG1));
	TEST_ASSERT(flags.get(FLAG2));

	flags.inv(FLAG1);
	TEST_ASSERT(!flags.get(FLAG1));
	TEST_ASSERT(flags.get(FLAG2));

	flags.clr(FLAG3);
	TEST_ASSERT(!flags.get(FLAG2));
	TEST_ASSERT(flags.get(FLAG3));

}

TEST_DEFINE(TEST_Clock)
{
	time_t n=time(NULL);

	TimePoint p1;
	p1.SetTime(n);

	TimePoint p2=p1+TimeSpan::Seconds(120);
	TimeSpan s1=p2-p1;
	TEST_ASSERT(p1+s1==p2);
	TEST_ASSERT(s1.GetSeconds()==120);
	TEST_ASSERT(s1/TimeSpan::Minutes(1)==2.0);
	TEST_ASSERT(p2>p1);

	struct tm* ptm=localtime(&n);
	TimeDetail td(p1,TimeDetail::LOCAL);

	TEST_ASSERT(td.GetYear()==ptm->tm_year+1900);
	TEST_ASSERT(td.GetMonth()==ptm->tm_mon+1);
	TEST_ASSERT(td.GetDay()==ptm->tm_mday);

}


int func0()
{
	return 0;
}

int func1(int v1)
{
	return v1*2;
}

int func2(int v1,int v2)
{
	return v1*2+v2*3;
}

int func3(int v1,int v2,int v3)
{
	return v1*2+v2*3+v3*5;
}

int func4(int v1,int v2,int v3,int v4)
{
	return v1*2+v2*3+v3*5+v4*7;
}

int func5(int v1,int v2,int v3,int v4,int v5)
{
	return v1*2+v2*3+v3*5+v4*7+v5*11;
}


TEST_DEFINE(TEST_Functor)
{

	TEST_ASSERT(ew::hbind(func0)()==func0());
	TEST_ASSERT(ew::hbind(func1,ew::_1)(2)==func1(2));
	TEST_ASSERT(ew::hbind(func2,ew::_1,3)(2)==func2(2,3));
	TEST_ASSERT(ew::hbind(func3,ew::_1,3,6)(2)==func3(2,3,6));
	TEST_ASSERT(ew::hbind(func4,ew::_1,3,_1,4)(2)==func4(2,3,2,4));
	TEST_ASSERT(ew::hbind(func5,1,2,3,4,5)()==func5(1,2,3,4,5));

	ew::Functor<int()> fac0;
	ew::Functor<int(int)> fac1;
	ew::Functor<int(int,int)> fac2;
	fac0.bind(&func5,1,2,3,4,5);
	TEST_ASSERT(fac0()==func5(1,2,3,4,5));

	fac1.bind(&func5,1,2,3,4,ew::_1);
	TEST_ASSERT(fac1(10)==func5(1,2,3,4,10));
	fac1.bind(&func5,ew::_1,1,2,3,4);
	TEST_ASSERT(fac1(10)==func5(10,1,2,3,4));

	fac2.bind(&func5,ew::_1,1,ew::_2,3,4);
	TEST_ASSERT(fac2(10,3)==func5(10,1,3,3,4));

}



TEST_DEFINE(TEST_Basic_other)
{

	Console::SetColor(Console::COLOR_R|Console::COLOR_S);
	Console::WriteLine("red text");
	Console::SetColor(Console::COLOR_B|Console::COLOR_S);
	Console::WriteLine("blue text");
	Console::SetColor(Console::COLOR_G|Console::COLOR_S);
	Console::WriteLine("green text");
	Console::SetColor(Console::COLOR_D);
	Console::WriteLine("white text");

	Console::WriteLine(
		String::Format("cores: %d, pagesize: %d, cacheline: %d",
					   System::GetCpuCount(),
					   System::GetPageSize(),
					   System::GetCacheLineSize()));

}



template<typename T>
class MyObjectT : public Object
{
public:
	T impl;
	DECLARE_OBJECT_INFO(MyObjectT,ObjectInfo)
};

IMPLEMENT_OBJECT_INFO_NAME_T1(MyObjectT,ObjectInfo,"myobj")


TEST_DEFINE(TEST_Object)
{

	MyObjectT<int32_t> my_obj;
	String sObjName=my_obj.GetObjectName();
	TEST_ASSERT(sObjName=="myobj#i32");


	ObjectCreator& objcreator(ObjectCreator::current());

	Object* obj=objcreator.Create("Object");
	Object* dat=objcreator.Create("ObjectData");
	Object* myobj=objcreator.Create("myobj#i32");

	TEST_ASSERT(obj!=NULL);
	TEST_ASSERT(dynamic_cast<ObjectData*>(dat)!=NULL);
	TEST_ASSERT(dynamic_cast<MyObjectT<int32_t>*>(myobj)!=NULL);

	delete obj;
	delete dat;
	delete myobj;

};



template<typename T>
class Counter
{
public:
	Counter(){m_nCount++;}
	Counter(const Counter&){m_nCount++;}
	~Counter(){m_nCount--;}

	static int m_nCount;
};

template<typename T>
int Counter<T>::m_nCount=0;


class Object1 : public Object
{
public:
	Counter<Object1> impl;
};

class Object2 : public Object1
{
public:
	Counter<Object2> impl;
};


TEST_DEFINE(TEST_Pointer)
{

	TEST_ASSERT(Counter<Object1>::m_nCount==0);
	TEST_ASSERT(Counter<Object2>::m_nCount==0);

	AutoPtrT<Object1> p1(new Object2);
	TEST_ASSERT(Counter<Object1>::m_nCount==1);
	TEST_ASSERT(Counter<Object2>::m_nCount==1);

	AutoPtrT<Object1> p2(new Object1);
	TEST_ASSERT(Counter<Object1>::m_nCount==2);
	TEST_ASSERT(Counter<Object2>::m_nCount==1);

	p1.reset(p2.release());
	TEST_ASSERT(p2.get()==NULL);
	TEST_ASSERT(Counter<Object1>::m_nCount==1);
	TEST_ASSERT(Counter<Object2>::m_nCount==0);

	p1.reset(NULL);
	TEST_ASSERT(Counter<Object1>::m_nCount==0);
	TEST_ASSERT(Counter<Object2>::m_nCount==0);

	arr_1t<Object1*> aObjects;
	for(size_t i=0;i<1024;i++)
	{
		aObjects.push_back(new Object1);
		TEST_ASSERT(Counter<Object1>::m_nCount==aObjects.size());
	}
	std::for_each(aObjects.begin(),aObjects.end(),[](Object1* p)
	{
		delete p;
	});
	aObjects.clear();

	TEST_ASSERT(Counter<Object1>::m_nCount==0);

	SharedPtrT<Object2> p3;
	TEST_ASSERT(p3==NULL);

	p3.reset(new Object2);
	TEST_ASSERT(Counter<Object1>::m_nCount==1);
	TEST_ASSERT(Counter<Object2>::m_nCount==1);
	TEST_ASSERT(p3.use_count()==1);
	TEST_ASSERT(p3.unique());

	WeakPtrT<Object1> p4(p3);

	TEST_ASSERT(Counter<Object1>::m_nCount==1);
	TEST_ASSERT(Counter<Object2>::m_nCount==1);
	TEST_ASSERT(p3.use_count()==1);
	TEST_ASSERT(!p4.expired());

	SharedPtrT<Object1> p5=p4.lock();
	TEST_ASSERT(Counter<Object1>::m_nCount==1);
	TEST_ASSERT(Counter<Object2>::m_nCount==1);
	TEST_ASSERT(p3.use_count()==2);
	TEST_ASSERT(p5.get()!=NULL);

	p3.reset();
	TEST_ASSERT(Counter<Object1>::m_nCount==1);
	TEST_ASSERT(Counter<Object2>::m_nCount==1);
	p5.reset();
	TEST_ASSERT(Counter<Object1>::m_nCount==0);
	TEST_ASSERT(Counter<Object2>::m_nCount==0);

	SharedPtrT<Object1> p6=p4.lock();
	TEST_ASSERT(p6.get()==NULL);
	TEST_ASSERT(p4.expired());

	p4.reset();

}


template<typename ATOMIC_TYPE>
class ThreadAtomic : public ThreadMulti
{
public:
	static const int N=1024*64;

	ATOMIC_TYPE val;
	void svc()
	{
		for(int i=0;i<N;i++) val++;
	}

	void test()
	{
		val=0;
		if(!activate(4)) return;
		wait();
		TEST_ASSERT(val==4*N);
	}
};

template<typename ATOMIC_TYPE>
void test_atomic()
{
	ATOMIC_TYPE val;
	TEST_ASSERT(val==0);

	TEST_ASSERT(val++==0);
	TEST_ASSERT(val==1);
	TEST_ASSERT(++val==2);
	TEST_ASSERT(val==2);

	TEST_ASSERT(val--==2);
	TEST_ASSERT(val==1);
	TEST_ASSERT(--val==0);
	TEST_ASSERT(val==0);

	TEST_ASSERT(val.exchange(-1)==0);
	TEST_ASSERT(val==-1);
	typename ATOMIC_TYPE::type expected=2;

	TEST_ASSERT(!val.compare_exchange(expected,-3));
	TEST_ASSERT(val==-1);
	TEST_ASSERT(expected==-1);

	expected=val;
	TEST_ASSERT(val.compare_exchange(expected,-3));
	TEST_ASSERT(val==-3);


	val=0;
	TEST_ASSERT(val.fetch_add(3)==0);
	TEST_ASSERT(val==3);
	TEST_ASSERT(val.fetch_sub(3)==3);
	TEST_ASSERT(val==0);


	val=8;
	TEST_ASSERT(val.fetch_or(9)==8);
	TEST_ASSERT(val==9);
	TEST_ASSERT(val.fetch_and(1)==9);
	TEST_ASSERT(val.load()==1);
	TEST_ASSERT(val.fetch_xor(1)==1);
	TEST_ASSERT(val==0);

	val.store(10);
	TEST_ASSERT(val.get()==10);

	ThreadAtomic<ATOMIC_TYPE> thrd;
	thrd.test();

}

TEST_DEFINE(TEST_Atomic)
{
	test_atomic<AtomicInt32>();
	test_atomic<AtomicUint32>();
	test_atomic<AtomicInt64>();
	test_atomic<AtomicUint64>();
}

