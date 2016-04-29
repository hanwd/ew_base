
#ifndef __H_EW_SERIALIZER_SOCKET__
#define __H_EW_SERIALIZER_SOCKET__

#include "ewa_base/serialization/serializer.h"
#include "ewa_base/net/ipaddress.h"
#include "ewa_base/net/socket.h"
#include "ewa_base/collection/linear_buffer.h"

EW_ENTER




// Usage:
// SerializerSocket ar;
//
// //reader
// if(ar.rd_sync()) // recv data from socket and store to recv buffer.
// {
//	 ar.reader() & obj & ...; // read data from recv buffer.
//	 // call ar.rd_test() to test read position reach the end of recv buffer.
// }
//
// //writer
// ar.writer() & obj & ...; write data to send buffer;
// if(ar.wr_sync()) // send buffer to socket
// {
//
// }
// //call ar.wr_skip() to clear send buffer;
//
class DLLIMPEXP_EWA_BASE SerializerSocket : public SerializerDuplex
{
public:

	SerializerSocket();

	void close();

	// read data from socket and store to buffer.
	bool rd_sync();

	// test if read position reach the end of the buffer
	bool rd_test();


	// send buffer to socket and clear buffer.
	bool wr_sync();

	// clear buffer
	void wr_skip();

	Socket sock;

protected:
	LinearBuffer<char> lbuf_rd;
	LinearBuffer<char> lbuf_wr;

	size_t recv(char* data,size_t size_);
	size_t send(const char* data,size_t size_);

};


EW_LEAVE
#endif

