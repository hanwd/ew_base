#ifndef __H_EW_IOCP_SESSION__
#define __H_EW_IOCP_SESSION__


#include "ewa_base/net/overlapped.h"
#include "ewa_base/net/q2packet.h"
#include "ewa_base/threading/thread.h"

#if defined(EW_WINDOWS)
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#else
#include <sys/epoll.h>
#endif


EW_ENTER


class DLLIMPEXP_EWA_BASE IOCPPool;

class DLLIMPEXP_EWA_BASE Session : public ObjectData, public ISessionBase
{
public:
	friend class IOCPPool;

	Session();
	virtual ~Session();

	// Call when AsyncSend/AsyncRecv is finished
	virtual void OnSendCompleted(TempOlapPtr& q);
	virtual void OnRecvCompleted(TempOlapPtr& q);

	virtual void OnSendReady();
	virtual void OnRecvReady();

	virtual void OnConnected();
	virtual void OnDisconnected();

	bool IsConnected();
	bool IsError();

	virtual void Disconnect();

	virtual bool TestTimeout(TimePoint& tp,TempOlapPtr& q);


	typedef LockFreeQueue<TempOlapPtr> LKFQueue;


	class LKFQueue2 : public LKFQueue
	{
	public:
		LKFQueue2(int sz, int f) :LKFQueue(sz, f){}
		TempOlapPtr getq()
		{
			TempOlapPtr q = LKFQueue::getq();
			if (!q)
			{
				q.reset(new MyOverLappedEx);
			}
			return q;
		}
	};

	static LKFQueue2 lkfq_free;

protected:

	bool HasPending();



	LKFQueue lkfq_send;
	LKFQueue lkfq_recv;

	AtomicInt32 m_nPendingSend;
	AtomicInt32 m_nPendingRecv;


	LitePtrT<IOCPPool> hiocp;


#ifndef EW_WINDOWS
	void ep_ctl(int f);
	int m_nLastEpoll_ctl;
	int m_nTempEpoll_ctl;
	BitFlags m_nLastEpoll_def;
	AtomicInt32 m_nEpCtl;
	TempOlapPtr tmp_send;

#endif

};


class DLLIMPEXP_EWA_BASE SessionTCP : public Session
{
public:

	friend class IOCPPool;

	SessionTCP();

	bool WaitForSend();
	bool WaitForRecv();

	virtual bool AsyncSend(const char* data,size_t size,int flag=1);

	virtual bool AsyncSend(TempOlapPtr& q);

	bool AsyncSend(IPacketEx& packet){return AsyncSend((char*)&packet,packet.size);}

	bool AsyncRecv(TempOlapPtr& q);
	bool AsyncRecv();

protected:
	bool DoAsyncSend(TempOlapPtr& q);
	bool DoAsyncRecv(TempOlapPtr& q);

	void DoAsyncSend();
	void DoAsyncRecv();

};


class DLLIMPEXP_EWA_BASE SessionUDP : public Session
{
public:

	friend class IOCPPool;

	SessionUDP();
	~SessionUDP();

	typedef Session basetype;


	bool Bind(IPAddress& addr);
	bool Bind(const String& ip,int port);
	bool Connect(const String& ip,int port);


	bool AsyncSend(const char* data,size_t size,IPAddress& addr);
	bool AsyncSend(const char* data,size_t size);
	bool AsyncSend(TempOlapPtr& q);

	bool AsyncRecv(TempOlapPtr& q);
	bool AsyncRecv();

protected:
	bool DoAsyncSend(TempOlapPtr& q);
	bool DoAsyncRecv(TempOlapPtr& q);

	void DoAsyncSend();
	void DoAsyncRecv();

protected:

	void OnConnected();
	void OnDisconnected();


};

class DLLIMPEXP_EWA_BASE SessionClient : public SessionTCP
{
public:
	typedef SessionTCP basetype;

	SessionClient();
	~SessionClient();

	bool Connect(const String& ip,int port);

protected:
	void OnConnected();
	void OnDisconnected();
};


class DLLIMPEXP_EWA_BASE SessionServer : public SessionTCP
{
public:
	typedef SessionTCP basetype;

#ifdef _MSC_VER
	LPFN_ACCEPTEX lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS lpfnGetAddrEx;
#endif

	SessionServer();

	bool Listen(const String& ip,int port);
	void Disconnect();
	void Close();

	bool WaitForAccept();

	virtual void NewSession(PerIO_socket& sk);

protected:

	void OnConnected();
	void OnDisconnected();

	virtual void OnRecvReady();

	void StartSession(Session*,IOCPPool* h=NULL);

	char tmp_buffer[1024];

	PerIO_socket sk_remote;
};



class DLLIMPEXP_EWA_BASE SessionEcho : public SessionTCP
{
public:
	typedef SessionTCP basetype;

	SessionEcho();
	~SessionEcho();

private:

	virtual void OnConnected();
	virtual void OnDisconnected();

	virtual void OnSendReady();
	virtual void OnRecvReady();

	virtual void OnSendCompleted(TempOlapPtr& q);
	virtual void OnRecvCompleted(TempOlapPtr& q);

	arr_1t<char> mybuf;
	int mypos;
	int mylen;
};



EW_LEAVE
#endif
