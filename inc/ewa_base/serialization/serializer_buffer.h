
#ifndef __H_EW_SERIALIZER_BUFFER__
#define __H_EW_SERIALIZER_BUFFER__

#include "ewa_base/serialization/serializer.h"
#include "ewa_base/collection/linear_buffer.h"


EW_ENTER


class DLLIMPEXP_EWA_BASE SerializerBuffer : public SerializerDuplex
{
public:

	SerializerBuffer();

	// skip data, return true if gptr()==gend();
	bool skip();

	// assign external buffer
	void assign(char* pbuf,size_t size);

	// allocate buffer
	void alloc(size_t bufsize);


	void setbuf(char* pbuf,size_t size)
	{
		lbuf.assign(pbuf,size);
		lbuf.wr_flip(size);
	}


	char* gbeg()
	{
		return lbuf.gbeg();   // buffer begin
	}

	char* gptr()
	{
		return lbuf.gptr();   // get position begin
	}

	char* gend()
	{
		return lbuf.gend();   // get position end or put position begin
	}

	char* last()
	{
		return lbuf.last();
	}

	size_t send(const char* data,size_t size);
	size_t recv(char* data,size_t size);

protected:
	
	LinearBuffer<char> lbuf;

};


EW_LEAVE
#endif

