
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

bool Socket::is_connected()
{
	return socket_detail::is_connected(impl);
}

int Socket::send(const char* buf,int len)
{
	return socket_detail::send(impl,buf,len);
}

int Socket::recv(char* buf,int len)
{
	return socket_detail::recv(impl,buf,len);
}

int Socket::send(const char* buf,int len,const IPAddress& addr)
{
	return socket_detail::send(impl,buf,len,addr);
}

int Socket::recv(char* buf,int len,IPAddress& addr)
{
	return socket_detail::recv(impl,buf,len,addr);
}

int Socket::send_all(const char* buf,int len)
{
	return socket_detail::send_msg(impl,buf,len);
}

int Socket::recv_all(char* buf,int len)
{
	return socket_detail::recv_msg(impl,buf,len);
}

int Socket::peek(char* buf,int len)
{
	return socket_detail::peek(impl,buf,len);
}

void Socket::block(bool f)
{
	socket_detail::block(impl,f);
}

void Socket::reuse_address(bool f)
{
	socket_detail::reuseaddr(impl,f);
}

void Socket::shutdown(int t)
{
	socket_detail::shutdown(impl,t);
}

void Socket::ensure(int t)
{
	socket_detail::ensure(impl,t);
}

bool Socket::connect(const String& ip,int port)
{
	IPAddress addr;
	addr.service(ip,port);
	return connect(addr);
}

bool Socket::connect(const IPAddress& addr)
{
	return socket_detail::connect(impl,addr);
}

bool Socket::accept(Socket& sock)
{
	IPAddress addr;
	return socket_detail::accept(impl,sock.impl,addr);
}

bool Socket::accept(Socket& sock,IPAddress& addr)
{
	return socket_detail::accept(impl,sock.impl,addr);
}

bool Socket::bind(const String& ip,int port)
{
	IPAddress addr(ip,port);
	return bind(addr);
}

bool Socket::bind(const IPAddress& addr)
{
	return socket_detail::bind(impl,addr);
}

bool Socket::get_peer(IPAddress& addr)
{
	return socket_detail::getpeername(impl,addr);
}

bool Socket::get_addr(IPAddress& addr)
{
	return socket_detail::getsockname(impl,addr);
}

bool Socket::listen()
{
	return socket_detail::listen(impl);
}

bool Socket::listen(const IPAddress& addr)
{
	if(!bind(addr))
	{
		return false;
	}

	if(!listen())
	{
		return false;
	}

	return true;
}

bool Socket::listen(const String& ip,int port)
{
	IPAddress addr(ip,port);
	return listen(addr);
}

void Socket::close()
{
	impl.reset();
}

void Socket::reset()
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
