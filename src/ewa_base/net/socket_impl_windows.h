#include "ewa_base/config.h"

#ifdef EW_WINDOWS
#include <WinSock2.h>
#include <Ws2tcpip.h>

EW_ENTER
class SocketImpl_windows
{
public:


	typedef Socket::impl_type impl_type;
	typedef SOCKET sock_type;

	static sock_type g(impl_type& impl)
	{
		impl_type::type p=impl;
		return (sock_type)p;
	}

	static bool create(impl_type& impl,int t)
	{
		impl.protocol=t;
		sock_type p=t==Socket::TCP?socket(AF_INET, t, IPPROTO_TCP):socket(AF_INET, t, IPPROTO_UDP);
		if(p!=INVALID_SOCKET)
		{
			SetHandleInformation((HANDLE)p, HANDLE_FLAG_INHERIT, 0);
			impl.reset(p);
			return true;
		}
		else
		{
			impl.close();
			return false;
		}
	}

	static bool is_connected(impl_type& impl)
	{
		if (!impl.ok()) return false;
		int optval;
		int optlen = sizeof(int);
		int ret = getsockopt(impl, SOL_SOCKET, SO_ERROR, (char*)&optval, &optlen);
		return (!ret && !optval);
	}

	static bool ensure(impl_type& impl,int t=-1)
	{
		if(t==-1)
		{
			t=impl.protocol;
		}

		if(impl.ok())
		{
			if(t==impl.protocol) return true;
			impl.close();
		}

		return create(impl,t);
	}

	static bool connect(impl_type& impl,const IPAddress& addr)
	{
		if(!ensure(impl)) return false;
		int r=::connect(g(impl),addr,addr.v4size());
		return r!=SOCKET_ERROR;
	}

	static void shutdown(impl_type& impl,int t)
	{
		if(impl.ok())
		{
			if(t==0)
			{
				::shutdown(g(impl),SD_RECEIVE);
			}
			else if(t==1)
			{
				::shutdown(g(impl),SD_SEND);
			}
			else
			{
				::shutdown(g(impl),SD_BOTH);
			}
		}
	}

	static bool block(impl_type& impl,bool f)
	{
		if(!ensure(impl)) return false;
		u_long ul=f?0:1;
		ioctlsocket(g(impl),FIONBIO,&ul);
		return true;
	}

	static bool reuseaddr(impl_type& impl,bool f)
	{
		if(!ensure(impl)) return false;
		int v = f ? 1 : 0;
		int result = ::setsockopt(g(impl), SOL_SOCKET, SO_REUSEADDR,reinterpret_cast<char*>(&v), sizeof(v));
		return result == 0;
	}

	static bool accept(impl_type& impl,impl_type& sock,IPAddress& addr)
	{
		if(!impl.ok()) return false;
		SOCKET _tmp=::accept(g(impl),addr,addr);
		if(_tmp==SOCKET_ERROR)
		{
			return false;
		}
		sock.reset(_tmp);
		return true;
	}

	static bool bind(impl_type& impl,const IPAddress& addr)
	{
		if(!ensure(impl)) return false;
		int r=::bind(g(impl),addr,addr.v4size());
		return r!=SOCKET_ERROR;
	}

	static bool getsockname(impl_type& impl,IPAddress& addr)
	{
		if(!impl.ok()) return false;
		int r=::getsockname(g(impl),addr,addr);
		return r!=SOCKET_ERROR;
	}

	static bool getpeername(impl_type& impl,IPAddress& addr)
	{
		if(!impl.ok()) return false;
		int r=::getpeername(g(impl),addr,addr);
		return r!=SOCKET_ERROR;
	}

	static bool listen(impl_type& impl)
	{
		if(!impl.ok()) return false;
		int r=::listen(g(impl),SOMAXCONN);
		return r!=SOCKET_ERROR;
	}

	static int send(impl_type& impl,const char* buf,int len)
	{
		return ::send(g(impl),buf,len,0);
	}

	static int recv(impl_type& impl,char* buf,int len)
	{
		return ::recv(g(impl),buf,len,0);
	}

	static int send(impl_type& impl,const char* buf,int len,const IPAddress& addr)
	{
		return ::sendto(g(impl),buf,len,0,addr,addr.v4size());
	}

	static int recv(impl_type& impl,char* buf,int len,IPAddress& addr)
	{
		return ::recvfrom(g(impl),buf,len,0,addr,addr);
	}

	static int peek(impl_type& impl,char* buf,int len)
	{
		return ::recv(g(impl),buf,len,MSG_PEEK);
	}

	static int send_msg(impl_type& impl,const char* buf,int len)
	{
		int total=0;

		while(total<len)
		{
			int ret=::send(g(impl),buf,len-total,0);
			if(ret==SOCKET_ERROR)
			{
				return -1;
			}
			buf+=ret;
			total+=ret;
		}

		return total;

	}

	static int recv_msg(impl_type& impl,char* buf,int len)
	{
#ifdef EW_MSVC
		return ::recv(g(impl),buf,len,MSG_WAITALL);
#else
		return ::recv(g(impl),buf,len,0);
#endif
	}

	static void destroy(impl_type::type& o)
	{
		::closesocket(o);
	}
};

EW_LEAVE

#endif

