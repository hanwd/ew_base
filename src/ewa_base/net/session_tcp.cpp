#include "ewa_base/basic/system.h"
#include "ewa_base/basic/console.h"
#include "ewa_base/net/session.h"
#include "ewa_base/net/iocp1.h"
#include "ewa_base/threading/thread_mutex.h"


#ifndef EW_WINDOWS
#include <cerrno>
#endif

EW_ENTER

SessionTCP::SessionTCP()
{

}

bool SessionTCP::AsyncSend(const char* data,size_t size)
{
	while (size > 0)
	{
		TempOlapPtr q = lkfq_free.getq();
		if (!q)
		{
			q.reset(new MyOverLappedEx);
		}

		size_t sz=std::min(size,IPacket::MAX_PACKET_SIZE);

		q->flags = size > IPacket::MAX_PACKET_SIZE ? 1:0;
		q->size = sz;

		memcpy(q->buffer, data, sz);
		if (!AsyncSend(q))
		{
			return false;
		}

		size -= sz;
		data += sz;
	}
	return true;
}

bool SessionTCP::AsyncSend(TempOlapPtr& q)
{
	if(!q)
	{
		return false;
	}

	MyOverLappedEx &idat(*q);

	idat.type=MyOverLapped::ACTION_TCP_SEND;
	idat.dbuf[0].buf=idat.buffer;
	idat.dbuf[0].len=idat.size;

	return DoAsyncSend(q);

}

bool SessionTCP::DoAsyncSend(TempOlapPtr& q)
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

bool SessionTCP::DoAsyncRecv(TempOlapPtr& q)
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

bool SessionTCP::AsyncRecv(TempOlapPtr& q)
{
	if(!q)
	{
		return false;
	}

	MyOverLappedEx &idat(*q);
	idat.type=MyOverLapped::ACTION_TCP_RECV;
	idat.dbuf[0].buf=idat.buffer;
	idat.dbuf[0].len=4096;

	return DoAsyncRecv(q);

}

bool SessionTCP::AsyncRecv()
{
	TempPtrT<MyOverLappedEx> q=lkfq_free.getq();
	if(!q)
	{
		q.reset(new MyOverLappedEx);
	}
	return AsyncRecv(q);
}


bool SessionTCP::WaitForSend()
{

	TempPtrT<MyOverLappedEx> q=lkfq_free.getq();
	if(!q)
	{
		q.reset(new MyOverLappedEx);
	}
	MyOverLappedEx &idat(*q);
	idat.type=MyOverLapped::ACTION_WAIT_SEND;
	idat.dbuf[0].buf=idat.buffer;
	idat.dbuf[0].len=0;

	return DoAsyncSend(q);
}

bool SessionTCP::WaitForRecv()
{

	TempPtrT<MyOverLappedEx> q=lkfq_free.getq();
	if(!q)
	{
		q.reset(new MyOverLappedEx);
	}
	MyOverLappedEx &idat(*q);
	idat.type=MyOverLapped::ACTION_WAIT_RECV;
	idat.dbuf[0].buf=idat.buffer;
	idat.dbuf[0].len=0;

	return DoAsyncRecv(q);

}

#ifdef EW_WINDOWS
static DWORD iocp_flag_recv=0;
#endif


void SessionTCP::DoAsyncSend()
{

#ifdef EW_WINDOWS

	TempPtrT<MyOverLappedEx> q=lkfq_send.getq();
	if(!q)
	{
		return;
	}

	MyOverLappedEx &idat(*q);

	int bRet=WSASend(sk_local.sock,
					 &idat.dbuf[0],
					 idat.dbuf[1].buf?2:1,
					 &idat.size,
					 0,
					 &idat.olap,
					 NULL);

	if(bRet!=0 && WSAGetLastError()!=WSA_IO_PENDING)
	{
		--m_nPendingSend;
		Disconnect();
	}
	else
	{
		q.release();
	}

#else

	ep_ctl(EPOLLOUT);

#endif
}


void SessionTCP::DoAsyncRecv()
{

#ifdef EW_WINDOWS

	TempPtrT<MyOverLappedEx> q=lkfq_recv.getq();
	if(!q)
	{
		return;
	}

	MyOverLappedEx &idat(*q);

	int bRet=WSARecv(sk_local.sock,
					 &idat.dbuf[0],
					 idat.dbuf[1].buf?2:1,
					 &idat.size,
					 &iocp_flag_recv,
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


EW_LEAVE
