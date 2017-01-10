#include "ewa_base/collection/detail/indexer_base.h"

#include "ewa_base/collection.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/codecvt.h"
#include "ewa_base/basic/system.h"


EW_ENTER


void RingBufferBase::rewind()
{
	if(!pHeader)
	{
		System::LogTrace("try to rewind empty ringbuffer");
		return;
	}
	pHeader->wr_pos=pHeader->rd_pos=0;
}


bool RingBufferBase::packet()
{
	uint16_t size=0;
	if(peek(&size,2)!=2)
	{
		return false;
	}

	int rd=pHeader->rd_pos;
	int wr=pHeader->wr_pos;
	int kp=(wr-rd)&pHeader->rb_mask;

	return size<=kp;
}


int RingBufferBase::packet(void* p_,int n_)
{
	uint16_t size=0;
	if(peek(&size,2)!=2)
	{
		return 0;
	}
	int rd=pHeader->rd_pos;
	int wr=pHeader->wr_pos;
	int kp=(wr-rd)&pHeader->rb_mask;

	if(size>kp)
	{
		return 0;
	}

	if(size>n_)
	{
		return -size;
	}

	return recv(p_,size);
}


int RingBufferBase::peek(void* p_,int n)
{
	char* p=(char*)p_;
	int rd=pHeader->rd_pos;
	int wr=pHeader->wr_pos;
	int kp=(wr-rd)&pHeader->rb_mask;

	if(kp>n)
	{
		kp=n;
	}
	else if(kp==0)
	{
		return 0;
	}

	int kn=rd+kp-pHeader->rb_mask-1;
	if(kn>0)
	{
		memcpy(p,pBuffer+rd,kp-kn);
		memcpy(p+kp-kn,pBuffer,kn);
	}
	else
	{
		memcpy(p,pBuffer+rd,kp);
	}

	return kp;
}

int RingBufferBase::rd_free()
{
	int rd=pHeader->rd_pos;
	int wr=pHeader->wr_pos;
	int kp=(wr-rd)&pHeader->rb_mask;
	return kp;
}

int RingBufferBase::recv(void* p_,int n)
{
	char* p=(char*)p_;
	int rd=pHeader->rd_pos;
	int wr=pHeader->wr_pos;
	int kp=(wr-rd)&pHeader->rb_mask;

	if(kp==0)
	{
		return 0;
	}
	if(kp>n)
	{
		kp=n;
	}

	int kn=rd+kp-pHeader->rb_mask-1;
	if(kn>0)
	{
		memcpy(p,pBuffer+rd,kp-kn);
		memcpy(p+kp-kn,pBuffer,kn);
		pHeader->rd_pos=kn;
	}
	else
	{
		memcpy(p,pBuffer+rd,kp);
		pHeader->rd_pos=kn&pHeader->rb_mask;
	}

	return kp;
}

int RingBufferBase::wr_free()
{
	int rd=pHeader->rd_pos;
	int wr=pHeader->wr_pos;
	int kp=(rd-wr-1)&pHeader->rb_mask;
	return kp;
}

int RingBufferBase::send(const void* p_,int n)
{
	const char* p=(const char*)p_;

	int rd=pHeader->rd_pos;
	int wr=pHeader->wr_pos;
	int kp=(rd-wr-1)&pHeader->rb_mask;

	if(kp>=n)
	{
		kp=n;
	}
	else
	{
		return 0;
	}

	int kn=wr+kp-pHeader->rb_mask-1;
	if(kn>0)
	{
		memcpy(pBuffer+wr,p,kp-kn);
		memcpy(pBuffer,p+kp-kn,kn);
		pHeader->wr_pos=kn;
	}
	else
	{
		memcpy(pBuffer+wr,p,kp);
		pHeader->wr_pos=kn&pHeader->rb_mask;
	}

	return kp;
}

RingBuffer::RingBuffer()
{
	pHeader=&_header;
	pBuffer=NULL;
}

RingBuffer::~RingBuffer()
{
	clear();
}

void RingBuffer::reset(int s)
{
	int k=sz_helper::n2p(s);
	char* p=(char*)mp_alloc(k);
	if(!p)
	{
		Exception::XBadAlloc();
	}

	clear();

	pBuffer=p;
	_header.rb_mask=k-1;
	_header.rd_pos=0;
	_header.wr_pos=0;

}

void RingBuffer::clear()
{
	if(pBuffer)
	{
		mp_free(pBuffer);
		pBuffer=NULL;
	}
}


template<typename T>
bool LinearBufferEx<T>::_grow(size_type _newsize)
{
	if(pBuffer!=NULL && pBuffer!=aBuff.data())
	{
		return false;
	}

	size_type _size=sz_helper::n2p(_newsize+(_newsize>>2));
	if(_size<=_newsize) _size=_newsize<<1;

	aBuff.resize(_size);
	pBuffer=aBuff.data();
	sz_buf=aBuff.size();
	return true;	
}

template class LinearBuffer<char>;
template class LinearBuffer<unsigned char>;
template class LinearBuffer<wchar_t>;
template class LinearBufferEx<char>;
template class LinearBufferEx<unsigned char>;
template class LinearBufferEx<wchar_t>;


EW_LEAVE
