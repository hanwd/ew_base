#include "ewa_base/config.h"
#include "ewa_base/memory/mempool.h"
#include "ewa_base/basic/lockguard.h"
#include "ewa_base/basic/atomic.h"
#include "ewa_base/basic/system.h"

#include <errno.h>

EW_ENTER

void mp_init();
void mp_cleanup();
void tc_init();
void tc_cleanup();
void tc_gc(int level);


#define SPLIT_UNIT_N(X) ((1024*1024/X)&~4095)

static const size_t FixAllocUnits[]=
{
#ifdef EW_X86
	4,
#endif
	8,16,24,32,48,64,96,128,192,256,384,512,1024,1024*2,1024*4,1024*8,
	1024*16,1024*24,1024*32,1024*64,
	SPLIT_UNIT_N(8),SPLIT_UNIT_N(6),SPLIT_UNIT_N(4),SPLIT_UNIT_N(3),SPLIT_UNIT_N(2),SPLIT_UNIT_N(1)
};

class MpAllocInfo
{
public:
	MpAllocInfo(){}
	MpAllocInfo(const char* f,int l):line(l),file(f)
	{
		nnum=-1;
	}

	int32_t flag;
	int32_t line;
	int32_t nnum;
	uint32_t size;
	const char* file;

	void set(void* p,size_t n)
	{
#ifdef EW_CHECK_HEAP
		memset(p,0xCD,n);
#endif
		((MpAllocInfo*)(((char*)p)+n))[-1]=*this;

	}

	static inline void check(void* p,size_t n)
	{
		MpAllocInfo* pinfo=get(p,n);
		if(!pinfo)
		{
			System::LogTrace("heap corruption detected, address:%p, alloc_size:%d ",p,(int)n);

#ifdef EW_CHECK_HEAP_BREAK
			System::DebugBreak();
#endif
			return;
		}

		for(uint8_t* p1=((uint8_t*)p)+pinfo->size;p1<(uint8_t*)pinfo;++p1)
		{
			if(*p1!=0xCD)
			{
				if(pinfo->line>0)
				{
					System::LogTrace(
						"heap corruption detected, address:%p, alloc_size:%d, exact_size:%d, file:%s, line:%d"
						,p,(int)n,(int)pinfo->size,pinfo->file,(int)pinfo->line);
				}
				else
				{
					System::LogTrace("heap corruption detected, address:%p, alloc_size:%d, exact_size:%d ",p,(int)n,(int)pinfo->size);
				}

#ifdef EW_CHECK_HEAP_BREAK
				System::DebugBreak();
#endif
				return;
			}
		}
	}

	static MpAllocInfo* get(void* p,size_t n)
	{
		if(n<sizeof(MpAllocInfo))
		{
			return NULL;
		}

		MpAllocInfo* i=((MpAllocInfo*)(((char*)p)+n))-1;
		if(i->flag==ALLOC_FLAG)
		{
			return i;
		}

		return NULL;
	}

	static const int32_t ALLOC_FLAG=0x12345678;
	static volatile int32_t ALLOC_NNUM;

	static int32_t ALLOC_BREAK;
	static int32_t ALLOC_CHECK_LEAK;

	static int32_t num()
	{
		int32_t  n=(*(AtomicInt32*)&ALLOC_NNUM)++;
		return n;
	}
};

class MpAllocConfig
{
public:

	static const size_t sp_bits=20;
	static const size_t sp_size=1<<sp_bits;
	static const size_t sp_mask=sp_size-1;

	static const size_t pg_size=1024*4;
	static const size_t pg_mask=pg_size-1;

	static const size_t bk_more=0;	//sp_size/max(FixAllocUnits)-1;

	typedef AtomicSpin spin_type;
	typedef LockGuard<AtomicSpin> lock_type;


};

class MpAllocFlag
{
public:

	MpAllocFlag():val(0){}

	uintptr_t val;

	bool get_fly(){return val!=0;}
	void set_fly(bool f){val=f?1:0;}

};



class MpAllocSlot;

class MpAllocSpan
{
public:

	uintptr_t sp_base;
	uintptr_t sp_size;
	MpAllocFlag sp_flag;

	MpAllocSpan* sl_next;
	MpAllocSpan* sl_prev;
	MpAllocSlot* sl_slot;

	// free linklist
	MpAllocNode* nd_free;

	// allocated nodes count
	uintptr_t nd_nnum;

};

class MpAllocSlot
{
public:

	MpAllocSpan* sp_head;
	uint32_t nd_size;
	uint32_t nd_nall;
	MpAllocFlag sp_flag;
	MpAllocConfig::spin_type sl_spin;

	void init(size_t nd);

};


