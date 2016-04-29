#ifndef __H_EW_NET_HTTP__
#define __H_EW_NET_HTTP__


#include "ewa_base/basic/object.h"
#include "ewa_base/basic/pointer.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/net/socket.h"


EW_ENTER

class DLLIMPEXP_EWA_BASE HttpBase : public ObjectData
{
public:

	int type;
	String head;

	class Cookie
	{
	public:
		String name;
		String value;
		String expire;
		String path;
		String domain;
	};

	indexer_map<String,String> prop;
	indexer_map<String,Cookie> cookie;

	StringBuffer<char> data;

	HttpBase();

	virtual void clear();

	virtual bool parse(const StringBuffer<char>& sb);
};


class DLLIMPEXP_EWA_BASE HttpResponse : public HttpBase
{
public:
	HttpResponse();
	bool parse(const StringBuffer<char>& sb);
};


class DLLIMPEXP_EWA_BASE HttpRequest : public HttpBase
{
public:
	enum
	{
		HTTP_GET,
		HTTP_POST,
	};

	int port;
	String uri;
	HttpRequest();

	void clear();

	indexer_map<String, String> postdata;

	DataPtrT<HttpResponse> Execute();
};

EW_LEAVE
#endif
