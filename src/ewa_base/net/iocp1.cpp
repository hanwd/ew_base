
#include "ewa_base/net/session.h"
#include "ewa_base/net/iocp1.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/basic/lockguard.h"
#include "ewa_base/threading/thread_mutex.h"
#include "ewa_base/basic/pointer.h"

#ifndef EW_WINDOWS
#include <sys/epoll.h>
#include <cerrno>
#endif

EW_ENTER

bool IOCPPool::Execute(IocpCommand* pcmd)
{
	DataPtrT<IocpCommand> kcmd(pcmd);
	return m_lkfqCommand.putq(kcmd);
}

int IOCPPool::Register(Session* pkey)
{
	DataPtrT<Session> h(pkey);
	if(!pkey || pkey->state.get()!=Session::STATE_READY)
	{
		return -1;
	}


	int n=m_lkfqSessionAvailable.getq();
	if(n<1) //LockFreeQueue<int>::invalid_value()) // max pkey reached
	{
		System::LogTrace("max session reached! connection refused");
		return -1;
	}

	if(m_aSessions[n])
	{
		System::LogError("Session[%d]is in use???",n);
		return -1;
	}

	pkey->sk_local.sock.block(false);
	pkey->hiocp.reset(this);
	pkey->tpLast=accounter.tTimeStamp;

#ifdef EW_WINDOWS
	CreateIoCompletionPort(*(HANDLE*)&pkey->sk_local.sock, hIOCPhandler.get(), (ULONG_PTR)pkey, 0);
	pkey->state.store(Session::STATE_OK);
#else
	pkey->tmp_send.reset(NULL);
	pkey->state.store(Session::STATE_OK);
	struct epoll_event ev;
	ev.data.ptr=pkey;
	ev.events=EPOLLERR|EPOLLHUP;//|EPOLLIN;//|EPOLLONESHOT|EPOLLET;
	int bRet=epoll_ctl(hIOCPhandler.get(),EPOLL_CTL_ADD,pkey->sk_local.sock,&ev);
	if(bRet==-1)
	{
		pkey->state.store(Session::STATE_READY);
		logger.LogError("epoll_ctl failed:%s",strerror(errno));
		return -1;
	}

#endif

	if(accounter.nSessionCount++==0)
	{
		m_nCanClose.reset();
	}

	m_aSessions[n].swap(h);
	pkey->tpLast=accounter.tTimeStamp;
	pkey->OnConnected();

	return n;



}


IOCPPool::IOCPPool(const String& name_,int maxconn_):m_sName(name_)
{
	if(maxconn_<16)
	{
		maxconn_=16;
	}

	m_pAccounterLast.reset(new IOCPAccounter);
	m_pAccounterTemp.reset(new IOCPAccounter);

	accounter.tTimeStamp=Clock::now();

	m_lkfqCommand.reset(1024);
	m_lkfqCommand.flags().add(LockFreeQueue<IocpCommand*>::QUEUE_NONBLOCK);

	m_nSessionMax=maxconn_;
	m_lkfqSessionAvailable.reset(m_nSessionMax);
	m_lkfqSessionAvailable.flags().add(LockFreeQueue<int>::QUEUE_NONBLOCK);
	m_aSessions.resize(m_nSessionMax);

	for(int i=1; i<m_nSessionMax; i++)
	{
		m_lkfqSessionAvailable.putq(i);
	}


	flags.add(FLAG_DYNAMIC);

	m_nCanClose.set();

#ifdef EW_WINDOWS
	HANDLE h1 = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if(h1!=NULL)
	{
		hIOCPhandler.reset(h1);
	}
	else
	{
		System::LogTrace("CreateIoCompletionPort failed");
	}
#else
	int h1=epoll_create(1024);
	if(h1!=-1)
	{
		hIOCPhandler.reset(h1);
	}
	else
	{
		System::LogTrace("epoll_create failed");
	}
#endif
}

