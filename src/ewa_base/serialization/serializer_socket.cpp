
#include "ewa_base/serialization/serializer_socket.h"

EW_ENTER

SerializerSocket::SerializerSocket()
{

}

void SerializerSocket::close()
{
	sock.Close();
}

bool SerializerSocket::rd_sync()
{

	int32_t sz(0);
	if(sock.Recv((char*)&sz,4)!=4)
	{
		return false;
	}

	if(sz<0)
	{
		return false;
	}

	lbuf_rd.rewind(sz);
	if(sock.RecvMsg(lbuf_rd.gbeg(),sz)!=sz)
	{
		return false;
	}

	lbuf_rd.wr_flip(sz);
	return true;
}

bool SerializerSocket::rd_test()
{
	int32_t s;
	return sock.Peek((char*)&s,sizeof(s))>0;
}

bool SerializerSocket::wr_sync()
{
	int32_t sz=lbuf_wr.rd_free();

	if(sock.Send((char*)&sz,4)!=4)
	{
		return false;
	}

	if(sock.SendMsg((char*)lbuf_wr.gbeg(),sz)!=sz)
	{
		return false;
	}

	wr_skip();
	return true;
}

void SerializerSocket::wr_skip()
{
	lbuf_wr.rewind();
}

size_t SerializerSocket::recv(char* data,size_t size_)
{
	return lbuf_rd.recv(data,size_);
}

size_t SerializerSocket::send(const char* data,size_t size_)
{
	return lbuf_wr.send(data,size_);
}

EW_LEAVE
