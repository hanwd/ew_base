#ifndef __H_EW_IOCP_SESSION_HTTP__
#define __H_EW_IOCP_SESSION_HTTP__

#include "ewa_base/net/session.h"
#include "ewa_base/net/iocp1.h"

EW_ENTER


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
	map_type postd;	
	map_type query;
	map_type cookie;

	int httpstatus;


	SessionHttp();

	virtual void HandleHeader(StringBuffer<char>& sb1,int sz);
	virtual void HandleContent(StringBuffer<char>& sb2);

	void HandleRequest();

	void OnSendCompleted(TempOlapPtr& q);

	virtual void OnRecvCompleted(TempOlapPtr& q);

	void OnConnected();

protected:


	void HandleLines();
};


EW_LEAVE
#endif
