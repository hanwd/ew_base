#include "ewa_base/net/session.h"
#include "ewa_base/net/iocp1.h"

EW_ENTER


#ifdef EW_WINDOWS
static DWORD iocp_flag_recv;
#endif



SessionUDP::SessionUDP()
{
	tsTimeout=TimeSpan::Day(120);
}

SessionUDP::~SessionUDP()
{

}

bool SessionUDP::AsyncSend(const char* data,size_t size)
{
	return AsyncSend(data,size,sk_local.peer);
}

bool SessionUDP::AsyncSend(const char* data,size_t size,IPAddress& addr)
{
	if(size>4096)
	{
		System::LogTrace("size>4096 in %s",__FUNCTION__);
		return false;
	}

	TempPtrT<MyOverLappedEx> q=lkfq_free.getq();
	if(!q)
	{
		q.reset(new MyOverLappedEx);
	}

	q->size=size;
	q->peer=addr;
	memcpy(q->buffer,data,size);
	return AsyncSend(q);
}

bool SessionUDP::AsyncSend(TempOlapPtr& q)
{
	if(!q)
	{
		return false;
	}

	MyOverLappedEx &idat(*q);
	idat.type=MyOverLapped::ACTION_UDP_SEND;
	idat.dbuf[0].buf=idat.buffer;
	idat.dbuf[0].len=idat.size;

	return DoAsyncSend(q);

}

bool SessionUDP::DoAsyncSend(TempOlapPtr& q)
{
	if(!lkfq_send.putq(q))
	{
		System::LogTrace("lkfq_send is full, packet dropped!");
		return false;
	}
	else
	{
		if(m_nPendingSend++==0)
		{
			DoAsyncSend();
		}
		return true;
	}
}

bool SessionUDP::DoAsyncRecv(TempOlapPtr& q)
{
	if(!lkfq_recv.putq(q))
	{
		System::LogTrace("lkfq_send is full, packet dropped!");
		return false;
	}
	else
	{
		if(m_nPendingRecv++==0)
		{
			DoAsyncRecv();
		}
		return true;
	}
}

bool SessionUDP::AsyncRecv(TempOlapPtr& q)
{
	if(!q)
	{
		return false;
	}

	MyOverLappedEx &idat(*q);
	idat.type=MyOverLapped::ACTION_UDP_RECV;
	idat.dbuf[0].buf=idat.buffer;
	idat.dbuf[0].len=4096;

	return DoAsyncRecv(q);

}

bool SessionUDP::AsyncRecv()
{
	TempPtrT<MyOverLappedEx> q=lkfq_free.getq();
	if(!q)
	{
		q.reset(new MyOverLappedEx);
	}
	return AsyncRecv(q);
}


void SessionUDP::DoAsyncSend()
{

#ifdef EW_WINDOWS

	TempPtrT<MyOverLappedEx> q=lkfq_send.getq();
	if(!q)
	{
		return;
	}

	MyOverLappedEx &idat(*q);
	int bRet=WSASendTo(sk_local.sock,
					   &idat.dbuf[0],
					   idat.dbuf[1].buf?2:1,
					   &idat.size,
					   0,
					   idat.peer,
					   idat.peer.v4size(),
					   &idat.olap,
					   NULL);

	if(bRet!=0 && WSAGetLastError()!=WSA_IO_PENDING)
	{
		System::CheckError("WSASend Error");
		Disconnect();
		--m_nPendingSend;
		return;
	}
	else
	{
		q.release();
	}

#else
	ep_ctl(EPOLLOUT);
#endif
}


void SessionUDP::DoAsyncRecv()
{

#ifdef EW_WINDOWS


	TempPtrT<MyOverLappedEx> q=lkfq_recv.getq();
	if(!q)
	{
		return;
	}

	MyOverLappedEx &idat(*q);

	int bRet=WSARecvFrom(sk_local.sock,
						 &idat.dbuf[0],
						 idat.dbuf[1].buf?2:1,
						 &idat.size,
						 &iocp_flag_recv,
						 idat.peer,
						 idat.peer,
						 &idat.olap,
						 NULL);

	if(bRet!=0 && WSAGetLastError()!=WSA_IO_PENDING)
	{
		--m_nPendingRecv;
		Disconnect();
		return;
	}
	else
	{
		q.release();
	}

#else
	ep_ctl(EPOLLIN);
#endif
}


void SessionUDP::OnConnected()
{

}

void SessionUDP::OnDisconnected()
{

}

bool SessionUDP::Bind(const String& ip,int port)
{
	IPAddress addr(ip,port);
	return Bind(addr);
}


bool SessionUDP::Bind(IPAddress& addr)
{
	if(state.get()!=STATE_READY)
	{
		return false;
	}

	sk_local.addr=addr;
	sk_local.sock.ensure(Socket::UDP);
	if(!sk_local.sock.bind(addr))
	{
		return false;
	}

	return true;
}

bool SessionUDP::Connect(const String& ip,int port)
{
	if(state.get()!=STATE_READY)
	{
		return false;
	}

	sk_local.peer.service(ip,port);
	sk_local.sock.ensure(Socket::UDP);
	if(!sk_local.sock.connect(sk_local.peer))
	{
		return false;
	}

	return true;
}


EW_LEAVE
