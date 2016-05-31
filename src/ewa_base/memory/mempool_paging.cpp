
#include "mempool_impl.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/collection/bst_set.h"
#include "ewa_base/collection/bst_map.h"
#include "ewa_base/memory/gc_ptr.h"

#include <cstdio>


EW_ENTER

volatile int32_t MpAllocInfo::ALLOC_NNUM=0;

int32_t MpAllocInfo::ALLOC_BREAK=-1;
int32_t MpAllocInfo::ALLOC_CHECK_LEAK=0;

void mp_break_alloc(int n)
{
	MpAllocInfo::ALLOC_BREAK=n;
}

void mp_check_leak(int n)
{
	 MpAllocInfo::ALLOC_CHECK_LEAK=n;
}

void MpAllocGlobal::init()
{
	spanalloc.alloc_batch();

	size_t k1=0;
	size_t k2=0;
	for(size_t i=0;i<sl_size;i++)
	{
		size_t sz=FixAllocUnits[i];
		aSlots[i].init(sz);

		if(sz<=sz_slot1)
		{
			size_t kn=sz>>sz_bits1;
			for(size_t k=k1;k<=kn;k++)
			{
				pSlot1[k]=&aSlots[i];
			}
			k1=kn+1;
		}
		else if(sz<=sz_slot2)
		{
			size_t kn=sz>>sz_bits2;
			for(size_t k=k2;k<=kn;k++)
			{
				pSlot2[k]=&aSlots[i];
			}
			k2=kn+1;
		}
	}

}

class MpFileLine
{
public:
	MpFileLine(){}
	MpFileLine(const char* f,int l):file(f),line(l){}
	const char* file;
	int line;
};
bool operator<(const MpFileLine& lhs,const MpFileLine& rhs)
{
	if(lhs.file<rhs.file) return true;
	if(lhs.file>rhs.file) return false;
	return lhs.line<rhs.line;
}

class MpMemoryInfo
{
public:
	void* data;
	uint32_t size;
	MpMemoryInfo* next;
};


template <typename T,bool S=false>
class FixedSizeAllocator : public AllocatorBase<T>
{
public:

	typedef std::size_t size_type;
	typedef T *pointer;

	MpFixedSizePool<sizeof(T),S> cached;
public:

	FixedSizeAllocator() throw() {}
	FixedSizeAllocator(const FixedSizeAllocator&) {}

	template <typename T2>
	FixedSizeAllocator(const FixedSizeAllocator<T2,S> &) throw(){}

	~FixedSizeAllocator() throw() {}


	inline pointer allocate(size_type n)
	{
		if(n!=1) Exception::XBadAlloc();
		pointer p=(pointer)cached.alloc();
		if(!p) Exception::XBadAlloc();
		return p;
	}

	inline void deallocate(pointer p, size_type)
	{
		if(!p) return;
		cached.dealloc(p);
	}

	template <typename T2>
	struct rebind{typedef FixedSizeAllocator<T2,S> other;};

	bool operator!=(const FixedSizeAllocator &other) const{return this!=&other;}
	bool operator==(const FixedSizeAllocator &other) const{return this==&other;}

};

class MpAllocLeakDetector : public NonCopyable
{
public:
	typedef bst_set<MpAllocInfo*,std::less<MpAllocInfo*>,FixedSizeAllocator<int> > set_type;
	typedef bst_map<MpFileLine,set_type,std::less<MpFileLine>,FixedSizeAllocator<int> > map_type;

	map_type mpLeakInfoMap;
	char* pCache;
	int nLeakCount;

	MpAllocLeakDetector()
	{
		nLeakCount=0;
		pCache=(char*)page_alloc(1024*1024);
	}

	~MpAllocLeakDetector()
	{
		if(pCache)
		{
			page_free(pCache,1024*1024);
		}
	}

	bool ok()
	{
		return pCache!=NULL;
	}

