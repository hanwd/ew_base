
#ifndef __H_EW_SERIALIZER_BUFFER__
#define __H_EW_SERIALIZER_BUFFER__

#include "ewa_base/serialization/serializer.h"
#include "ewa_base/collection/linear_buffer.h"


EW_ENTER


class DLLIMPEXP_EWA_BASE IStreamBuffer : public IStreamData
{
public:

	IStreamBuffer();

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

	int send(const char* data,size_t size);
	int recv(char* data,size_t size);

protected:
	
	LinearBufferEx<char> lbuf;

};


EW_LEAVE
#endif