bool IOCPPool::activate(int n)
{
	if(!hIOCPhandler.get())
	{
		return false;
	}

	ThreadEx::InvokerGroup g;

	g.resize(n,ew::hbind(&IOCPPool::svc_worker,this));
	g.append(ew::hbind(&IOCPPool::svc_checker,this));

	return basetype::activate(g);
}


void IOCPPool::ccc_update_info()
{
	accounter.tTimeStamp=Clock::now();
	if(accounter.tTimeStamp-m_pAccounterLast->tTimeStamp<TimeSpan::Seconds(2))
	{
		return;
	}
	*m_pAccounterTemp=accounter;


	uint32_t _sc=(uint32_t)(m_pAccounterTemp->nSendCount.get()-m_pAccounterLast->nSendCount.get());
	uint32_t _rc=(uint32_t)(m_pAccounterTemp->nRecvCount.get()-m_pAccounterLast->nRecvCount.get());
	uint64_t _sb=(uint64_t)(m_pAccounterTemp->nSendBytes.get()-m_pAccounterLast->nSendBytes.get());
	uint64_t _rb=(uint64_t)(m_pAccounterTemp->nRecvBytes.get()-m_pAccounterLast->nRecvBytes.get());

	double _ts=(m_pAccounterTemp->tTimeStamp-m_pAccounterLast->tTimeStamp)/TimeSpan::Seconds(1);
	double _pps=double(_sc+_rc)/_ts;
	double _bps=double(_sb+_rb)/_ts;

	if(_pps>5.0e3)
	{
		logger.LogMessage("%s ssc: %d,pps: %gW, bps: %gM",m_sName,accounter.nSessionCount.get(),_pps/1.0e4,_bps/(1024.0*1024.0));
	}
	else
	{
		logger.LogMessage("%s ssc: %d,pps: %g, bps: %gM",m_sName,accounter.nSessionCount.get(),_pps,_bps/(1024.0*1024.0));
	}

	m_pAccounterTemp.swap(m_pAccounterLast);

}

void IOCPPool::ccc_handle_sock()
{
	accounter.tTimeStamp=Clock::now();

	TempOlapPtr q(Session::lkfq_free.getq());
	if(!q) q.reset(new MyOverLappedEx);

	for(int i=1; i<m_nSessionMax; i++)
	{
		Session* pkey=m_aSessions[i].get();
		if(!pkey) continue;

		int s=pkey->state.get();
		switch(s)
		{
		case Session::STATE_OK:
			if(pkey->TestTimeout(accounter.tTimeStamp,q)) //timeout
			{
				pkey->Disconnect();
			}
			continue;
		case Session::STATE_DISCONNECT:
			if(pkey->HasPending()) continue;
			svc_del(i);
			continue;

		default:
			System::LogError("Unexpected Session state[%d]",s);
			continue;
		}
	}

}

void IOCPPool::ccc_execute_cmd()
{
	for(;;)
	{
		DataPtrT<IocpCommand> cmd=m_lkfqCommand.getq();
		if(!cmd) break;
		cmd->Handle(m_aSessions);
	}
}


void IOCPPool::AttachObject(DataPtrT<IocpObject> pobj)
{
	if(!pobj) return;
	m_aObjects.insert(pobj);
}

void IOCPPool::DetachObject(DataPtrT<IocpObject> pobj)
{
	m_aObjects.erase(pobj);
}

void IOCPPool::svc_checker()
{
	while(!test_canceled())
	{
		ccc_update_info();
		ccc_handle_sock();
		ccc_execute_cmd();

		for(bst_set<DataPtrT<IocpObject> >::iterator it=m_aObjects.begin();it!=m_aObjects.end();++it)
		{
			const_cast<DataPtrT<IocpObject>&>(*it)->idle_check();
		}

		Thread::sleep_for(100);
	}

	while(m_lkfqCommand.getq());
}

IOCPPool::~IOCPPool()
{
	cancel();
	wait();
}