	void OutputString(const char* s)
	{
		System::LogTrace(s);

#ifdef EW_MSVC
		::OutputDebugStringW(IConv::to_wide(s).c_str());
		::OutputDebugStringW(L"\n");
#endif

	}

	void leak(void* p,size_t n)
	{

#ifdef EW_CHECK_HEAP
		MpAllocInfo::check(p,n);
#endif
		MpAllocInfo* pinfo=MpAllocInfo::get(p,n);
		if(pinfo)
		{
			MpAllocInfo& info(*pinfo);
			MpFileLine fl(info.file,info.line);
			mpLeakInfoMap[fl].insert(pinfo);
		}
		else
		{
			nLeakCount++;

			::sprintf(pCache,"mp_alloc:leak detected: %p,%d",p,(int)n);
			OutputString(pCache);
		}
	}

	void test(MpAllocSpan* sp)
	{
		if(!sp) return;

		if(sp->sl_slot)
		{
			MpAllocSlot& sl(*sp->sl_slot);
			memset(pCache,0,sl.nd_nall);
			for(MpAllocNode* h=sp->nd_free;h;h=h->nd_next)
			{
				size_t d=(uintptr_t(h)-sp->sp_base)/sl.nd_size;
				if(d<sl.nd_nall)
				{
					pCache[d]=1;
				}
				else
				{
					System::LogError("mp_alloc:MpAllocNode in incorrect Slot detected");
				}
			}

			for(size_t i=0;i<sl.nd_nall;i++)
			{
				if(pCache[i]!=0) continue;
				leak((void*)(sp->sp_base+i*sl.nd_size),sl.nd_size);
			}

		}
		else
		{
			if(sp->sp_flag.get_fly())
			{
				leak((void*)(sp->sp_base),sp->sp_size);
			}
			else
			{
				leak((void*)(sp->sp_base),sp->sp_size);
			}
		}
	}


	void report()
	{
		if(mpLeakInfoMap.empty())
		{
			if(nLeakCount==0)
			{
				::sprintf(pCache,"mp_alloc:no leak detected");
				OutputString(pCache);
			}
			return;
		}

		for(map_type::iterator it=mpLeakInfoMap.begin();it!=mpLeakInfoMap.end();++it)
		{

			set_type &st((*it).second);
			set_type::iterator si=st.begin();
			set_type::iterator se=st.end();

			if(si==se)
			{
				continue;
			}

			char* p1=pCache;
			p1+=sprintf(p1,"mp_alloc:leak detected, file:%s, line: %d, count:%d {",(*it).first.file,(int)(*it).first.line,(int)(*it).second.size());

			for(size_t i=0;;)
			{
				p1+=sprintf(p1,"%d",(*si)->nnum);
				if(++si!=se)
				{
					*p1++=',';
					if(++i==16)
					{
						*p1++='.';
						*p1++='.';
						*p1++='.';
						break;
					}
				}
				else
				{
					break;
				}
			}

			*p1++='}';
			*p1++='\0';

			OutputString(pCache);
		}
	}
};



void MpAllocGlobal::cleanup()
{
	gc(-1);

	if(MpAllocInfo::ALLOC_CHECK_LEAK==0)
	{
		return;
	}

	try
	{
		MpAllocLeakDetector mpld;

#ifdef EW_X86

		for(size_t i=0;i<pgmap.size1;i++)
		{
			mpld.test(pgmap.buckets[i].get());
		}
#else
		for(size_t j=0;j<pgmap.size2;j++)
		{
			pagemap_type::BucketLevel2* bk=pgmap.buckets2[j];
			if(!bk) continue;
			for(size_t i=0;i<pgmap.size1;i++)
			{
				mpld.test((*bk)[i].get());
			}
		}
#endif

		mpld.report();

	}
	catch(...)
	{

	}

	AtomicSpin::noop();

}


