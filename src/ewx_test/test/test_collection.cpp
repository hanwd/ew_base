
#include "ewa_base.h"

using namespace ew;


TEST_DEFINE(TEST_arr_1t)
{

	arr_1t<int> hh;
	TEST_ASSERT(hh.empty());
	TEST_ASSERT(hh.size()==0);
	TEST_ASSERT(hh.capacity()==0);


	hh.push_back(3);
	TEST_ASSERT(hh.size()==1);
	TEST_ASSERT(hh[0]==3);

	hh.resize(100,2);
	TEST_ASSERT(hh.size()==100);
	TEST_ASSERT(hh[0]==3);

	int ne;

	ne=0;
	for(size_t i=1;i<hh.size();i++)
	{
		if(hh[i]!=2) ++ne;
	}
	TEST_ASSERT(ne==0);

	hh.resize(50);
	TEST_ASSERT(hh.size()==50);
	TEST_ASSERT(hh[0]==3);

	ne=0;
	for(size_t i=1;i<hh.size();i++)
	{
		if(hh[i]!=2) ++ne;
	}
	TEST_ASSERT(ne==0);


	hh.clear();
	for(size_t i=2;i<10;i++) hh.push_back(i);
	hh.shrink_to_fit();
	TEST_ASSERT(hh.capacity()==hh.size());

	// insert case 1
	hh.insert(hh.begin(),1);
	TEST_ASSERT(hh[0]==1);

	// insert case 2
	hh.insert(hh.begin(),0);
	TEST_ASSERT(hh[0]==0);
	TEST_ASSERT(hh[1]==1);

	arr_1t<int> h2(hh);
	TEST_ASSERT(h2==hh);

	hh.resize(5);
	for(size_t i=0;i<hh.size();i++) hh[i]=i+10;
	hh.reserve(20);

	// insert case 3
	hh.insert(hh.begin(),h2.begin(),h2.end());
	for(size_t i=0;i<hh.size();i++)
	{
		TEST_ASSERT(hh[i]==(int)i);
	}


	hh.clear();
	TEST_ASSERT(hh.size()==0);
	//TEST_ASSERT_THROW_ANY(hh.back());
	//TEST_ASSERT_THROW_ANY(hh.front());
	//TEST_ASSERT_THROW_ANY(hh.pop_back());

	hh.shrink_to_fit();
	TEST_ASSERT(hh.capacity()==0);

	int a[3]= {1,2,3};

// assign case 1
	hh.resize(100,4);

	hh.assign(a,a+3);
	TEST_ASSERT(hh.size()==3);
	TEST_ASSERT(hh[0]==a[0]);
	TEST_ASSERT(hh[1]==a[1]);
	TEST_ASSERT(hh[2]==a[2]);

// assign case 2
	hh.clear();
	hh.assign(a,a+3);
	TEST_ASSERT(hh.size()==3);
	TEST_ASSERT(hh[0]==a[0]);
	TEST_ASSERT(hh[1]==a[1]);
	TEST_ASSERT(hh[2]==a[2]);

// assign case 3
	hh.clear();
	hh.reserve(5);
	hh.push_back(6);
	hh.assign(a,a+3);
	TEST_ASSERT(hh.size()==3);
	TEST_ASSERT(hh[0]==a[0]);
	TEST_ASSERT(hh[1]==a[1]);
	TEST_ASSERT(hh[2]==a[2]);

	hh.clear();

	hh.insert(hh.begin(),a,3);
	hh.insert(hh.begin()+1,a,2);

	TEST_ASSERT(hh[0]==1);
	TEST_ASSERT(hh[1]==1);
	TEST_ASSERT(hh[2]==2);
	TEST_ASSERT(hh[3]==2);
	TEST_ASSERT(hh.back()==3);
	hh.erase(hh.begin()+2,hh.end());
	TEST_ASSERT(hh.size()==2);
	TEST_ASSERT(hh.back()==1);


	arr_1t<String> hs;
	hs.push_back("hello");
	hs.push_back("world");
	hs.insert(hs.begin()+1," ");

	TEST_ASSERT(hs[0]=="hello");
	TEST_ASSERT(hs[1]==" ");
	TEST_ASSERT(hs[2]=="world");

}

template<template<typename,typename,typename> class P>
void test_bst_set()
{
	std::vector<int> aInts;
	for(int i=0; i<1024; ++i)
	{
		aInts.push_back(i);
	}
	std::random_shuffle(aInts.begin(),aInts.end());

	typedef bst_set<int,std::less<int>,def_allocator,P> bst_set_type;
	bst_set_type s;

	s.insert(aInts.begin(),aInts.end());


	TEST_ASSERT(s.size()==1024);
	TEST_ASSERT(s.find(9)!=s.end());
	TEST_ASSERT(s.count(9)==1);
	s.erase(9);
	TEST_ASSERT(s.find(9)==s.end());
	TEST_ASSERT(s.count(9)==0);
	TEST_ASSERT(s.size()==1023);

	TEST_ASSERT(s.insert(1).second==false);
	TEST_ASSERT(s.insert(2048).second==true);

	bst_set_type s2(s);
	TEST_ASSERT(s==s2);

	s.clear();
	TEST_ASSERT(s.size()==0);

}

