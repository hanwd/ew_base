
#include "ewa_base/serialization/serializer_buffer.h"

EW_ENTER

SerializerBuffer::SerializerBuffer()
{

}

bool SerializerBuffer::skip()
{
	return lbuf.skip();
}

// assign m_pBuffer
void SerializerBuffer::assign(char* pbuf,size_t size)
{
	lbuf.assign(pbuf,size);
}

// allocate buffer
void SerializerBuffer::alloc(size_t bufsize)
{
	lbuf.alloc(bufsize);
}


size_t SerializerBuffer::send(const char* data,size_t size)
{
	return lbuf.send(data,size);
}

size_t SerializerBuffer::recv(char* data,size_t size)
{
	return lbuf.recv(data,size);
}

EW_LEAVE