MpAllocNode* MpAllocGlobal::dealloc_batch_nolock(MpAllocSlot& sl,MpAllocNode* fp,size_t sz)
{
	while(fp)
	{
		if(sz==0)
		{
			return fp;
		}
		sz--;

		MpAllocNode* fn=fp;fp=fp->nd_next;
		MpAllocBucket* pbk=pgmap.find_bucket(fn);

		if(!pbk||pbk->get()->sl_slot!=&sl)
		{
			System::LogError("dealloc_batch failed");
			break;
		}

		MpAllocSpan* sp=pbk->get();
		fn->nd_next=sp->nd_free;
		sp->nd_free=fn;

		if(!fn->nd_next)
		{
			sp->sl_next=sl.sp_head;
			sp->sl_prev=NULL;
			sl.sp_head=sp;
			if(sp->sl_next)
			{
				sp->sl_next->sl_prev=sp;
			}
		}

		if(--sp->nd_nnum!=0||sp->sl_next==NULL)
		{
			continue;
		}

		if(sl.sp_head==sp)
		{
			sl.sp_head=sp->sl_next;
			sp->sl_next->sl_prev=NULL;
		}
		else
		{
			sp->sl_next->sl_prev=sp->sl_prev;
			sp->sl_prev->sl_next=sp->sl_next;
		}


		pbk->set(NULL);

		if(sp->sp_flag.get_fly())
		{
			page_free(reinterpret_cast<void*>(sp->sp_base),sp->sp_size);
			//span_spin.lock();
			spanalloc.dealloc(sp);
			//span_spin.unlock();
		}
		else
		{
			page_free(reinterpret_cast<void*>(sp->sp_base),sp->sp_size);
		}
	}

	return NULL;
}


MpAllocNode* MpAllocGlobal::alloc_small_batch(MpAllocSlot& sl,size_t& sz)
{
	sl.sl_spin.lock();

	if(!sl.sp_head)
	{
		sl.sp_head=create_span_nolock(sl);
		if(!sl.sp_head)
		{
			sl.sl_spin.unlock();
			return NULL;
		}
	}

	sz=sl.nd_nall-sl.sp_head->nd_nnum;

	MpAllocNode* nd=sl.sp_head->nd_free;
	sl.sp_head->nd_free=NULL;
	sl.sp_head->nd_nnum=sl.nd_nall;

	sl.sp_head=sl.sp_head->sl_next;
	if(sl.sp_head)
	{
		sl.sp_head->sl_prev=NULL;
	}


	sl.sl_spin.unlock();
	return nd;
}



void* MpAllocGlobal::realloc_real(void* p,size_t n,MpAllocBucket& bk)
{
	MpAllocSpan* sp=bk.get();
	size_t n0=0;
	if(sp->sl_slot)
	{
		n0=sp->sl_slot->nd_size;
	}
	else if(sp->sp_flag.get_fly())
	{
		n0=sp->sp_size-sizeof(MpAllocSpan);
	}
	else
	{
		n0=sp->sp_size;
	}

#ifdef EW_CHECK_HEAP
	if(n<=n0)
	{
		return p;
	}
	void* m=mp_alloc_real(n);
#else
	void* m=alloc(n);
#endif

	if(!m)
	{
		return NULL;
	}

	memcpy(m,p,n0);
	dealloc_real(p,bk);
	return m;
}


