#include "mempool_impl.h"


EW_ENTER


void MpAllocCachedNoLock::init()
{
	
	for(size_t i=0;i<MpAllocGlobal::sl_size;i++)
	{
		aLinks[i].nd_head=NULL;
		aLinks[i].nd_nnum=0;
		aLinks[i].nd_nmax=g_myalloc_impl->aSlots[i].nd_nall*2;

		if(aLinks[i].nd_nmax<4)
		{
			aLinks[i].nd_nmax=4;
		}
	}
}

void MpAllocCachedNoLock::gc(int level)
{
	if(level==0)
	{
		for(size_t i=0;i<MpAllocGlobal::sl_size;i++)
		{
			MpAllocSlot& sl(g_myalloc_impl->aSlots[i]);
			if(!sl.sl_spin.try_lock()) continue;
			aLinks[i].nd_head=g_myalloc_impl->dealloc_batch_nolock(sl,aLinks[i].nd_head,aLinks[i].nd_nnum);
			EW_ASSERT(!aLinks[i].nd_head);
			aLinks[i].nd_nnum=0;
			sl.sl_spin.unlock();
		}	
	}
	else
	{
		for(size_t i=0;i<MpAllocGlobal::sl_size;i++)
		{
			MpAllocSlot& sl(g_myalloc_impl->aSlots[i]);
			aLinks[i].nd_head=g_myalloc_impl->dealloc_batch(sl,aLinks[i].nd_head,aLinks[i].nd_nnum);
			EW_ASSERT(!aLinks[i].nd_head);
			aLinks[i].nd_nnum=0;
		}
	}

}

void* MpAllocCachedNoLock::alloc(size_t n,MpAllocInfo& i)
{

	MpAllocSlot* psl=g_myalloc_impl->get_slot(n+sizeof(MpAllocInfo));
	if(!psl)
	{
		return g_myalloc_impl->alloc_large(n,i);
	}

	size_t k=psl-g_myalloc_impl->aSlots;
	EW_ASSERT(k<MpAllocGlobal::sl_size);

	MpAllocNode* p=aLinks[k].nd_head;
	if(!p)
	{
		EW_ASSERT(aLinks[k].nd_nnum==0);
		p=g_myalloc_impl->alloc_small_batch(g_myalloc_impl->aSlots[k],aLinks[k].nd_nnum);
		if(!p) return NULL;
	}
	aLinks[k].nd_nnum--;
	aLinks[k].nd_head=p->nd_next;

	i.set(p,psl->nd_size);

	return p;

}

void* MpAllocCachedNoLock::alloc(size_t n)
{
	MpAllocSlot* psl=g_myalloc_impl->get_slot(n);
	if(!psl)
	{
		return g_myalloc_impl->alloc_large(n);
	}

	size_t k=psl-g_myalloc_impl->aSlots;
	EW_ASSERT(k<MpAllocGlobal::sl_size);

	MpAllocNode* p=aLinks[k].nd_head;
	if(!p)
	{
		EW_ASSERT(aLinks[k].nd_nnum==0);
		p=g_myalloc_impl->alloc_small_batch(g_myalloc_impl->aSlots[k],aLinks[k].nd_nnum);
		if(!p) return NULL;
	}
	aLinks[k].nd_nnum--;
	aLinks[k].nd_head=p->nd_next;
	return p;
}

void MpAllocCachedNoLock::dealloc(void* p)
{
	MpAllocBucket* pbk=g_myalloc_impl->pgmap.find_bucket(p);
	if(!pbk)
	{
		System::LogTrace("mp_dealloc(%p) failed",p);
		return;
	}

	MpAllocSlot* psl=pbk->get()->sl_slot;
	if(!psl)
	{
		g_myalloc_impl->dealloc_real(p,*pbk);
		return;
	}

#ifdef EW_CHECK_HEAP
	MpAllocInfo::check(p,psl->nd_size);
#endif

	MpAllocNode* n=(MpAllocNode*)p;
	uintptr_t k=psl-g_myalloc_impl->aSlots;
	EW_ASSERT(k<MpAllocGlobal::sl_size);

	n->nd_next=aLinks[k].nd_head;
	aLinks[k].nd_head=n;
	if(aLinks[k].nd_nnum++>=aLinks[k].nd_nmax)
	{
		size_t sz=aLinks[k].nd_nnum/2;
		aLinks[k].nd_head=g_myalloc_impl->dealloc_batch(*psl,aLinks[k].nd_head,sz);
		EW_ASSERT(aLinks[k].nd_head!=NULL);
		aLinks[k].nd_nnum-=sz;
	}
}


void tc_init()
{
	MpAllocCachedNoLock* tc_data=tls_tc_data;
	if(tls_tc_data)
	{
		System::LogTrace("ThreadCachedData already inited");
		return;
	}

	tls_tc_data=(MpAllocCachedNoLock*)mp_alloc_real(sizeof(MpAllocCachedNoLock));
	if(tls_tc_data)
	{
		tls_tc_data->init();
	}
}

void tc_cleanup()
{
	if(tls_tc_data)
	{
		tls_tc_data->gc(-1);
		g_myalloc_impl->dealloc(tls_tc_data);
		tls_tc_data=NULL;
	}
}

void tc_gc(int level)
{
	if(tls_tc_data)
	{
		tls_tc_data->gc(level);
	}
}





EW_LEAVE
