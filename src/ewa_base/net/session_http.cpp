#include "ewa_base/net/session_http.h"

EW_ENTER


SessionHttpEwsl::SessionHttpEwsl(SessionManager& t):Target(t)
{

}

SessionData* SessionManager::CreateSessionData()
{
	return new SessionData;
}

bool SessionData::is_timeout(const SessionManager& sm)
{
	return flags.get(FLAG_ABONDON)||(sm.tp_now-tplast).GetSeconds()>180;
}

void SessionManager::EraseTimeoutSessions()
{
	TimePoint tp=Clock::now();
	if((tp-tp_now).GetSeconds()<30) return;
	tp_now=tp;

	for(map_type::iterator it=session_map.begin();it!=session_map.end();)
	{
		if((*it).second->is_timeout(*this))
		{
			it=session_map.erase(it);
		}
		else
		{
			it++;
		}
	}	
}

SessionManager::session_ptr SessionManager::GetSession(String& cookie_id)
{
	LockGuard<AtomicSpin> lock(spin);
	EraseTimeoutSessions();
	session_ptr item;
	map_type::iterator it=session_map.find(cookie_id);
	if(it==session_map.end()||(*it).second->flags.get(SessionData::FLAG_ABONDON))
	{
		cookie_id=NewSessionId();
		session_ptr &item(session_map[cookie_id]);
		item.reset(CreateSessionData());
		item->id=cookie_id;
		return item;
	}
	else
	{
		session_ptr &item(session_map[cookie_id]);
		item->touch();
		return item;
	}


}

String SessionManager::NewSessionId()
{
	String id;
	int64_t val=Clock::now().val/1000;
	static uint32_t pwd=val;
	pwd=(pwd*0x12345789+0x5131)^(pwd>>8);

	id<<String::Format("%08d",pwd%100000000u);
	id<<val;

	return id;
}


SessionHttp::SessionHttp(){phase=0;httpstatus=200;length=-1;}

void SessionHttp::HandleHeader(StringBuffer<char>& sb2)
{
	StringBuffer<char> sb1;

	sb1<<"HTTP/1.1 "<<httpstatus<<" OK\r\n";
	sb1<<"Cache-Control:no-cache\r\n";

	for(map_type::const_iterator it=cookie.begin();it!=cookie.end();++it)
	{
		sb1<<"Set-Cookie: "<<(*it).first<<"="<<string_escape((*it).second)<<"; path=/; domain=127.0.0.1\r\n";		
	}
	if(httpstatus==301)
	{
		sb1<<"Location: " <<props["Location"]<<"\r\n";
	}
	else if(sb2.size()>0)
	{
		sb1<<"Content-Length:"<<sb2.size()<<"\r\n";
	}

	sb1<<"\r\n";

	if(httpstatus!=301)
	{
		sb1<<sb2;
	}

	sb1.swap(sb2);


}

void SessionHttp::HandleContent(StringBuffer<char>& sb2)
{
	httpstatus=404;

	sb2<<"\r\n";	
	sb2<<"<HTML><HEAD>";
	sb2<<"<meta http-equiv=Content-Type content=\"text/html;charset=utf-8\">";
	sb2<<"</HEAD>";
	sb2<<"<BODY><PRE>";
	sb2<<"400 page not found";
	sb2<<"\r\n";
	sb2<<"</PRE></BODY></HTML>\r\n";

}

void SessionHttp::HandleRequest()
{
	if(phase++!=1)
	{
		System::LogTrace("phase != 1");
	}

	StringBuffer<char> sb1;

	HandleContent(sb1);
	HandleHeader(sb1);

	AsyncSend(sb1.c_str(),sb1.size());	

}

void SessionHttp::HandleQuery(const String& s)
{
	arr_1t<String> q2=string_split(s,"&");
	for(size_t i=0;i<q2.size();i++)
	{
		int q1=q2[i].find('=');
		if(q1>0)
		{
			String key=string_unescape(q2[i].substr(0,q1));
			String value=string_unescape(q2[i].substr(q1+1));
			query[key]=value;
		}
	}
}

void SessionHttp::HandleLines()
{
	arr_1t<String> head=string_split(lines[0]," ");
	if(head.size()>1)
	{
		method=head[0];
		String& temp_uri(head[1]);

		int p1=temp_uri.find('#');

		if(p1>=0)
		{
			anchor=string_unescape(temp_uri.substr(p1+1));
			temp_uri=temp_uri.substr(0,p1);
		}

		int p2=temp_uri.find('?');
		if(p2>=0)
		{
			HandleQuery(temp_uri.substr(p2+1));
			temp_uri=temp_uri.substr(0,p2);
		}

		uri=string_unescape(temp_uri);

	}

	for(size_t i=1;i<lines.size();i++)
	{
		int n=lines[i].find(':');
		if(n>0)
		{
			String key=string_trim(lines[i].substr(0,n));
			String value=lines[i].substr(n+1);

			if(key=="Cookie")
			{		
				for(int p0=0;p0>=0;)
				{
					int p1=value.find('=',p0);
					if(p1<0) break;
					int p2=value.find(';',p1);

					String ck_key=string_trim(value.substr(p0,p1-p0));
					String ck_value=string_trim(value.substr(p1+1,p2-p1-1));
					cookie[ck_key]=string_unescape(ck_value);

					if(p2<0) break;
					p0=p2+1;
				}
			}
			else if(key=="Content-Length")
			{
				value.ToNumber(&length);
			}
			else
			{
				props[key]=value;
			}
		}
	}

	flags=props["Transfer-Encoding"]=="chunked"?1:0;

}