void MpAllocGlobal::dealloc_real(void* p,MpAllocBucket& bk)
{
	MpAllocSpan* sp=bk.get();
	if(sp->sl_slot)
	{

		MpAllocSlot& sl(*sp->sl_slot);

#ifdef EW_CHECK_HEAP
		MpAllocInfo::check(p,sl.nd_size);
#endif
		sl.sl_spin.lock();

		MpAllocNode* fn=(MpAllocNode*)p;
		fn->nd_next=sp->nd_free;
		sp->nd_free=fn;

		if(!fn->nd_next)
		{
			sp->sl_next=sl.sp_head;
			sp->sl_prev=NULL;
			sl.sp_head=sp;
			if(sp->sl_next)
			{
				sp->sl_next->sl_prev=sp;
			}
		}

		if(--sp->nd_nnum!=0||sp->sl_next==NULL)
		{
			sl.sl_spin.unlock();
			return;
		}

		if(sl.sp_head==sp)
		{
			sl.sp_head=sp->sl_next;
			sp->sl_next->sl_prev=NULL;
		}
		else
		{
			sp->sl_next->sl_prev=sp->sl_prev;
			sp->sl_prev->sl_next=sp->sl_next;
		}

		sl.sl_spin.unlock();

		return;
	}

	bk.set(NULL);

#ifdef EW_CHECK_HEAP
	MpAllocInfo::check(p,sp->sp_size);
#endif

	if(sp->sp_flag.get_fly())
	{
		page_free(reinterpret_cast<void*>(sp->sp_base),sp->sp_size);
		//span_spin.lock();
		spanalloc.dealloc(sp);
		//span_spin.unlock();
	}
	else
	{
		page_free(reinterpret_cast<void*>(sp->sp_base),sp->sp_size+sizeof(MpAllocSpan));
	}

}


void* MpAllocGlobal::alloc_small(size_t n,MpAllocSlot& sl)
{
	EW_ASSERT(n<=sl.nd_size);

	sl.sl_spin.lock();

	if(!sl.sp_head)
	{
		sl.sp_head=create_span_nolock(sl);
		if(!sl.sp_head)
		{
			sl.sl_spin.unlock();
			return NULL;
		}
	}

	MpAllocNode* nd=sl.sp_head->nd_free;
	sl.sp_head->nd_free=nd->nd_next;
	sl.sp_head->nd_nnum++;

	if(!sl.sp_head->nd_free)
	{
		sl.sp_head=sl.sp_head->sl_next;
		if(sl.sp_head)
		{
			sl.sp_head->sl_prev=NULL;
		}
	}
	sl.sl_spin.unlock();
	return nd;
}

void* MpAllocGlobal::alloc_small(size_t n,MpAllocSlot& sl,MpAllocInfo& i)
{
	EW_ASSERT(n<=sl.nd_size);

	sl.sl_spin.lock();

	if(!sl.sp_head)
	{
		sl.sp_head=create_span_nolock(sl);
		if(!sl.sp_head)
		{
			sl.sl_spin.unlock();
			return NULL;
		}
	}

	MpAllocNode* nd=sl.sp_head->nd_free;
	sl.sp_head->nd_free=nd->nd_next;
	sl.sp_head->nd_nnum++;

	if(!sl.sp_head->nd_free)
	{
		sl.sp_head=sl.sp_head->sl_next;
		if(sl.sp_head)
		{
			sl.sp_head->sl_prev=NULL;
		}
	}
	sl.sl_spin.unlock();

	i.set(nd,sl.nd_size);

	return nd;
}

void* MpAllocGlobal::alloc_large(size_t n)
{
	size_t sz=(n+MpAllocConfig::pg_mask)&(~MpAllocConfig::pg_mask);
	char* p1=(char*)page_alloc(sz);

	if(sz-n>=sizeof(MpAllocSpan))
	{
		MpAllocSpan* sp=(MpAllocSpan*)(p1+sz-sizeof(MpAllocSpan));
		sp->sp_base=reinterpret_cast<uintptr_t>(p1);
		sp->sp_size=sz-sizeof(MpAllocSpan);
		sp->sp_flag.set_fly(false);
		sp->sl_slot=NULL;

		pgmap.insert_span_nolock(sp);
		return p1;
	}
	else
	{
		MpAllocSpan* sp;
		{
			//span_spin.lock();
			sp=(MpAllocSpan*)spanalloc.alloc();
			//span_spin.unlock();
		}

		if(!sp)
		{
			page_free(p1,sz);
			return NULL;
		}

		sp->sp_base=reinterpret_cast<uintptr_t>(p1);
		sp->sp_size=sz;
		sp->sl_slot=NULL;
		sp->sp_flag.set_fly(true);

		pgmap.insert_span_nolock(sp);
		return p1;
	}
}