void IOCPPool::cancel()
{
	DisconnectAll();

	LockGuard<Mutex> lock1(m_thrd_mutex);
	m_nState=STATE_CANCEL;
	m_cond_state_changed.notify_all();
}

void IOCPPool::DisconnectAll()
{
	class IocpCommandDisconnectAll : public IocpCommand
	{
	public:
		void Handle(SessionArray& akey)
		{
			for(int i=1; i<(int)akey.size(); i++)
			{
				if(!akey[i]) continue;
				akey[i]->Disconnect();
			}
		}
	};

	Execute(new IocpCommandDisconnectAll);
}


void IOCPPool::wait_for_all_session_exit()
{
	m_nCanClose.wait();
	cancel();
	wait();
}

void IOCPPool::svc_del(int n)
{
	EW_ASSERT(n>0 && n<m_nSessionMax);
	Session* pkey=m_aSessions[n].get();
	EW_ASSERT(pkey!=NULL);
	EW_ASSERT(pkey->state.get()==Session::STATE_DISCONNECT);

#ifndef EW_WINDOWS

	struct epoll_event ev;
	epoll_ctl(hIOCPhandler.get(),EPOLL_CTL_DEL,pkey->sk_local.sock,&ev);

#endif

	pkey->state.store(Session::STATE_READY);
	pkey->sk_local.sock.close();
	pkey->OnDisconnected();
	m_aSessions[n].reset(NULL);
	m_lkfqSessionAvailable.putq(n);

	if(--accounter.nSessionCount==0)
	{
		m_nCanClose.set();
	}

}

#ifdef EW_WINDOWS

inline void IOCPPool::ccc_handle_iocp(Session* pkey,MyOverLapped* pdat)
{

	Session& ikey(*pkey);
	MyOverLapped& idat(*pdat);

	switch(idat.type)
	{
	case MyOverLapped::ACTION_UDP_SEND:
	{
		TempPtrT<MyOverLappedEx> q(static_cast<MyOverLappedEx*>(&idat));
		if(idat.size<=0)
		{
			ikey.Disconnect();
		}
		else if(!ikey.IsError())
		{
			accounter.nSendBytes.fetch_add(idat.size);
			accounter.nSendCount++;
			ikey.OnSendCompleted(q);
			static_cast<SessionUDP&>(ikey).DoAsyncSend();
		}
		--ikey.m_nPendingSend;
	}
	break;
	case MyOverLapped::ACTION_TCP_SEND:
	{
		TempPtrT<MyOverLappedEx> q(static_cast<MyOverLappedEx*>(&idat));
		if(idat.size<=0)
		{
			ikey.Disconnect();
		}
		else if(!ikey.IsError())
		{
			accounter.nSendBytes.fetch_add(idat.size);
			accounter.nSendCount++;
			ikey.OnSendCompleted(q);
			static_cast<SessionTCP&>(ikey).DoAsyncSend();
		}

		--ikey.m_nPendingSend;
	}
	break;
	case MyOverLapped::ACTION_UDP_RECV:
	{
		ikey.tpLast=accounter.tTimeStamp;
		TempPtrT<MyOverLappedEx> q(static_cast<MyOverLappedEx*>(&idat));
		if(idat.size<=0)
		{
			ikey.Disconnect();
		}
		else if(!ikey.IsError())
		{
			accounter.nRecvBytes.fetch_add(idat.size);
			accounter.nRecvCount++;
			ikey.OnRecvCompleted(q);
			static_cast<SessionUDP&>(ikey).DoAsyncRecv();
		}
		--ikey.m_nPendingRecv;
	}
	break;
	case MyOverLapped::ACTION_TCP_RECV:
	{
		ikey.tpLast=accounter.tTimeStamp;
		TempPtrT<MyOverLappedEx> q(static_cast<MyOverLappedEx*>(&idat));
		if(idat.size<=0)
		{
			ikey.Disconnect();
		}
		else if(!ikey.IsError())
		{
			accounter.nRecvBytes.fetch_add(idat.size);
			accounter.nRecvCount++;
			ikey.OnRecvCompleted(q);
			static_cast<SessionTCP&>(ikey).DoAsyncRecv();
		}
		--ikey.m_nPendingRecv;
	}
	break;
	case MyOverLapped::ACTION_ACCEPT:
	case MyOverLapped::ACTION_WAIT_RECV:
	{
		ikey.tpLast=accounter.tTimeStamp;
		if(idat.size<0)
		{
			ikey.Disconnect();
		}
		else if(!ikey.IsError())
		{
			accounter.nRecvCount++;
			ikey.OnRecvReady();
			static_cast<SessionTCP&>(ikey).DoAsyncRecv();
		}
		--ikey.m_nPendingRecv;
	}
	break;
	case MyOverLapped::ACTION_WAIT_SEND:
	{
		if(idat.size<0)
		{
			ikey.Disconnect();
		}
		else if(!ikey.IsError())
		{
			accounter.nSendCount++;
			ikey.OnSendReady();
			static_cast<SessionTCP&>(ikey).DoAsyncSend();
		}
		--ikey.m_nPendingSend;
	}
	break;
	default:
		System::LogTrace("Invalid pdat_type %d",idat.type);
		ikey.Disconnect();
		break;
	}
}

