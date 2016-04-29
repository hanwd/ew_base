#include "ewa_base/memory/allocator.h"

EW_ENTER

void* AlignedAlloc(size_t size,size_t align,size_t shift)
{
	unsigned char* _pRaw=(unsigned char*)::malloc(size+align);
	if(_pRaw==NULL) return NULL;

	unsigned char _nDif=align-(((size_t)(_pRaw+shift))%align);
	unsigned char* pRet=_pRaw+_nDif;
	pRet[-1]=_nDif;
	return pRet;
}

void* AlignedAlloc(size_t size,size_t align)
{
	unsigned char* _pRaw=(unsigned char*)::malloc(size+align);
	if(_pRaw==NULL) return NULL;

	unsigned char _nDif=align-(((size_t)(_pRaw))%align);
	unsigned char* pRet=_pRaw+_nDif;
	pRet[-1]=_nDif;
	return pRet;
}

void AllignedFree(void* p)
{
	unsigned char* _pRet=(unsigned char*)p;
	if(!_pRet) return;
	::free(_pRet-_pRet[-1]);
}

EW_LEAVE
