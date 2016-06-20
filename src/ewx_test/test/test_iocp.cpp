

#include "ewa_base.h"
#include "ewa_base/net/iocp1.h"

#define N_TCP_CLIENT 1024*2
#define N_UDP_CLIENT 1024*2

using namespace ew;

class SessionTCPEcho : public SessionClient
{
public:

	SessionTCPEcho() {}

	Q2Packet<IPacketEx> q2packet;

	virtual void OnRecvCompleted(TempOlapPtr& q)
	{
		//AsyncSend(q);
		//AsyncRecv();

		if(!q2packet.update(q))
		{
			Disconnect();
			return;
		}

		size_t n=q2packet.size();
		for(size_t i=0; i<n; i++)
		{
			OnPacket(q2packet[i]);
		}

		AsyncRecv(q);
	}

	virtual void OnPacket(IPacketEx& pk)
	{
		EW_ASSERT(pk.check());
		pk.update();

		AsyncSend(pk);
	}

	void OnConnected()
	{
		AsyncRecv();
	}


};

class SessionServerEcho : public SessionServer
{
public:
	SessionServerEcho() {}

	void NewSession(PerIO_socket& sk)
	{
		sk.sock.block(false);
		DataPtrT<SessionTCPEcho> echo(new SessionTCPEcho);
		echo->sk_local.swap(sk);
		StartSession(echo.get(),hiocp);
	}

};


class SessionClientEcho : public SessionClient
{
public:

	Q2Packet<IPacketEx> q2packet;
	IPacketEx packet;

	Logger logger;

	uint32_t seq_send;
	uint32_t seq_recv;

	void StartSend()
	{
		SendPacket();
		AsyncRecv();
	}

	void SendPacket()
	{
		packet.size=1024;
		packet.tag1=seq_send;
		packet.update();

		seq_send++;
		AsyncSend(packet);
	}

	void OnConnected()
	{
		q2packet.reset();
		seq_send=0;
		seq_recv=0;
	}

	virtual void OnRecvCompleted(TempOlapPtr& q)
	{

		if(!q2packet.update(q))
		{
			Disconnect();
			return;
		}

		size_t n=q2packet.size();
		for(size_t i=0; i<n; i++)
		{
			OnPacket(q2packet[i]);
		}

		AsyncRecv(q);

	}


	void OnPacket(IPacketEx& pk)
	{
		EW_ASSERT(pk.tag1==seq_recv);

		seq_recv++;
		SendPacket();
	}

};


class IocpCommandStartSend : public IocpCommand
{
public:
	void Handle(SessionArray& akey)
	{
		for(size_t i=0; i<akey.size(); i++)
		{
			SessionClientEcho* pEcho=dynamic_cast<SessionClientEcho*>(akey[i].get());
			if(!pEcho) continue;
			pEcho->StartSend();
		}
	}
};


TEST_DEFINE(TEST_IOCP_TCP)
{

	IOCPPool hiocp_client("clt",1024*6);
	IOCPPool hiocp_server("svr",1024*6);

	hiocp_server.activate(1);
	hiocp_client.activate(1);

	Logger logger;
	logger.LogMessage("testing tcp ----------------");

	DataPtrT<SessionServerEcho> pEchoServer(new SessionServerEcho);

	{

		if(pEchoServer->Listen("127.0.0.1",10241))
		{
			logger.LogMessage("echo server ready");
		}
		else
		{
			logger.LogError("echo server failed");
			return;
		}

		hiocp_server.Register(pEchoServer.get());
	}


	TimePoint tk1=Clock::now();
	int nCount=0;


	for(int i=0; i<N_TCP_CLIENT; i++)
	{
		DataPtrT<SessionClientEcho> pEcho1(new SessionClientEcho);
		bool f1=pEcho1->Connect("127.0.0.1",10241);
		if(f1)
		{

			if(nCount%1000==0 && nCount>0)
			{
				Console::WriteLine(String::Format("%d connections",nCount));
			}
			int ret=hiocp_client.Register(pEcho1.get());
			if(ret<0)
			{
				break;
			}

			nCount++;
		}
		else
		{
			Console::WriteLine("connection failed");
			nCount=nCount;
			break;

		}
	}
	TimePoint tk2=Clock::now();
	logger.LogMessage("%g seconds, %d connections",(tk2-tk1)/TimeSpan::Seconds(1),nCount);


	//hiocp_client.logger.reset(NULL);
	hiocp_client.Execute(new IocpCommandStartSend);


	for(int i=0; i<8; i++)
	{
		Thread::sleep_for(1000);
	}

	logger.LogMessage("disconnecting all sessions");

	{
		hiocp_server.DisconnectAll();
		hiocp_client.DisconnectAll();
	}

	logger.LogMessage("wait_session_exit");
	hiocp_server.wait_for_all_session_exit();
	hiocp_client.wait_for_all_session_exit();
	logger.LogMessage("wait_session_done");


}


class SessionUDPEchoClt : public SessionUDP
{
public:

	IPacketEx packet;

	SessionUDPEchoClt() {}

	virtual void OnSendCompleted(TempOlapPtr& q)
	{
		AsyncRecv(q);
	}

	virtual void OnRecvCompleted(TempOlapPtr& q)
	{
		AsyncSend(q);
	}

	void OnConnected()
	{
		packet.size=1024;
		AsyncSend((char*)&packet,packet.size);
	}
};


class SessionUDPEchoSvr : public SessionUDP
{
public:

	SessionUDPEchoSvr() {}

	virtual void OnSendCompleted(TempOlapPtr& q)
	{
		AsyncRecv(q);
	}

	virtual void OnRecvCompleted(TempOlapPtr& q)
	{
		AsyncSend(q);
	}

	void OnConnected()
	{
		AsyncRecv();
	}

};


TEST_DEFINE(TEST_IOCP_UDP)
{

	IOCPPool hiocp_client("clt",1024*6);
	IOCPPool hiocp_server("svr",1024*6);

	hiocp_client.activate(1);
	hiocp_server.activate(1);

	Logger logger;
	logger.LogMessage("testing udp ----------------");

	DataPtrT<SessionUDPEchoSvr> udp_svr(new SessionUDPEchoSvr);

	IPAddress svr_addr("127.0.0.1",10211);
	udp_svr->Bind(svr_addr);
	hiocp_server.Register(udp_svr.get());

	for(int i=0; i<N_UDP_CLIENT; i++)
	{
		DataPtrT<SessionUDPEchoClt> udp_clt(new SessionUDPEchoClt);
		udp_clt->Connect("127.0.0.1",10211);
		hiocp_client.Register(udp_clt.get());
	}

	for(int i=0; i<8; i++)
	{
		Thread::sleep_for(1000);
	}

	logger.LogMessage("disconnecting all sessions");

	{
		hiocp_server.DisconnectAll();
		hiocp_client.DisconnectAll();
	}

	logger.LogMessage("wait_session_exit");
	hiocp_server.wait_for_all_session_exit();
	hiocp_client.wait_for_all_session_exit();
	logger.LogMessage("wait_session_done");


}

