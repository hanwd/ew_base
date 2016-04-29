#ifndef __H_EW_COLLECTION_RING_BUFFER__
#define __H_EW_COLLECTION_RING_BUFFER__

#include "ewa_base/collection/detail/collection_base.h"
#include "ewa_base/basic/atomic.h"
#include "ewa_base/basic/system.h"
#include <cstring>

EW_ENTER


class DLLIMPEXP_EWA_BASE ringbuffer_header
{
public:
	uint32_t rd_pos;
	uint32_t wr_pos;
	uint32_t rb_mask;
	uint32_t padding;
};

// RingBufferBase one thread read/one thread write
class DLLIMPEXP_EWA_BASE RingBufferBase
{
public:

	RingBufferBase()
	{
		pBuffer=NULL;
		pHeader=NULL;
	}

	void rewind();

	void wr_flip(int n)
	{
		pHeader->wr_pos=(pHeader->wr_pos+n)&pHeader->rb_mask;
	}
	void rd_flip(int n)
	{
		pHeader->rd_pos=(pHeader->rd_pos+n)&pHeader->rb_mask;
	}

	bool packet();
	int packet(void* p_,int n_);

	int peek(void* p_,int n_);
	int recv(void* p_,int n_);
	int send(const void* p_,int n_);

	int rd_free();
	int wr_free();


	char* pBuffer;
	ringbuffer_header* pHeader;
};


class DLLIMPEXP_EWA_BASE RingBuffer : public RingBufferBase, private NonCopyable
{
public:

	RingBuffer();
	~RingBuffer();

	void reset(int s);
	void clear();

private:
	ringbuffer_header _header;
};

EW_LEAVE

#endif
