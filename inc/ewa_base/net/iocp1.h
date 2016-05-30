
#ifndef __H_EW_NET_IOCP1__
#define __H_EW_NET_IOCP1__

#include "ewa_base/basic/platform.h"
#include "ewa_base/net/session.h"
#include "ewa_base/threading/thread.h"
#include "ewa_base/threading/thread_event.h"
#include "ewa_base/threading/thread_spin.h"
#include "ewa_base/logging/logger.h"
#include "ewa_base/basic/lockguard.h"
#include "ewa_base/scripting/callable_data.h"

EW_ENTER

typedef arr_1t<DataPtrT<Session> > SessionArray;

class DLLIMPEXP_EWA_BASE IocpCommand : public ObjectData
{
public:
	virtual void Handle(SessionArray& akey)=0;
};

class  DLLIMPEXP_EWA_BASE IocpObject : public CallableData
{
public:
	virtual void idle_check(){}

};

class DLLIMPEXP_EWA_BASE IOCPPool : public ThreadEx, private NonCopyable
{
public:
	typedef ThreadEx basetype;

	Logger logger;

	bool activate(int nWorker_=1);

	IOCPPool(const String& name_="iocp_server",int maxconn_=1024*16);
	~IOCPPool();

	void cancel();
	void wait_for_all_session_exit();

	void DisconnectAll();

	KO_Policy_handle::type native_handle()
	{
		return hIOCPhandler;
	}

	int Register(Session* session);
	bool Execute(IocpCommand* cmd);

	const IOCPAccounter& GetAccounter() const
	{
		return accounter;
	}

	void SetName(const String& s)
	{
		m_sName=s;
	}
	const String& GetName() const
	{
		return m_sName;
	}

	void AttachObject(DataPtrT<IocpObject> pobj);
	void DetachObject(DataPtrT<IocpObject> pobj);

protected:

	void ccc_update_info();
	void ccc_handle_sock();
	void ccc_execute_cmd();


#ifdef EW_WINDOWS
	void ccc_handle_iocp(Session* pkey,MyOverLapped* pdat);
#endif

	AutoPtrT<IOCPAccounter> m_pAccounterLast,m_pAccounterTemp;

	IOCPAccounter accounter;

	SessionArray m_aSessions;
	bst_set<DataPtrT<IocpObject> > m_aObjects; 


	LockFreeQueue<int> m_lkfqSessionAvailable;
	LockFreeQueue<DataPtrT<IocpCommand>,LockFreeQueuePolicyObj<DataPtrT<IocpCommand> > > m_lkfqCommand;
	int m_nSessionMax;

	KO_Handle<KO_Policy_handle> hIOCPhandler;

	void svc_del(int n);

	void svc_worker();
	void svc_checker();

	Event m_nCanClose;

	String m_sName;

#ifndef EW_WINDOWS
	void HandleSend(Session& ikey);
	void HandleRecv(Session& ikey);
#endif

};


EW_LEAVE
#endif