void* MpAllocGlobal::alloc_large(size_t n,MpAllocInfo& i)
{

	size_t sz=(n+MpAllocConfig::pg_mask+sizeof(MpAllocInfo))&(~MpAllocConfig::pg_mask);
	char* p1=(char*)page_alloc(sz);

	if(sz-n-sizeof(MpAllocInfo)>=sizeof(MpAllocSpan))
	{
		MpAllocSpan* sp=(MpAllocSpan*)(p1+sz-sizeof(MpAllocSpan));
		sp->sp_base=reinterpret_cast<uintptr_t>(p1);
		sp->sp_size=sz-sizeof(MpAllocSpan);
		sp->sp_flag.set_fly(false);
		sp->sl_slot=NULL;

		pgmap.insert_span_nolock(sp);
		i.set(p1,sz-sizeof(MpAllocSpan));

		return p1;
	}
	else
	{
		MpAllocSpan* sp;
		{
			//span_spin.lock();
			sp=(MpAllocSpan*)spanalloc.alloc();
			//span_spin.unlock();
		}

		if(!sp)
		{
			page_free(p1,sz);
			return NULL;
		}

		sp->sp_base=reinterpret_cast<uintptr_t>(p1);
		sp->sp_size=sz;
		sp->sl_slot=NULL;
		sp->sp_flag.set_fly(true);

		i.set(p1,sz);

		pgmap.insert_span_nolock(sp);
		return p1;
	}
}

void* MpAllocGlobal::alloc(size_t n,MpAllocInfo& i)
{
	MpAllocSlot* psl=get_slot(n+sizeof(MpAllocInfo));
	if(!psl)
	{
		return alloc_large(n,i);
	}
	else
	{
		return alloc_small(n,*psl,i);
	}
}

MpAllocSpan* MpAllocGlobal::create_span_nolock(MpAllocSlot& sl)
{
	size_t sz=MpAllocConfig::sp_size;

	MpAllocNode* p1=(MpAllocNode*)page_alloc(sz);

	if(!p1)
	{
		return NULL;
	}

	MpAllocSpan* sp;
	if(sl.sp_flag.get_fly())
	{
		//span_spin.lock();

		sp=(MpAllocSpan*)spanalloc.alloc();
		if(!sp)
		{
			page_free(p1,sz);
			//span_spin.unlock();
			return NULL;
		}
		sp->sp_flag.set_fly(true);

		//span_spin.unlock();
	}
	else
	{
		sp=(MpAllocSpan*)(((char*)p1)+sz-sizeof(MpAllocSpan));
		sp->sp_flag.set_fly(false);
	}

	sp->sp_base=reinterpret_cast<uintptr_t>(p1);
	sp->sp_size=sz;
	sp->nd_nnum=0;
	sp->nd_free=p1;
	sp->sl_slot=&sl;
	sp->sl_next=NULL;
	sp->sl_prev=NULL;

	MpAllocNode::link(p1,sl.nd_nall*sl.nd_size,sl.nd_size,NULL);

	pgmap.insert_span_nolock(sp);
	return sp;
}


