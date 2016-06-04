#ifndef __H_EW_NET_SOCKET__
#define __H_EW_NET_SOCKET__


#include "ewa_base/basic/platform.h"
#include "ewa_base/net/ipaddress.h"

#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif



#ifndef EW_WINDOWS
#include <sys/socket.h>
#include <netinet/in.h>
#endif



EW_ENTER

class DLLIMPEXP_EWA_BASE IPAddress;

class DLLIMPEXP_EWA_BASE KO_Policy_socket
{
public:
#ifdef EW_WINDOWS
	typedef SOCKET type;
	static type invalid_value()
	{
		return INVALID_SOCKET;
	}
#else
	typedef int type;
	static type invalid_value()
	{
		return -1;
	}
#endif

	typedef type const_reference;
	static void destroy(type& o);
};

class DLLIMPEXP_EWA_BASE Socket
{
public:

	enum
	{
		DEF	=0,
		TCP	=1,
		UDP	=2,
	};

	class impl_type : public KO_Handle<KO_Policy_socket>
	{
	public:
		impl_type()
		{
			protocol=TCP;
		}
		int protocol;
	};

	Socket();
	~Socket();

	void ensure(int t=TCP);

	bool bind(const IPAddress& addr);
	bool bind(const String& ip,int port);

	// server
	bool listen();
	bool listen(const IPAddress& addr);
	bool listen(const String& ip,int port);

	bool accept(Socket& sock);
	bool accept(Socket& sock,IPAddress& addr);

	// client
	bool connect(const String& ip,int port);
	bool connect(const IPAddress& addr);


	int send(const char* buf,int len);
	int recv(char* buf,int len);

	int peek(char* buf,int len);

	int send_all(const char* buf,int len);
	int recv_all(char* buf,int len);

	// udp send/recv
	int send(const char* buf,int len,const IPAddress& addr);
	int recv(char* buf,int len,IPAddress& addr);

	bool is_connected();

	bool get_peer(IPAddress& addr);
	bool get_addr(IPAddress& addr);

	void shutdown(int t=-1);
	void close();

	void block(bool f);
	void reuse_address(bool f);

	operator impl_type::const_reference()
	{
		return impl.get();
	}

	void reset(impl_type::const_reference s)
	{
		impl.reset(s);
	}

	void swap(Socket& s)
	{
		impl.swap(s.impl);
	}

	void reset();

	impl_type::type native_handle()
	{
		return impl.get();
	}

private:
	impl_type impl;
};

EW_LEAVE
#endif
