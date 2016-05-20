#ifndef __H_EW_IOCP_SESSION_HTTP__
#define __H_EW_IOCP_SESSION_HTTP__

#include "ewa_base/net/session.h"
#include "ewa_base/net/iocp1.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE SessionManager;
class DLLIMPEXP_EWA_BASE SessionHttp;

class DLLIMPEXP_EWA_BASE SessionData : public CallableTableEx
{
public:
	TimePoint tplast;
	String id;
	AtomicSpin spin;
	BitFlags flags;

	enum
	{
		FLAG_ABONDON=1<<0,
	};

	virtual CallableData* DoClone(ObjectCloneState& cs)
	{
		if(cs.type==0) return this;
		return new SessionData(*this);
	}

	bool is_timeout(const SessionManager& sm);
	void abondon(){flags.add(FLAG_ABONDON);}
	void touch(){tplast=Clock::now();}

};


class DLLIMPEXP_EWA_BASE SessionManager : public Object
{
public:
	typedef DataPtrT<SessionData> session_ptr;
	typedef bst_map<String,session_ptr> map_type;

	session_ptr GetSession(String& session_id);

	TimePoint tp_now;
	String server_path;

protected:
	SessionData* CreateSessionData();

	void EraseTimeoutSessions();
	virtual String NewSessionId();

	map_type session_map;
	AtomicSpin spin;
};




class DLLIMPEXP_EWA_BASE SessionHttp : public SessionClient
{
public:

	StringBuffer<char> sb;
	String uri;
	String method;
	String anchor;

	arr_1t<String> lines;
	int phase;
	int flags;

	typedef indexer_map<String,String> map_type;

	map_type props;
	map_type query;
	map_type cookie;

	int httpstatus;

	SessionHttp();

	virtual void HandleHeader(StringBuffer<char>& sb1);
	virtual void HandleContent(StringBuffer<char>& sb2);

	void HandleQuery(const String& s);

	void HandleRequest();

	void OnSendCompleted(TempOlapPtr& q);

	virtual void OnRecvCompleted(TempOlapPtr& q);

	void OnConnected();

	void Redirect(const String& url);

protected:
	void HandleLines();
};



class DLLIMPEXP_EWA_BASE SessionHttpEwsl : public SessionHttp
{
public:

	SessionManager& Target;
	SessionHttpEwsl(SessionManager& t);

	virtual void HandleContent(StringBuffer<char>& sb);


};

class  DLLIMPEXP_EWA_BASE SessionHttpServer : public SessionServer
{
public:
	SessionManager Target;
	void NewSession(PerIO_socket& sk);


	static void Register(const String& name,Variant& object);
};

EW_LEAVE
#endif
