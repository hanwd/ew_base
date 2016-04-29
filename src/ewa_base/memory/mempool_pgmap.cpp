#include "mempool_impl.h"
#include "ewa_base/basic/system.h"

EW_ENTER

void RadixPageMapT<64>::insert_span_nolock(MpAllocSpan* sp)
{
	MpAllocBucket& bk(bucket(sp));
	EW_ASSERT(bk.get()==NULL);
	bk.set(sp);
}

void RadixPageMapT<64>::remove_span_nolock(MpAllocSpan* sp)
{
	MpAllocBucket& bk(bucket(sp));
	EW_ASSERT(bk.get()==sp);
	bk.set(NULL);
}

MpAllocBucket* RadixPageMapT<64>::find_bucket(void* p)
{
	uintptr_t kp=reinterpret_cast<uintptr_t>(p);
	uintptr_t bp0=kp>>MpAllocConfig::sp_bits;
	uintptr_t bp1=bp0>>bits1;
	uintptr_t bp2=bp0&mask1;

	if(!buckets2[bp1])
	{
		return NULL;
	}

	BucketLevel2* pbucket(buckets2[bp1]);

	MpAllocSpan* sp=(*pbucket)[bp2].get();
	if(sp)
	{
		if(kp>=sp->sp_base)
		{
			EW_ASSERT(kp-sp->sp_base<sp->sp_size);
			return &(*pbucket)[bp2];
		}
	}
		
	// fall back 1 bucket
	if(bp2==0)
	{
		if(bp1==0)
		{
			return NULL;
		}

		pbucket=buckets2[--bp1];
		if(!pbucket)
		{
			return NULL;
		}
		bp2=size1;
	}

	sp=(*pbucket)[--bp2].get();
	if(sp)
	{
		if(uintptr_t(kp-sp->sp_base)<sp->sp_size)
		{
			return &(*pbucket)[bp2];
		}
		else
		{
			return NULL;
		}
	}

	for(size_t i=0;i<MpAllocConfig::bk_more;i++)
	{		
		if(bp2==0)
		{
			if(bp1==0)
			{
				return NULL;
			}

			pbucket=buckets2[--bp1];
			if(!pbucket)
			{
				return NULL;
			}
			bp2=size1;
		}

		sp=(*pbucket)[--bp2].get();
		if(sp)
		{
			if(uintptr_t(kp-sp->sp_base)<sp->sp_size)
			{
				return &(*pbucket)[bp2];
			}
			else
			{
				return NULL;
			}
		}
	}

		
	return NULL;
}


EW_LEAVE
