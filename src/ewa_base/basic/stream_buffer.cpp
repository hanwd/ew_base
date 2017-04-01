
#include "ewa_base/basic/stream_buffer.h"

EW_ENTER

IStreamBuffer::IStreamBuffer()
{

}

bool IStreamBuffer::skip()
{
	return lbuf.skip();
}

// assign m_pBuffer
void IStreamBuffer::assign(char* pbuf,size_t size)
{
	lbuf.assign(pbuf,size);
}

// allocate buffer
void IStreamBuffer::alloc(size_t bufsize)
{
	lbuf.alloc(bufsize);
}


int IStreamBuffer::send(const char* data,size_t size)
{
	return lbuf.send(data,size);
}

int IStreamBuffer::recv(char* data,size_t size)
{
	return lbuf.recv(data,size);
}

EW_LEAVE
