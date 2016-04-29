
#include "ewa_base/net/ipaddress.h"
#include "ewa_base/net/socket.h"
#include "ewa_base/memory/mempool.h"
#include "ewa_base/basic/system.h"

#include "socket_impl_detail.h"


EW_ENTER

bool InitNetworkObjects();

void KO_Policy_socket::destroy(type& o)
{
	socket_detail::destroy(o);
}

Socket::Socket()
{
	static bool bNetInitedOk=InitNetworkObjects();
	(void)&bNetInitedOk;
}

Socket::~Socket()
{

}

bool Socket::IsConnected()
{
	return socket_detail::is_connected(impl);
}

int Socket::Send(const char* buf,int len)
{
	return socket_detail::send(impl,buf,len);
}

int Socket::Recv(char* buf,int len)
{
	return socket_detail::recv(impl,buf,len);
}

int Socket::Send(const char* buf,int len,const IPAddress& addr)
{
	return socket_detail::send(impl,buf,len,addr);
}

int Socket::Recv(char* buf,int len,IPAddress& addr)
{
	return socket_detail::recv(impl,buf,len,addr);
}

int Socket::SendMsg(const char* buf,int len)
{
	return socket_detail::send_msg(impl,buf,len);
}

int Socket::RecvMsg(char* buf,int len)
{
	return socket_detail::recv_msg(impl,buf,len);
}

int Socket::Peek(char* buf,int len)
{
	return socket_detail::peek(impl,buf,len);
}

void Socket::Block(bool f)
{
	socket_detail::block(impl,f);
}

void Socket::ReuseAddress(bool f)
{
	socket_detail::reuseaddr(impl,f);
}

void Socket::Shutdown(int t)
{
	socket_detail::shutdown(impl,t);
}

void Socket::Ensure(int t)
{
	socket_detail::ensure(impl,t);
}

bool Socket::Connect(const String& ip,int port)
{
	IPAddress addr;
	addr.service(ip,port);
	return Connect(addr);
}

bool Socket::Connect(const IPAddress& addr)
{
	return socket_detail::connect(impl,addr);
}

bool Socket::Accept(Socket& sock)
{
	IPAddress addr;
	return socket_detail::accept(impl,sock.impl,addr);
}

bool Socket::Accept(Socket& sock,IPAddress& addr)
{
	return socket_detail::accept(impl,sock.impl,addr);
}

bool Socket::Bind(const String& ip,int port)
{
	IPAddress addr(ip,port);
	return Bind(addr);
}

bool Socket::Bind(const IPAddress& addr)
{
	return socket_detail::bind(impl,addr);
}

bool Socket::GetPeerAddr(IPAddress& addr)
{
	return socket_detail::getpeername(impl,addr);
}

bool Socket::GetSockAddr(IPAddress& addr)
{
	return socket_detail::getsockname(impl,addr);
}

bool Socket::Listen()
{
	return socket_detail::listen(impl);
}

bool Socket::Listen(const IPAddress& addr)
{
	if(!Bind(addr))
	{
		return false;
	}

	if(!Listen())
	{
		return false;
	}

	return true;
}

bool Socket::Listen(const String& ip,int port)
{
	IPAddress addr(ip,port);
	return Listen(addr);
}

void Socket::Close()
{
	impl.close();
}

void Socket::Reset()
{
	socket_detail::create(impl,impl.protocol);
}

#ifdef EW_WINDOWS
bool InitNetworkObjects()
{
	static bool status=false;
	static bool first=true;
	if(first)
	{
		first=false;

		WORD wVersionRequested;
		WSADATA wsaData;
		wVersionRequested = MAKEWORD( 2, 2 );
		int err = WSAStartup( wVersionRequested, &wsaData );
		if ( err != 0 )
		{
			System::LogTrace("WSAStartup failed");
			status= false;
		}
		else
		{
			status=true;
		}

	}

	return status;
}
#else

bool InitNetworkObjects()
{
	return true;
}
#endif


EW_LEAVE
