
#include "ewa_base/net/session.h"
#include "ewa_base/net/iocp1.h"
#include "ewa_base/basic/system.h"

EW_ENTER

SessionServer::SessionServer()
{
	tsTimeout=TimeSpan::Day(120);
}

bool SessionServer::Listen(const String& ip,int port)
{
	Logger &logger(this_logger());

	if(state.get()!=STATE_READY)
	{
		logger.LogError("Listen failed! Server is already connected!");
		return false;
	}

	sk_local.addr.service(ip,port);
	if(!sk_local.sock.Listen(sk_local.addr))
	{
		logger.LogError("Listen failed on %s:%d",sk_local.addr.host(),sk_local.addr.port());
		return false;
	}

	sk_local.sock.Block(false);

#ifdef _MSC_VER
	DWORD dwBytes;
	DWORD iResult;

	GUID GuidAcceptEx = WSAID_ACCEPTEX;

	iResult = WSAIoctl(sk_local.sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
					   &GuidAcceptEx, sizeof (GuidAcceptEx),
					   &lpfnAcceptEx, sizeof (lpfnAcceptEx),
					   &dwBytes, NULL, NULL);

	if (iResult == SOCKET_ERROR)
	{
		int ret=WSAGetLastError();
		System::LogTrace("WSAIoctl(WSAID_ACCEPTEX) failed with error %d",ret);
		return false;
	}

	GUID GuidGetAddrEx = WSAID_GETACCEPTEXSOCKADDRS;

	iResult = WSAIoctl(sk_local.sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
					   &GuidGetAddrEx, sizeof (GuidGetAddrEx),
					   &lpfnGetAddrEx, sizeof (lpfnGetAddrEx),
					   &dwBytes, NULL, NULL);

	if (iResult == SOCKET_ERROR)
	{
		int ret=WSAGetLastError();
		System::LogTrace("WSAIoctl(WSAID_GETACCEPTEXSOCKADDRS) failed with error %d",ret);
		return false;
	}

#endif

	return true;
}

void SessionServer::OnConnected()
{
	WaitForAccept();
}

void SessionServer::OnDisconnected()
{
	basetype::OnDisconnected();
}

void SessionServer::Disconnect()
{
	basetype::Disconnect();
}

void SessionServer::Close()
{
	sk_local.sock.Close();
}


void SessionServer::StartSession(Session* pkey,IOCPPool* hpool)
{
	if(!hpool)
	{
		hpool=hiocp;
	}

	//this_logger().LogMessage("%p connected %d",pkey,hpool->GetAccounter().nSessionCount.get());
	hpool->Register(pkey);
}

MyOverLapped olap_wait_for_accept(MyOverLapped::ACTION_ACCEPT);

bool SessionServer::WaitForAccept()
{

#ifdef _MSC_VER

	m_nPendingRecv++;

	MyOverLapped& idat(olap_wait_for_accept);

	sk_remote.sock.Ensure(Socket::TCP);

	int bRet = lpfnAcceptEx(
				   sk_local.sock,
				   sk_remote.sock,
				   tmp_buffer,
				   0,
				   sizeof(sockaddr_in) + 16,
				   sizeof(sockaddr_in) + 16,
				   &idat.size,
				   &idat.olap);

	if(bRet!=0 && WSAGetLastError()!=WSA_IO_PENDING)
	{
		Disconnect();
		--m_nPendingRecv;
		int ret=WSAGetLastError();
		System::LogTrace("AcceptEx failed with error %d",ret);
		return false;
	}
	else
	{
		return true;
	}
#elif defined(EW_WINDOWS)

#else
	WaitForRecv();
	return true;
#endif


}


void SessionServer::OnRecvReady()
{

#ifndef _MSC_VER

	for(;;)
	{
		sk_remote.sock.Ensure(Socket::TCP);
		if(!sk_local.sock.Accept(sk_remote.sock,sk_remote.peer))
		{
			break;
		}
		NewSession(sk_remote);
		break;
	}

#else

	sockaddr_in* addr=NULL;
	sockaddr_in* peer=NULL;
	int len1=0;
	int len2=0;

	lpfnGetAddrEx(
		tmp_buffer,
		0,
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		(LPSOCKADDR*)&addr,
		&len1,
		(LPSOCKADDR*)&peer,
		&len2);

	if(addr) memcpy(sk_remote.addr.sk_addr_in(),addr,len1);
	if(peer) memcpy(sk_remote.peer.sk_addr_in(),peer,len2);

	NewSession(sk_remote);

	for(;;)
	{
		sk_remote.sock.Ensure(Socket::TCP);
		if(!sk_local.sock.Accept(sk_remote.sock,sk_remote.peer))
		{
			break;
		}

		NewSession(sk_remote);
	}

#endif

	WaitForAccept();
}


void SessionServer::NewSession(PerIO_socket& sk)
{
	sk.sock.Shutdown();
}




EW_LEAVE