void IOCPPool::svc_worker()
{

	DWORD BytesTransferred;

	Session* pkey=NULL;
	MyOverLapped* pdat;
	BOOL bRet;

	while(!test_canceled())
	{

		bRet = GetQueuedCompletionStatus(hIOCPhandler.get(), &BytesTransferred, (PULONG_PTR)&pkey, (LPOVERLAPPED*)&pdat,100);
		if(bRet == 0)
		{
			int r=::WSAGetLastError();

			if(r==WAIT_TIMEOUT)
			{
				continue;
			}

			if(pdat==NULL)
			{
				System::LogError("NULL Overlapped, worker exit!");
				break;
			}

			if(r!=ERROR_OPERATION_ABORTED && r!=ERROR_NETNAME_DELETED && r!=1236)
			{
				System::CheckError("GetQueuedCompletionStatus");
			}

			pkey->Disconnect();

		}

		if(!pdat)
		{
			if(pkey->m_nPendingSend.get()==0 && pkey->m_nPendingRecv.get()==1)
			{
				--pkey->m_nPendingRecv;
			}
			else
			{
				PostQueuedCompletionStatus(native_handle(),0,(ULONG_PTR)pkey,NULL);
			}
		}
		else
		{
			pdat->size=BytesTransferred;
			ccc_handle_iocp(pkey,pdat);
		}

	}
}


#else


void IOCPPool::HandleSend(Session& ikey)
{

	if(ikey.m_nPendingSend.get()==0)
	{
		return;
	}

	TempOlapPtr q=ikey.tmp_send;

	for(;; q.reset(NULL))
	{
		if(!q)
		{
			q=ikey.lkfq_send.getq();
			if(!q) break;
			q->size=0;

			if(q->type==MyOverLapped::ACTION_WAIT_SEND)
			{
				ikey.m_nPendingSend--;
				ikey.OnSendReady();
				ikey.lkfq_free.putq(q);
				continue;
			}
		}

		if(q->type==MyOverLapped::ACTION_UDP_SEND)
		{
			int bRet=ikey.sk_local.sock.send(q->dbuf[0].buf,q->dbuf[0].len,q->peer);
			if(bRet>0)
			{
				q->size=bRet;
				accounter.nSendBytes.fetch_add(q->size);
				accounter.nSendCount++;
				ikey.m_nPendingSend--;
				ikey.OnSendCompleted(q);
				continue;
			}
			else if(bRet==0)
			{
				ikey.tmp_send=q;
				break;
			}
			else
			{
				ikey.Disconnect();
				break;
			}
		}

		for(;;)
		{

			int bRet=ikey.sk_local.sock.send(q->dbuf[0].buf+q->size,q->dbuf[0].len-q->size);
			if(bRet<0)
			{
				if(errno==EAGAIN)
				{
					ikey.tmp_send=q;
					break;
				}
				if(errno==EINTR)
				{
					continue;
				}

				ikey.Disconnect();
				break;
			}
			else if(bRet==0)
			{
				ikey.tmp_send=q;
				return;
			}

			q->size+=bRet;

			if(q->size==q->dbuf[0].len)
			{
				accounter.nSendCount++;
				accounter.nSendBytes.fetch_add(q->size);
				ikey.m_nPendingSend--;
				ikey.OnSendCompleted(q);
				break;
			}

		}


	}

}

