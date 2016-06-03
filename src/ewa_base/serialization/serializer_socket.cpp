
#include "ewa_base/serialization/serializer_socket.h"

EW_ENTER


void SerializerSocket::close()
{
	socket.Close();
}


int SerializerSocket::recv(char* data,int size_)
{
	return socket.Recv(data,size_);
}

int SerializerSocket::send(const char* data,int size_)
{
	return socket.Send(data,size_);
}

EW_LEAVE
