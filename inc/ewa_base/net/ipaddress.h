#ifndef __H_EW_NET_IPADDRESS__
#define __H_EW_NET_IPADDRESS__

#include "ewa_base/config.h"


#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif

#ifdef EW_WINDOWS
#include <winsock2.h>
#include <mswsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

EW_ENTER


class DLLIMPEXP_EWA_BASE IPAddress
{
public:

	IPAddress();
	IPAddress(const IPAddress&);
	IPAddress(const String& ip,int port);
	~IPAddress();

	const IPAddress& operator=(const IPAddress& o);

	void service(const String& addr,int port);

	String host();
	int port();

	void swap(IPAddress& addr);

	struct sockaddr* sk_addr();
	struct sockaddr_in* sk_addr_in();
	const struct sockaddr* sk_addr() const;
	const struct sockaddr_in* sk_addr_in() const;
	int* sz_ptr();
	int v4size() const;

	operator struct sockaddr*()
	{
		return sk_addr();
	}
	operator struct sockaddr_in*()
	{
		return sk_addr_in();
	}
	operator const struct sockaddr*() const
	{
		return sk_addr();
	}
	operator const struct sockaddr_in*() const
	{
		return sk_addr_in();
	}
	operator int*()
	{
		return sz_ptr();
	}

private:
	void ensure() const;
	mutable struct sockaddr_in* pimpl;
	mutable int nsize;
};

EW_LEAVE
#endif
