
#include "ewa_base/serialization/serializer_socket.h"

EW_ENTER


void SerializerSocket::close()
{
	socket.close();
}


int SerializerSocket::recv(char* data,int size_)
{
	return socket.recv(data,size_);
}

int SerializerSocket::send(const char* data,int size_)
{
	return socket.send(data,size_);
}

EW_LEAVE
