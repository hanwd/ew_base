#ifndef EW_WINDOWS

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <cerrno>

EW_ENTER

class SocketImpl_linux
{
public:

	typedef Socket::impl_type impl_type;
	typedef int sock_type;

	static sock_type g(impl_type& impl)
	{
		impl_type::type p=impl.get();
		return (sock_type)((char*)p-(char*)NULL);
	}

	static bool create(impl_type& impl,int t)
	{
		sock_type p=socket(PF_INET,t,0);
		if(p>=0)
		{
			int opt =1;
			setsockopt(p,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
			impl.reset(p);
			return true;
		}
		else
		{
			impl.reset();
			return false;
		}
	}

	static bool is_connected(impl_type& impl)
	{
		if (!impl.ok()) return false;
		int optval;
		socklen_t optlen = sizeof(int);
		int ret = getsockopt(impl.get(), SOL_SOCKET, SO_ERROR, (char*)&optval, &optlen);
		return (!ret && !optval);
	}

	static bool ensure(impl_type& impl,int t=-1)
	{
		if(impl.ok())
		{
			if(t==impl.protocol||t==-1) return true;
			impl.reset();
		}
		else if(t==-1)
		{
			t=Socket::TCP;
		}

		return create(impl,t);
	}

	static bool connect(impl_type& impl,const IPAddress& addr)
	{
		if(!ensure(impl)) return false;
		int r=::connect(g(impl),addr,addr.v4size());
		return r>=0;
	}

	static bool block(impl_type& impl,bool f)
	{
		if(!ensure(impl)) return false;
		int sockfd=impl.get();

		int flag=fcntl(sockfd, F_GETFD, 0);
		if(f) flag|=O_NONBLOCK;
		else flag&=~O_NONBLOCK;

		if (fcntl(sockfd, F_SETFL, flag) == -1)
		{
			return false;
		}

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
		socklen_t la=addr.v4size();
		sock_type _tmp=::accept(g(impl),addr,&la);
		if(_tmp<0)
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
		return r>=0;
	}

	static bool getsockname(impl_type& impl,IPAddress& addr)
	{
		if(!impl.ok()) return false;
		socklen_t naddr=addr.v4size();
		int r=::getsockname(g(impl),addr,&naddr);
		if(r<0) return false;
		return true;
	}

	static bool getpeername(impl_type& impl,IPAddress& addr)
	{
		if(!impl.ok()) return false;
		socklen_t naddr=addr.v4size();
		int r=::getpeername(g(impl),addr,&naddr);
		if(r<0) return false;
		return true;
	}

	static bool listen(impl_type& impl)
	{
		if(!impl.ok()) return false;
		int r=::listen(g(impl),1024);
		return r>=0;
	}

	static int send(impl_type& impl,const char* buf,int len)
	{
		int offset = 0;
		while (offset < len)
		{
			int result = ::send(g(impl), buf + offset, len - offset, 0);
			if (result == 0)
			{
				break;
			}
			else if (result > 0)
			{
				offset += result;
			}
			else
			{
				if (errno == EINTR) continue;
				return 0;
			}
		}
		return offset;
	}

	static int recv(impl_type& impl,char* buf,int len)
	{
		if (len <= 0) return 0;
		EW_ASSERT(buf!=NULL);
		while (true)
		{
			int result = ::recv(g(impl), buf, len, 0);
			if (result < 0 && errno == EINTR) continue;  // Retry after signal.
			return result;
		}
	}

	static int send(impl_type& impl,const char* buf,int len,const IPAddress& addr)
	{
		return ::sendto(g(impl),buf,len,0,addr,addr.v4size());
	}

	static int recv(impl_type& impl,char* buf,int len,IPAddress& addr)
	{
		socklen_t nlen=addr.v4size();
		while(true)
		{
			int result=::recvfrom(g(impl),buf,len,0,addr,&nlen);
			if (result < 0 && errno == EINTR) continue;  // Retry after signal.
			return result;
		}
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
			if(ret<0)
			{
				if(errno == EINTR) continue;
				return -1;
			}
			buf+=ret;
			total+=ret;
		}

		return total;

	}

	static int recv_msg(impl_type& impl,char* buf,int len)
	{
		return ::recv(g(impl),buf,len,MSG_WAITALL);
	}

	static void destroy(impl_type::type& o)
	{
		sock_type s=o;
		if(s>=0) close(s);
	}

	static void shutdown(impl_type& impl,int t)
	{
		if(impl.ok())
		{
			if(t==0)
			{
				::shutdown(g(impl),SHUT_RD);
			}
			else if(t==1)
			{
				::shutdown(g(impl),SHUT_WR);
			}
			else
			{
				::shutdown(g(impl),SHUT_RDWR);
			}
		}
	}
};

EW_LEAVE

#endif
