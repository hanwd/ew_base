#include "mempool_impl.h"

EW_ENTER


void MpAllocNode::link(void* p1,size_t sz,size_t tp,void* p3)
{
	EW_ASSERT(tp>=sizeof(MpAllocNode));

	MpAllocNode* n1=(MpAllocNode*)p1;		
	MpAllocNode* n2=(MpAllocNode*)(((char*)p1)+(sz/tp)*tp);
	size_t kp=tp/sizeof(MpAllocNode);

	for(;;)
	{
		MpAllocNode* fn=n1+kp;
		if(fn>=n2)
		{
			EW_ASSERT(fn==n2);
			n1->nd_next=(MpAllocNode*)p3;
			return;
		}

		n1->nd_next=fn;
		n1=fn;
	}
}


void MpAllocSlot::init(size_t nd)
{
	nd_size=nd;
	sp_head=NULL;

	nd_nall=(MpAllocConfig::sp_size)/nd_size;

	if(nd_size>64 && MpAllocConfig::sp_size-nd_nall*nd_size<sizeof(MpAllocSpan))
	{
		sp_flag.set_fly(true);
	}
	else
	{
		nd_nall=(MpAllocConfig::sp_size-sizeof(MpAllocSpan))/nd_size;
		sp_flag.set_fly(false);
	}
}

EW_LEAVE