void SessionHttp::OnSendCompleted(TempOlapPtr& q)
{
	if(q->flags==0)
	{
		Disconnect();
	}
}




void SessionHttp::HandleMulitpart(const char* p1,const char* p3)
{
	const char* L1=::strstr(p1,"\n");
	if(!L1||L1>p3)
	{
		System::LogDebug("invalid multipart");
		return;
	}
	p1=L1+1;
	while(1)
	{
		const char* p2=p1;
		ParserBase::skip<lkt_not_newline>(p2);
		if(p2-p1==2)
		{

		}
	}
	

}

void SessionHttp::OnRecvCompleted(TempOlapPtr& q)
{
	size_t s0=sb.size();
	sb.append(q->buffer,q->size);

	size_t p1=0;

	if(phase==0)
	{
		for(size_t p2=1;p2<sb.size();p2++)
		{
			if(sb[p2]=='\n')
			{
				size_t p3=sb[p2-1]=='\r'?p2-1:p2;
				lines.append(String(&sb[p1],p3-p1));
				if(p3==p1)
				{
					p1=p2+1;
					phase=1;
					break;
				}
				p1=p2+1;
			}
		}

		if(p1!=0)
		{
			size_t s2=sb.size()-p1;
			memmove(sb.data(),sb.data()+p1,s2);
			sb.resize(s2);
		}

		if(phase==1)
		{
			HandleLines();

			if(method!="POST")
			{
				HandleRequest();
				return;
			}
		}
		else
		{
			AsyncRecv(q);
			return;
		}

	}

	EW_ASSERT(method=="POST");

	if(phase==1)
	{

		if(length>0)
		{
			 if(sb.size()!=length)
			 {
				AsyncRecv(q);
				return;			 
			 }

			if(::strstr(props["Content-Type"].c_str(),"multipart/form-data")!=NULL)
			{

				const char* boundary=::strstr(props["Content-Type"].c_str(),"boundary=");
				if(!boundary)
				{
					System::LogTrace("invalid multipart/form-data");
				}
				else
				{
					boundary+=::strlen("boundary=");
					const char* p1=sb.c_str();
					while(p1=::strstr(p1,boundary))
					{
						const char* p2=::strstr(p1+1,boundary);
						if(p2)
						{
							HandleMulitpart(p1,p2);
							p1=p2+1;
						}
						else
						{

							System::LogTrace("invalid multipart/form-data boundary");
							break;
						}
					}
				}								

			}
			HandleRequest();

			return;
		}

		if(flags==1)
		{
			
		}
		else 
		{

			HandleQuery(sb);
		}
		HandleRequest();	

	}


}

void SessionHttp::OnConnected()
{
	AsyncRecv();
}


void SessionHttp::Redirect(const String& url)
{
	httpstatus=301;
	props["Location"]=url;	
}



class DLLIMPEXP_EWA_BASE SessionHttpRequest : public CallableTableEx
{
public:
	SessionHttpRequest(SessionHttpEwsl& o)
	{
		value["uri"].reset(o.uri);
		value["method"].reset(o.method);
		value["anchor"].reset(o.anchor);
		VariantTable& query(value["query"].ref<VariantTable>());

		for(SessionHttpEwsl::map_type::iterator it=o.query.begin();it!=o.query.end();++it)
		{
			query[(*it).first].reset((*it).second);
		}
	}
};

class DLLIMPEXP_EWA_BASE SessionHttpResponse : public CallableTableEx
{
public:
	SessionHttpResponse()
	{
		value["buffer"].kptr(new CallableWrapT<StringBuffer<char> >);
		value["status"].reset(200);
	}
};


DataPtrT<CallableTableEx> http_server_objects(new CallableTableEx);

void SessionHttpEwsl::HandleContent(StringBuffer<char>& sb)
{

	String filepath=Target.server_path+uri;

	int i=-1;
	for(const char* p=uri.c_str();*p;p++)
	{
		if(*p=='.') i=p-uri.c_str();
	}

	if(i>0 && uri.substr(i)==".ewsl")
	{
		Variant request(new SessionHttpRequest(*this));
		Variant session(Target.GetSession(cookie["KSessionId"]));
		DataPtrT<SessionHttpResponse> response(new SessionHttpResponse);


		Executor ewsl;
		ewsl.push(session);
		ewsl.push(request);
		ewsl.push(response);
		ewsl.push(http_server_objects);

		if(ewsl.execute_file(filepath,4))
		{
			httpstatus=variant_cast<int>(response->value["status"]);
			if(httpstatus==301)
			{
				props["Location"]=variant_cast<String>(response->value["location"]);
			}
			else
			{
				sb.swap(response->value["buffer"].ref<StringBuffer<char> >());
			}
		}
		else
		{
			sb<<"server_error";		
		}
	}
	else if(!sb.load(filepath,FILE_BINARY))
	{
		SessionHttp::HandleContent(sb);
	}
}


void SessionHttpServer::NewSession(PerIO_socket& sk)
{
	sk.sock.Block(false);
	DataPtrT<SessionHttpEwsl> kjobd_worker(new SessionHttpEwsl(Target));
	kjobd_worker->sk_local.swap(sk);
	StartSession(kjobd_worker.get(),hiocp);
}



void SessionHttpServer::Register(const String& name,Variant& object)
{
	http_server_objects->value[name]=object;
}

EW_LEAVE