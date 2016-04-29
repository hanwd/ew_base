#include "ewa_base/net/session.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/basic/console.h"
#include "ewa_base/net/iocp1.h"
#include "ewa_base/threading/thread_mutex.h"
#include "ewa_base/memory/mempool.h"

#ifndef EW_WINDOWS
#include <cerrno>
#endif

EW_ENTER

Session::LKFQueue Session::lkfq_free(2048,Session::LKFQueue::QUEUE_NONBLOCK);


Session::Session()
{
#ifndef EW_WINDOWS
	m_nLastEpoll_def.clr(EPOLLERR|EPOLLHUP);
	m_nTempEpoll_ctl=0;
#endif

	lkfq_send.reset(1024);
	lkfq_recv.reset(1024);
	lkfq_send.flags().add(LKFQueue::QUEUE_NONBLOCK);
	lkfq_recv.flags().add(LKFQueue::QUEUE_NONBLOCK);

	tsTimeout=TimeSpan::Seconds(60);

}


Session::~Session()
{

}

bool Session::HasPending()
{
#ifdef EW_WINDOWS
	if(m_nPendingRecv.get()!=0 || m_nPendingSend.get()!=0)
	{
		return true;
	}
	return false;
#else
	return false;
#endif

}

void Session::OnSendCompleted(TempOlapPtr& q)
{
	lkfq_free.putq(q);
}

void Session::OnRecvCompleted(TempOlapPtr& q)
{
	lkfq_free.putq(q);
}


void Session::OnSendReady()
{

}

void Session::OnRecvReady()
{

}


void Session::OnConnected()
{

}

void Session::OnDisconnected()
{

}


bool Session::IsConnected()
{
	return state.get()==STATE_OK;
}

bool Session::IsError()
{
	return state.get()==STATE_DISCONNECT;
}

bool Session::TestTimeout(TimePoint& tp,TempOlapPtr&)
{
	if(tp-tpLast>tsTimeout)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Session::Disconnect()
{

	int val=STATE_OK;
	if(!state.compare_exchange(val,STATE_DISCONNECT))
	{
		return;
	}

#ifdef EW_WINDOWS
	sk_local.sock.Close();
	m_nPendingRecv.fetch_add(1);
	PostQueuedCompletionStatus(hiocp->native_handle(),0,(ULONG_PTR)this,NULL);
#else
	sk_local.sock.Shutdown();
#endif

	TempOlapPtr q;
	for(;;)
	{
		q=lkfq_recv.getq();
		if(!q) break;
		lkfq_free.putq(q);
		m_nPendingRecv--;
	}

	for(;;)
	{
		q=lkfq_send.getq();
		if(!q) break;
		lkfq_free.putq(q);
		m_nPendingSend--;
	}

}

#ifndef EW_WINDOWS

void Session::ep_ctl(int f)
{

	if(state.get()!=STATE_OK)
	{
		return;
	}

	int _def_ctl=EPOLLERR|EPOLLHUP|EPOLLET;//|EPOLLONESHOT;
	int _tmp_ctl=_def_ctl;//|EPOLLONESHOT;//|EPOLLONESHOT;//m_nLastEpoll_def.val();

	if(m_nPendingSend.get()>0)
	{
		_tmp_ctl=_tmp_ctl|EPOLLOUT;
	}

	if(m_nPendingRecv.get()>0)
	{
		_tmp_ctl=_tmp_ctl|EPOLLIN;
	}

	if(_tmp_ctl==_def_ctl)
	{
		return;
	}

	m_nLastEpoll_ctl=_tmp_ctl;

	struct epoll_event ev;
	ev.data.ptr=this;
	ev.events=_tmp_ctl;


	int bRet=epoll_ctl(hiocp->native_handle(),EPOLL_CTL_MOD,sk_local.sock.native_handle(),&ev);
	if(bRet<0)
	{
		this_logger().LogError("epoll_ctl failed: %s",strerror(errno));
		Disconnect();
	}


}
#endif


EW_LEAVE