template<template<typename,typename,typename> class P>
void test_bst_multiset()
{
	std::vector<int> aInts;
	for(int i=0; i<1024; ++i)
	{
		aInts.push_back(i);
	}
	for(int i=0; i<1024; ++i)
	{
		aInts.push_back(i);
	}
	std::random_shuffle(aInts.begin(),aInts.end());


	typedef bst_multiset<int,std::less<int>,def_allocator,P> bst_set_type;
	bst_set_type s;

	s.insert(aInts.begin(),aInts.end());

	TEST_ASSERT(s.size()==2048);
	TEST_ASSERT(s.find(9)!=s.end());
	TEST_ASSERT(s.count(9)==2);
	s.erase(9);
	TEST_ASSERT(s.find(9)==s.end());
	TEST_ASSERT(s.count(9)==0);
	TEST_ASSERT(s.size()==2046);

	s.insert(1);
	TEST_ASSERT(s.count(1)==3);

	std::pair<typename bst_set_type::iterator,typename bst_set_type::iterator> eqr(s.equal_range(1));
	for(typename bst_set_type::iterator it=eqr.first; it!=eqr.second; ++it)
	{
		TEST_ASSERT((*it)==1);
	}

	bst_set_type s2(s);
	TEST_ASSERT(s==s2);

	s.clear();
	TEST_ASSERT(s.size()==0);

}


TEST_DEFINE(TEST_Collection)
{
	test_bst_set<rbt_trait>();
	test_bst_set<avl_trait>();
	test_bst_multiset<rbt_trait>();
	test_bst_multiset<avl_trait>();

	indexer_set<String> sh;

	sh.insert("hello");
	sh.insert("world");

	TEST_ASSERT(sh.size()==2);
	TEST_ASSERT(sh.get(0)=="hello");
	TEST_ASSERT(sh.get(1)=="world");
	TEST_ASSERT(sh.find1("hello")==0);
	TEST_ASSERT(sh.find1("world")==1);
	TEST_ASSERT(sh.find1("adfs")==-1);

	indexer_map<String,int> sm;

	sm["a"]=1;
	sm["b"]=4;
	sm["c"]=2;

	TEST_ASSERT(sm["a"]==1);
	TEST_ASSERT(sm["b"]==4);
	sm.rehash(12425);
	TEST_ASSERT(sm["a"]==1);
	TEST_ASSERT(sm["b"]==4);

	sm.erase("b");
	TEST_ASSERT(sm.find1("b")==-1);

	ew::arr_xt<double> arr;

	//TEST_ASSERT_THROW(arr.resize((size_t)(-1)),std::bad_alloc);

	arr.resize(6,5,4,3,2,1);
	arr(3,3)=3.25;
	for(size_t i=0; i<6; i++)
	{
		TEST_ASSERT(arr.size(i)==6-i);
	}

	TEST_ASSERT(arr.size()==6*5*4*3*2*1);

	arr.resize(6,6);


	TEST_ASSERT(arr(3,3)==3.25);
	TEST_ASSERT(arr.size()==36);
	arr.clear();

	TEST_ASSERT(arr.size()==0);

};



class Thread_SRSW_Queue : public ThreadMulti
{
public:

	SRSW_Queue<int> q;

	Thread_SRSW_Queue()
	{
		q.resize(1024*32);
	}

	void svc_reader()
	{
		int v;
		for(int i=0;i<1024*1024;i++)
		{
			while(!q.popq(v))
			{
				if(test_canceled())
				{
					TEST_ASSERT_MSG(false,"svc_reader exit");
					return;
				}
				AtomicSpin::noop();
			}
			if(v!=i)
			{
				TEST_ASSERT_MSG(false,"svc_reader failed");
				cancel();
				return;
			}
		}

		TEST_ASSERT_MSG(true,"svc_reader done");
	}

	void svc_writer()
	{
		for(int i=0;i<1024*1024;i++)
		{
			while(!q.push(i))
			{
				if(test_canceled())
				{
					TEST_ASSERT_MSG(false,"svc_writer exit");
					return;
				}
				AtomicSpin::noop();
			}
		}
		TEST_ASSERT_MSG(true,"svc_writer done");
	}

	void svc()
	{
		int r=rank();
		if(r==0)
		{
			svc_writer();
		}
		else if(r==1)
		{
			svc_reader();
		}
	}

};


TEST_DEFINE(TEST_SRSW_Queue)
{
	Thread_SRSW_Queue q;
	q.activate(2);
	q.wait();
}