void IOCPPool::HandleRecv(Session& ikey)
{
	if(ikey.m_nPendingRecv.get()==0)
	{
		return;
	}

	for(;;)
	{
		TempOlapPtr q=ikey.lkfq_recv.getq();

		if(!q)
		{
			break;
		}

		if(q->type==MyOverLapped::ACTION_WAIT_RECV)
		{
			ikey.m_nPendingRecv--;
			ikey.OnRecvReady();
			break;
		}


		if(q->type==MyOverLapped::ACTION_UDP_RECV)
		{
			int bRet=ikey.sk_local.sock.recv(q->dbuf[0].buf,q->dbuf[0].len,q->peer);
			if(bRet>0)
			{
				q->size=bRet;
				accounter.nRecvBytes.fetch_add(bRet);
				accounter.nSendCount++;

				ikey.m_nPendingRecv--;
				ikey.OnRecvCompleted(q);
				break;
			}
			else if(bRet<0)
			{
				ikey.m_nPendingRecv--;
				ikey.Disconnect();
				break;
			}
			else
			{
				ikey.m_nPendingRecv--;
				this_logger().LogMessage("recv udp failed");
			}
			break;
		}

		q->size=0;

		for(;;)
		{

			int bRet=ikey.sk_local.sock.recv(q->dbuf[0].buf+q->size,q->dbuf[0].len-q->size);
			if(bRet<0)
			{
				if(errno==EAGAIN)
				{
					break;
				}
				if(errno==EINTR)
				{
					continue;
				}

				ikey.Disconnect();
				break;
			}
			else if(bRet==0)
			{
				break;
			}

			q->size+=bRet;
			break;
		}

		if(q->size>0)
		{
			accounter.nRecvCount++;
			accounter.nRecvBytes.fetch_add(q->size);
			ikey.m_nPendingRecv--;
			ikey.OnRecvCompleted(q);
		}
		else
		{
			ikey.m_nPendingRecv--;
		}

		break;

	}

}

void IOCPPool::svc_worker()
{

	struct epoll_event evts[20];
	if(!hIOCPhandler.ok()) return;

	while (!test_canceled())
	{
		int nfds=epoll_wait(hIOCPhandler.get(),evts,1,1000);

		if(nfds<0)
		{
			logger.LogError("ERROR:%s",strerror(errno));
			continue;
		}
		else if(nfds==0)
		{
			continue;
		}

		for(int i=0; i<nfds; i++)
		{
			int nevt=evts[i].events;
			Session* pkey=(Session*)evts[i].data.ptr;

			if(!pkey)
			{
				logger.LogError("ERROR: NO Hander");
				continue;
			}

			Session& ikey(*pkey);

			if((nevt&(EPOLLERR|EPOLLHUP))!=0)
			{
				ikey.Disconnect();
				continue;
			}

			if(ikey.IsError())
			{
				continue;
			}

			ikey.tpLast=accounter.tTimeStamp;
			if(nevt&EPOLLIN)
			{
				HandleRecv(ikey);
			}
			else if(nevt&EPOLLOUT)
			{
				HandleSend(ikey);
			}

			if(ikey.IsError())
			{
				continue;
			}
			else
			{
				ikey.m_nEpCtl.exchange(0);
			}

		}
	}
}

#endif


EW_LEAVE