class MpAllocBucket
{
public:
	AtomicIntT<MpAllocSpan*>  span;
	MpAllocSpan* get()
	{
		return span.get();
	}
	void set(MpAllocSpan* sp)
	{
		span.store(sp);
	}
};


template<size_t BITS>
class RadixPageMapT;

template<>
class RadixPageMapT<32>
{
public:
	static const uintptr_t bits1=32-20;
	static const uintptr_t size1=1<<bits1;
	static const uintptr_t mask1=size1-1;

	MpAllocBucket buckets[size1];

	void insert_span_nolock(MpAllocSpan* sp)
	{
		uintptr_t bp=sp->sp_base>>MpAllocConfig::sp_bits;
		EW_ASSERT(buckets[bp].get()==NULL);
		buckets[bp].set(sp);
	}

	void remove_span_nolock(MpAllocSpan* sp)
	{
		uintptr_t bp=sp->sp_base>>MpAllocConfig::sp_bits;
		EW_ASSERT(buckets[bp].get()==sp);
		buckets[bp].set(NULL);
	}

	inline MpAllocBucket* find_bucket(void* p)
	{
		uintptr_t kp=reinterpret_cast<uintptr_t>(p);
		uintptr_t bp=kp>>MpAllocConfig::sp_bits;

		MpAllocSpan* sp=buckets[bp].get();
		if(sp)
		{
			if(kp>=sp->sp_base)
			{
				EW_ASSERT(kp-sp->sp_base<sp->sp_size);
				return &buckets[bp];
			}
		}

		if(bp==0) return NULL;

		// fall back 1 bucket
		sp=buckets[--bp].get();
		if(sp)
		{
			if(kp-sp->sp_base<sp->sp_size)
			{
				return &buckets[bp];
			}
			else
			{
				return NULL;
			}
		}

		// fall back more buckets, depends on
		for(size_t i=0;i<MpAllocConfig::bk_more;i++)
		{
			if(bp==0) return NULL;

			sp=buckets[--bp].get();
			if(sp)
			{
				if(kp-sp->sp_base<sp->sp_size)
				{
					return &buckets[bp];
				}
				else
				{
					return NULL;
				}
			}
		}

		return NULL;
	}

	inline MpAllocSpan* find_span(void* p)
	{
		MpAllocBucket* bk=find_bucket(p);
		return bk?bk->get():NULL;
	}
};

template<>
class RadixPageMapT<64>
{
public:
	static const uintptr_t bits0=64-MpAllocConfig::sp_bits;

	static const uintptr_t bits1=bits0/2;
	static const uintptr_t size1=1<<bits1;
	static const uintptr_t mask1=size1-1;

	static const uintptr_t bits2=bits0-bits1;
	static const uintptr_t size2=1<<bits2;
	static const uintptr_t mask2=size2-1;


	typedef MpAllocBucket BucketLevel2[size1];
	BucketLevel2* buckets2[size2];

	MpAllocConfig::spin_type bk_spin;;

	inline MpAllocBucket& bucket(MpAllocSpan* sp)
	{
		uintptr_t bp0=sp->sp_base>>MpAllocConfig::sp_bits;
		uintptr_t bp1=bp0>>bits1;
		uintptr_t bp2=bp0&mask1;
		if(buckets2[bp1]==NULL)
		{
			MpAllocConfig::lock_type lock1(bk_spin);
			if(buckets2[bp1]==NULL)
			{
				buckets2[bp1]=(BucketLevel2*)page_alloc(sizeof(BucketLevel2));
			}
		}
		return (*buckets2[bp1])[bp2];
	}

	void insert_span_nolock(MpAllocSpan* sp);
	void remove_span_nolock(MpAllocSpan* sp);

	MpAllocBucket* find_bucket(void* p);

	MpAllocSpan* find_span(void* p)
	{
		MpAllocBucket* bk=find_bucket(p);
		return bk?bk->get():NULL;
	}
};


class MpAllocGlobal
{
public:

	~MpAllocGlobal();

#ifdef EW_X86 
	static const size_t sz_bits1=2;
#else
	static const size_t sz_bits1=3;
#endif

	static const size_t sz_bits2=11;
	static const size_t sz_bits3=20;

	static const size_t sz_slot1=1<<sz_bits2;
	static const size_t sz_slot2=1<<sz_bits3;

	static const size_t sl_size=sizeof(FixAllocUnits)/sizeof(size_t);

	typedef RadixPageMapT<sizeof(void*)*8> pagemap_type;

	pagemap_type pgmap;
	MpFixedSizePool<sizeof(MpAllocSpan),true> spanalloc;

	MpAllocSlot aSlots[sl_size];
	MpAllocSlot* pSlot1[(sz_slot1>>sz_bits1)+1];
	MpAllocSlot* pSlot2[(sz_slot2>>sz_bits2)+1];

