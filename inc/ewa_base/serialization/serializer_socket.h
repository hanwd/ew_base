
#ifndef __H_EW_SERIALIZER_SOCKET__
#define __H_EW_SERIALIZER_SOCKET__

#include "ewa_base/serialization/serializer.h"
#include "ewa_base/net/ipaddress.h"
#include "ewa_base/net/socket.h"
#include "ewa_base/collection/linear_buffer.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE SerializerSocket : public SerializerDuplex
{
public:

	SerializerSocket(){}
	SerializerSocket(Socket& sock):socket(sock){}

	void close();

	Socket socket;


protected:

	int recv(char* data,int size_);
	int send(const char* data,int size_);

};


EW_LEAVE
#endif

