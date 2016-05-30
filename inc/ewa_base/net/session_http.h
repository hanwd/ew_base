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

class DLLIMPEXP_EWA_BASE MultiPartFormData : public ObjectData
{
public:

	MultiPartFormData(SessionHttp& t,const String& s);

	void HandleData(TempOlapPtr& q);

	SessionHttp& Target;

	String name;
	String type;
	String filename;
	String boundary;

	int length_max;
	int length_cur;
	int length_tag;

	int phase;

	File file;

	const char* _find_boundary(const char* p1,const char* p2);

	void _handle_phase1_line(const char* p1);

	void _handle_phase1();
	void _handle_phase2();
	void _handle_phase3();


	typedef char* mychar_ptr;
	mychar_ptr pbeg;
	mychar_ptr pend;
	mychar_ptr pcur;
};


class DLLIMPEXP_EWA_BASE SessionHttp : public SessionClient
{
public:

	enum
	{
		FLAG_REQUEST_CHUNKED	=1<<0,
		FLAG_RESPONSE_CHUNKED	=1<<1,
	};

	StringBuffer<char> sb;

	String uri;
	String method;
	String anchor;
	int length;

	int phase;
	BitFlags flags;

	//String boundary;
	DataPtrT<MultiPartFormData> multipart_formdata;

	typedef indexer_map<String,String> map_type;

	map_type props;
	map_type cookie;

	VariantTable query;

	int httpstatus;

	SessionHttp();

	Stream chunked_stream;

	virtual void HandleHeader(StringBuffer<char>& sb1);
	virtual void HandleContent(StringBuffer<char>& sb2);

	void HandleQuery(const String& s);
	void HandleRequest();

	void HandleFile(StringBuffer<char>& sb2,const String& filepath);

	virtual void OnSendCompleted(TempOlapPtr& q);
	virtual void OnRecvCompleted(TempOlapPtr& q);

	void OnConnected();

	void Redirect(const String& url);

protected:

	void _ParseRequestHeaders();

	arr_1t<int> lines;
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
