#include "ewa_base/memory/mempool.h"
#include "ewa_base/basic/system.h"

EW_ENTER

void* MemPoolMalloc::allocate(size_t size)
{
	void* pMem=::malloc(size);
	if(pMem==NULL)
	{
		System::LogTrace("malloc failed: size=%d",(int)size);
		Exception::XBadAlloc();
	}
	return pMem;
}

void MemPoolMalloc::deallocate(void* p)
{
	::free(p);
}

EW_LEAVE
