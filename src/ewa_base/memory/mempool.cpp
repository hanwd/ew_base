#include "mempool_impl.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/basic/platform.h"

#ifdef EW_WINDOWS
#include <windows.h>
#else
#include <sys/mman.h>
#include <cstring>
#endif

#ifdef new
#undef new
#endif


EW_ENTER

EW_THREAD_TLS MpAllocCachedNoLock* tls_tc_data;

#ifdef EW_WINDOWS

void* page_alloc(size_t n)
{
	void* p = ::VirtualAlloc(NULL, n, MEM_COMMIT, PAGE_READWRITE );
	if(p)
	{
		return p;
	}

	if(g_myalloc_impl)
	{
		System::LogTrace("page_alloc failed, call gc() and retry");
		mp_force_gc(0);

		p = ::VirtualAlloc(NULL, n, MEM_COMMIT, PAGE_READWRITE );
		if(p)
		{
			return p;
		}
	}

	System::CheckError("VirtualAlloc failed");
	return NULL;
}

void page_free(void* p,size_t)
{
	if(::VirtualFree(p, 0, MEM_RELEASE)==0)
	{
		System::LogTrace("VirtualFree failed: ptr=%p",p);
	}
}

int page_access(void* p,size_t n,int f)
{
	DWORD flag_old;
	DWORD flag_new=0;
	if(f&FLAG_FILE_WR)
	{
		flag_new=PAGE_READWRITE;
	}
	else if(f&FLAG_FILE_RD)
	{
		flag_new=PAGE_READONLY;
	}
	else
	{
		flag_new=PAGE_NOACCESS;
	}

	if(f&FLAG_FILE_EXEC)
	{
		flag_new=flag_new<<4;
	}

	VirtualProtect(p,n,flag_new,&flag_old);
	f=0;
	if(flag_old&0xF0)
	{
		f|=FLAG_FILE_EXEC;
		flag_old>>=4;
	}
	if(flag_old==PAGE_READWRITE)
	{
		f|=FLAG_FILE_RW;
	}
	else if(flag_old==PAGE_READONLY)
	{
		f|=FLAG_FILE_RD;
	}

	return f;

}

#else

void* page_alloc(size_t nSize)
{

	void* p=mmap(NULL,nSize,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);

	if(p!=MAP_FAILED)
	{
		return p;
	}

	if(g_myalloc_impl)
	{
		System::LogTrace("page_alloc failed, call gc() and retry");
		mp_force_gc(0);

		p=mmap(NULL,nSize,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
		if(p!=MAP_FAILED)
		{
			return p;
		}
	}
	
	System::CheckError("mmap failed");
	return NULL;
}

DLLIMPEXP_EWA_BASE int page_access(void* p,size_t n,int f)
{
	int flag_old;
	int flag_new=0;

	if(f&FLAG_FILE_WR)
	{
		flag_new|=PROT_WRITE;
	}
	if(f&FLAG_FILE_RD)
	{
		flag_new|=PROT_READ;
	}
	if(f&FLAG_FILE_EXEC)
	{
		flag_new|=PROT_EXEC;
	}

	flag_old=mprotect (p, n, flag_new);

	f=0;

	if(flag_old&PROT_WRITE)
	{
		f|=FLAG_FILE_RW;
	}
	if(flag_old&PROT_READ)
	{
		f|=FLAG_FILE_RD;
	}
	if(flag_old&PROT_EXEC)
	{
		f|=FLAG_FILE_EXEC;
	}
	return f;


}

void page_free(void* p,size_t s)
{
	munmap(p,s);
}
#endif


void* mp_obj::operator new(size_t nSize)
{
	return MemPoolPaging::allocate(nSize);
}

void* mp_obj::operator new[](size_t nSize)
{
	return MemPoolPaging::allocate(nSize);
}

void* mp_obj::operator new(size_t nSize,const char* sFile,int nLine)
{
	return MemPoolPaging::allocate(nSize,sFile,nLine);
}

void* mp_obj::operator new[](size_t nSize,const char* sFile,int nLine)
{
	return MemPoolPaging::allocate(nSize,sFile,nLine);
}

void mp_obj::operator delete(void* p,const char*,int)
{
	MemPoolPaging::deallocate(p);
}

void mp_obj::operator delete[](void* p,const char*,int)
{
	MemPoolPaging::deallocate(p);
}

void mp_obj::operator delete(void* p)
{
	MemPoolPaging::deallocate(p);
}

void mp_obj::operator delete[](void* p)
{
	MemPoolPaging::deallocate(p);
}


EW_LEAVE

#if defined(EW_MEMDEBUG) || defined(EW_MEMUSEPOOL)

void* operator new(size_t nSize)
{
	return ew::MemPool::allocate(nSize);
}

void operator delete(void* p)
{
	ew::MemPool::deallocate(p);
}

void* operator new(size_t nSize,const char* sFile,int nLine)
{
	return ew::MemPool::allocate(nSize,sFile,nLine);
}

void operator delete(void* p,const char*,int)
{
	ew::MemPool::deallocate(p);
}

void* operator new(size_t nSize,int,const char* sFile,int nLine)
{
	return ew::MemPool::allocate(nSize,sFile,nLine);
}

void operator delete(void* p,int,const char*,int)
{
	ew::MemPool::deallocate(p);
}

void* operator new[](size_t nSize)
{
	return ew::MemPool::allocate(nSize);
}

void operator delete[](void* p)
{
	ew::MemPool::deallocate(p);
}

void* operator new[](size_t nSize,const char* sFile,int nLine)
{
	return ew::MemPool::allocate(nSize,sFile,nLine);
}

void operator delete[](void* p,const char*,int)
{
	ew::MemPool::deallocate(p);
}

void* operator new[](size_t nSize,int,const char* sFile,int nLine)
{
	return ew::MemPool::allocate(nSize,sFile,nLine);
}

void operator delete[](void* p,int,const char*,int)
{
	ew::MemPool::deallocate(p);
}

#endif
