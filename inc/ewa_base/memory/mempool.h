
#ifndef __H_EW_MEMORY_MEMPOOL__
#define __H_EW_MEMORY_MEMPOOL__

#include "ewa_base/basic/exception.h"
//#include "ewa_base/basic/system.h"
#include "ewa_base/basic/atomic.h"

EW_ENTER
	
DLLIMPEXP_EWA_BASE void* mp_alloc(size_t n);
DLLIMPEXP_EWA_BASE void* mp_alloc(size_t n,const char* f,int line);
DLLIMPEXP_EWA_BASE void* mp_realloc(void* p,size_t n);
DLLIMPEXP_EWA_BASE void mp_free(void* p);

DLLIMPEXP_EWA_BASE void mp_force_gc(int level);

DLLIMPEXP_EWA_BASE void mp_break_alloc(int n);
DLLIMPEXP_EWA_BASE void mp_check_leak(int t);

DLLIMPEXP_EWA_BASE void* mp_alloc_with_page_protect(size_t n);
DLLIMPEXP_EWA_BASE void mp_free_with_page_protect(void* p);

DLLIMPEXP_EWA_BASE void* page_alloc(size_t n);
DLLIMPEXP_EWA_BASE void page_free(void* p,size_t n);
DLLIMPEXP_EWA_BASE int page_access(void* p,size_t n,int f);


class DLLIMPEXP_EWA_BASE MemPoolPaging
{
public:

	// Allocate memory with given size, if the given size exceed the max fixed-size, mempoll will use malloc to allocate.
	static void* allocate(size_t size);

	// Deallocate memory. if p is NOT allocated by this mempool, mempool will use free to deallocate.
	static void deallocate(void* p);

	// Same as above, drop the extra parameters.
	static void* allocate(size_t size,const char*,int);

	static void deallocate(void* p,size_t)
	{
		deallocate(p);
	}

	static MemPoolPaging& current()
	{
		static MemPoolPaging pool;
		return pool;
	}

};


// MemPoolMalloc use malloc/free to allocate/deallocate memory.
class DLLIMPEXP_EWA_BASE MemPoolMalloc
{
public:

	static void* allocate(size_t size);

	static void* allocate(size_t size,const char*,int)
	{
		return allocate(size);
	}

	static void deallocate(void* p);

	static void deallocate(void* p,size_t)
	{
		deallocate(p);
	}

	static MemPoolMalloc& current()
	{
		static MemPoolMalloc pool;
		return pool;
	}
};


typedef MemPoolPaging MemPool;


class DLLIMPEXP_EWA_BASE MpAllocNode
{
public:
	MpAllocNode* nd_next;
	static void link(void* p1,size_t sz,size_t tp,void* p3=NULL);
};


// TS = ThreadSafe
// DG = DynamicGrow
template<size_t N,bool TS=false,bool DG=true>
class MpFixedSizePool : public ThreadSafe<TS>
{
public:
	static const size_t nd_size=(N+sizeof(void*)-1)&~(sizeof(void*)-1);
	static const size_t sp_size=1024*1024;

	typedef ThreadSafe<TS> basetype;

	MpAllocNode* nd_free;
	MpAllocNode* pg_list;

	void swap(MpFixedSizePool& p)
	{
		lock_mutex2(*this,p);
		std::swap(nd_free,p.nd_free);
		std::swap(pg_list,p.pg_list);
		unlock_mutex2(*this,p);
	}

	MpFixedSizePool():nd_free(NULL),pg_list(NULL){}

	MpFixedSizePool(const MpFixedSizePool&):nd_free(NULL),pg_list(NULL){}
	MpFixedSizePool& operator=(const MpFixedSizePool&){return *this;}

	~MpFixedSizePool()
	{
		while(pg_list)
		{
			MpAllocNode* pg=pg_list;
			pg_list=pg_list->nd_next;
			page_free(pg,sp_size);
		}
	}

	void* alloc()
	{
		basetype::lock();

		MpAllocNode* p;
		if(nd_free|| (DG && alloc_batch_nolock()) )
		{
			p=nd_free;
			nd_free=nd_free->nd_next;
		}
		else
		{
			p=NULL;
		}

		basetype::unlock();
		return p;
	}

	void dealloc(void* p)
	{
		basetype::lock();

		MpAllocNode* fn=(MpAllocNode*)p;
		fn->nd_next=nd_free;
		nd_free=fn;

		basetype::unlock();
	}

	bool alloc_batch()
	{
		basetype::lock();
		bool f=alloc_batch_nolock();
		basetype::unlock();
		return f;
	}

protected:

	bool alloc_batch_nolock()
	{
		MpAllocNode* p1=(MpAllocNode*)page_alloc(sp_size);
		if(!p1)
		{			
			System::LogError("page_alloc failed in MpFixedSizePool::alloc_batch");
			return false;
		}

		MpAllocNode::link(p1,sp_size,nd_size,nd_free);
		nd_free=p1->nd_next;
		p1->nd_next=pg_list;
		pg_list=p1;

		return true;
	}


};



EW_LEAVE

#endif