	void init();
	void cleanup();

	void gc(int level);

	EW_FORCEINLINE MpAllocSlot* get_slot(size_t n)
	{
		if(n<=sz_slot1)
		{
			size_t k=(n+((1<<sz_bits1)-1))>>sz_bits1;
			return pSlot1[k];
		}
		if(n<=sz_slot2)
		{
			size_t k=(n+((1<<sz_bits2)-1))>>sz_bits2;
			return pSlot2[k];
		}
		return NULL;
	}


	MpAllocSpan* create_span_nolock(MpAllocSlot& sl);
	MpAllocNode* dealloc_batch_nolock(MpAllocSlot& sl,MpAllocNode* fp,size_t sz);

	inline MpAllocNode* dealloc_batch(MpAllocSlot& sl,MpAllocNode* fp,size_t sz)
	{
		sl.sl_spin.lock();
		MpAllocNode* node=dealloc_batch_nolock(sl,fp,sz);
		sl.sl_spin.unlock();
		return node;
	}

	MpAllocNode* alloc_small_batch(MpAllocSlot& sl,size_t& sz);

	void* alloc_small(size_t n,MpAllocSlot& sl);
	void* alloc_small(size_t n,MpAllocSlot& sl,MpAllocInfo& i);
	void* alloc_large(size_t n);
	void* alloc_large(size_t n,MpAllocInfo& i);

	void dealloc_real(void* p,MpAllocBucket& bk);
	void* realloc_real(void* p,size_t n,MpAllocBucket& bk);

	EW_FORCEINLINE  void* alloc(size_t n)
	{
		MpAllocSlot* psl=get_slot(n);
		if(!psl)
		{
			return alloc_large(n);
		}
		else
		{
			return alloc_small(n,*psl);
		}
	}

	void* alloc(size_t n,MpAllocInfo& i);


	inline void* realloc(void* p,size_t n)
	{
		if(n==0)
		{
			dealloc(p);
			return NULL;
		}

		if(p==NULL)
		{
			return mp_alloc(n);
		}
		MpAllocBucket* pbk=pgmap.find_bucket(p);
		if(pbk)
		{
			return realloc_real(p,n,*pbk);
		}

		return ::realloc(p,n);
	}

	inline void dealloc(void* p)
	{
		MpAllocBucket* pbk=pgmap.find_bucket(p);
		if(pbk)
		{
			dealloc_real(p,*pbk);
			return;
		}

		System::LogTrace("mp_free(%p) failed, fallback to free",p);
		::free(p);
	}
};

class MpAllocNodeLink
{
public:
	MpAllocNodeLink(){nd_head=NULL;nd_nnum=0;}
	MpAllocNode* nd_head;
	size_t nd_nnum;
	size_t nd_nmax;
};


class MpAllocCachedNoLock
{
public:
	MpAllocNodeLink aLinks[MpAllocGlobal::sl_size+4];


	void init();

	void gc(int);

	void* alloc(size_t n);
	void* alloc(size_t n,MpAllocInfo& i);

	void dealloc(void* p);

};

void mp_init();
void tc_init();

extern EW_THREAD_TLS MpAllocCachedNoLock* tls_tc_data;
extern MpAllocGlobal *g_myalloc_impl;

inline MpAllocCachedNoLock* tc_get()
{
	if(!tls_tc_data)
	{
		if(!g_myalloc_impl)
		{
			mp_init();
			tc_init();
		}
	}
	return tls_tc_data;
}

inline void* mp_alloc_real(size_t n,const char* f,int l)
{
	if(MpAllocInfo::ALLOC_BREAK==MpAllocInfo::ALLOC_NNUM)
	{
		System::DebugBreak();
	}

	MpAllocInfo info(f,l);
	info.size=n;
	info.nnum=MpAllocInfo::num();
	info.flag=MpAllocInfo::ALLOC_FLAG;

	MpAllocCachedNoLock* tc_data=tc_get();
	if(tc_data)
	{
		return tc_data->alloc(n,info);
	}
	else
	{
		return g_myalloc_impl->alloc(n,info);
	}
}

inline void* mp_alloc_real(size_t n)
{

#ifdef EW_CHECK_HEAP
	return mp_alloc_real(n,"unknown file",-1);
#else

	MpAllocCachedNoLock* tc_data=tc_get();

	if(tc_data)
	{
		return tc_data->alloc(n);
	}
	else
	{
		return g_myalloc_impl->alloc(n);
	}
#endif
}

inline void mp_free_real(void* p)
{
	MpAllocCachedNoLock* tc_data=tls_tc_data;
	if(tc_data)
	{
		tc_data->dealloc(p);
	}
	else
	{
		g_myalloc_impl->dealloc(p);
	}
}


EW_LEAVE