void MpAllocGlobal::gc(int level)
{

	for(size_t i=0;i<sl_size;i++)
	{
		MpAllocSlot& sl(aSlots[i]);
		if(!sl.sl_spin.try_lock())
		{
			continue;
		}

		MpAllocSpan* sp=sl.sp_head;
		while(sp)
		{
			if(sp->nd_nnum!=0)
			{
				sp=sp->sl_next;
				continue;
			}

			MpAllocSpan* kk=sp;
			sp=sp->sl_next;

			if(sp) sp->sl_prev=kk->sl_prev;
			if(kk->sl_prev)
			{
				kk->sl_prev->sl_next=sp;
			}
			else
			{
				sl.sp_head=sp;
			}

			MpAllocBucket* pbk=g_myalloc_impl->pgmap.find_bucket(reinterpret_cast<void*>(kk->sp_base));

			if(pbk)
			{
				EW_ASSERT(pbk->get()==kk);
				pbk->set(NULL);
			}
			else
			{
				System::LogError("MpAllocSpan not in pgmap");
			}

			if(kk->sp_flag.get_fly())
			{
				page_free(reinterpret_cast<void*>(kk->sp_base),kk->sp_size);
				spanalloc.dealloc(kk);
			}
			else
			{
				page_free(reinterpret_cast<void*>(kk->sp_base),kk->sp_size);
			}
		}
		sl.sl_spin.unlock();
	}

}


MpAllocGlobal *g_myalloc_impl=NULL;



void mp_cleanup()
{
	System::LogTrace("mp_cleanup");

	if(!g_myalloc_impl) return;
	g_myalloc_impl->cleanup();
}

void mp_init()
{
	if(!g_myalloc_impl)
	{

#if defined(EW_WINDOWS) && defined(_DEBUG)
		_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)|_CRTDBG_LEAK_CHECK_DF);
#endif

		MpAllocGlobal* p=(MpAllocGlobal*)page_alloc(sizeof(MpAllocGlobal));
		if(!p)
		{
			System::LogFatal("Cannot initialize MpAllocGlobal");
		}
		p->init();
		g_myalloc_impl=p;

		::atexit(mp_cleanup);
		::atexit(tc_cleanup);

		//System::LogTrace("mp_init");
	}
}


void* mp_realloc(void* p,size_t n)
{
	void* m=g_myalloc_impl->realloc(p,n);
	if(!m&&n>0)
	{
		errno=ENOMEM;
	}
	return m;
}


void* mp_alloc(size_t n)
{
	void* p=mp_alloc_real(n);
	if(!p)
	{
		errno=ENOMEM;
	}
	return p;
}

void* mp_alloc(size_t n,const char* f,int l)
{
	void* p=mp_alloc_real(n,f,l);
	if(!p)
	{
		errno=ENOMEM;
	}
	return p;
}

void mp_free(void* p)
{
	mp_free_real(p);
}

static const intptr_t pagesize=4096;
static const intptr_t pagemask=pagesize-1;

void* mp_alloc_with_page_protect(size_t n)
{
	size_t n1=(n+pagemask)&~pagemask;
	char* p=(char*)mp_alloc_real(n+pagesize);
	if(!p)
	{
		errno=ENOMEM;
	}
	page_protect(p+n1,pagesize,0);
	return p+n1-n;
}

void mp_free_with_page_protect(void* p)
{
	p=(void*)(intptr_t(p)&~pagemask);
	MpAllocBucket* pbk=g_myalloc_impl->pgmap.find_bucket(p);
	if(pbk)		
	{
		MpAllocSpan* sp=pbk->get();
		size_t nd_size=sp->sl_slot?sp->sl_slot->nd_size:sp->sp_size;
		EW_ASSERT(nd_size>=pagesize && nd_size%pagesize==0);
		page_protect(p,nd_size,FileAccess::FLAG_RW);
	}
	mp_free_real(p);
}

void mp_force_gc(int level)
{
	tc_gc(level);
	g_myalloc_impl->gc(level);
}

void* MemPoolPaging::allocate(size_t n)
{
	void* p=mp_alloc_real(n);
	if(!p)
	{
		Exception::XBadAlloc();
	}
	return p;
}

void* MemPoolPaging::allocate(size_t n,const char* f,int l)
{
	void* p=mp_alloc_real(n,f,l);
	if(!p)
	{
		Exception::XBadAlloc();
	}
	return p;
}

void MemPoolPaging::deallocate(void* p)
{
	mp_free_real(p);
}

EW_LEAVE
