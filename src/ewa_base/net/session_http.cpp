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


SessionHttp::SessionHttp()
{
	phase=0;
	httpstatus=200;
	length=-1;

	lines.push_back(0);
}

void SessionHttp::HandleHeader(StringBuffer<char>& sb2)
{
	StringBuffer<char> sb1;

	sb1<<"HTTP/1.1 "<<httpstatus<<" OK\r\n";
	sb1<<"Cache-Control:no-cache\r\n";

	for(map_type::const_iterator it=cookie.begin();it!=cookie.end();++it)
	{
		sb1<<"Set-Cookie: "<<(*it).first<<"="<<string_escape((*it).second)<<"; path=/; \r\n";		
	}

	if(httpstatus==301)
	{
		sb1<<"Location: " <<props["Location"]<<"\r\n\r\n";
	}
	else if(flags.get(FLAG_RESPONSE_CHUNKED))
	{
		EW_ASSERT(sb2.empty());
		sb1<<"Transfer-Encoding: chunked\r\n\r\n";
	}
	else
	{
		sb1<<"Content-Length:"<<sb2.size()<<"\r\n\r\n";
	}


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
			query[key].reset(value);
		}
	}
}

template<unsigned N>
class lkt_not_colon
{
public:
	static const unsigned value = N!=':' && N!=0;
};

void SessionHttp::_ParseRequestHeaders()
{
	const char* p=sb.data();

	arr_1t<String> head=string_split(p," ");
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

	for(size_t i=2;i<lines.size();i++)
	{

		const char* p1=p+lines[i-1];
		const char* p2=p1;

		ParserBase::skip<lkt_not_colon>(p2);
		if(*p2==0) continue;

		if(p2)
		{
			String key=string_trim(String(p1,p2));
			String value=p2+1;

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

	if(props["Transfer-Encoding"]=="chunked")
	{
		flags.add(FLAG_REQUEST_CHUNKED);
	}

	if(::strstr(props["Content-Type"].c_str(),"multipart/form-data")!=NULL)
	{
		char* p1=(char*)::strstr(props["Content-Type"].c_str(),"boundary=");
		if(!p1)
		{
			System::LogTrace("invalid multipart/form-data");
		}
		else
		{
			boundary="--"+String(p1+strlen("boundary="));
		}
	}
}

void SessionHttp::OnSendCompleted(TempOlapPtr& q)
{
	if(q->flags!=0) return;

	if(!flags.get(FLAG_RESPONSE_CHUNKED))
	{
		Disconnect();
		return;
	}


	static const int CHUNKED_BUFFER_SIZE=1024*8;

	char buf[CHUNKED_BUFFER_SIZE];
	char *p1=buf+16;
	int s1=CHUNKED_BUFFER_SIZE-18;

	int32_t sz=chunked_stream.Read(p1,s1);
	if(sz>0)
	{
		char* p2=p1+sz;

		*--p1='\n';
		*--p1='\r';
		int32_t tmp=sz;
		for(int tmp=sz;tmp>0;tmp=tmp/16)
		{
			int d=tmp%16;
			char ch=d<10?'0'+d:'A'-10+d;
			*--p1=ch;
		}

		if(sz+7<s1)
		{
			flags.del(FLAG_RESPONSE_CHUNKED);
			memcpy(p2,"\r\n0\r\n\r\n",7);
			p2+=7;
		}
		else
		{
			*p2++='\r';
			*p2++='\n';	
		}
		
		AsyncSend(p1,p2-p1);
		return;
	}

	flags.del(FLAG_RESPONSE_CHUNKED);
	AsyncSend("0\r\n\r\n",5);

	if(sz<0)
	{
		System::LogTrace("stream read error");	
	}

}

void SessionHttp::_ParseMultipartFormdata(char* p1,char* p2)
{
	String name;
	String filename;
	String type;

	StringBuffer<char> result;
	lines.resize(1);

	char* pd=p1;
	for(;pd!=p2;pd++)
	{
		if((*pd)!='\n') continue;
		int n=lines.back();

		pd[0]=0;
		if(pd[-1]=='\r') pd[-1]=0;

		int i=pd-p1+1;
		lines.push_back(i);

		if(i-n==2)
		{
			break;
		}

	}

	if(pd==p2)
	{
		System::LogTrace("invalid multipart data");
		return;
	}

	for(size_t j=0;j<lines.size();j++)
	{
		const char* s0=p1+lines[j];
		const char* s1=strstr(s0,":");

		if(s1==NULL) continue;
		String key=String(s0,s1);
		String value=String(s1+1);

		if(key=="Content-Disposition")
		{
			arr_1t<String> kvs=string_split(value,";");
			for(arr_1t<String>::iterator it=kvs.begin();it!=kvs.end();++it)
			{
				int pe=(*it).find('=');
				if(pe<=0) continue;
				String n=string_trim((*it).substr(0,pe));
				String v=(*it).substr(pe+2,(*it).size()-pe-3);

				if(n=="name")
				{
					name=v;
				}
				else if(n=="filename")
				{
					const char* fp=v.c_str();
					for(const char* p=fp;*p;p++)
					{
						if(*p=='/'||*p=='\\') fp=p+1;
					}
					filename=fp;
				}						
			}
		}
		else if(key=="Content-Type")
		{
			type=string_trim(value);
		}				
	}

	pd=p1+lines.back();

	if(name=="")
	{

	}
	else if(type=="")
	{
		query[name].ref<String>().assign(pd,p2);
	}
	else
	{
		result.assign(pd,p2);
		VariantTable& tb(query[name].ref<VariantTable>());;
		tb["data"].reset(result);
		tb["filename"].reset(filename);
		tb["type"].reset(type);
	}

}

void SessionHttp::OnRecvCompleted(TempOlapPtr& q)
{
	size_t s1=sb.size();
	sb.append(q->buffer,q->size);

	if(phase==0)
	{
		size_t s2=sb.size();
		for(size_t i=s1;i<s2;i++)
		{
			if(sb[i]!='\n') continue;

			sb[i]=0;
			if(sb[i-1]=='\r') sb[i-1]=0;
	
			int n=lines.back();
			lines.push_back(++i);
			if(i-n!=2) continue;

			phase=1;
			_ParseRequestHeaders();

			if(method=="GET")
			{
				HandleRequest();
				return;
			}
			else if(method=="POST")
			{
				sb.erase(sb.begin(),sb.begin()+lines.back());
				break;
			}
			else
			{
				System::LogTrace("unknown http method:"+method);
				Disconnect();
				return;
			}
		}	

		if(phase==0)
		{
			AsyncRecv(q);
			return;		
		}	
	}

	if(phase!=1||method=="POST")
	{
		System::LogTrace("invalid state at "__FUNCTION__);
	}

	if(length>0)
	{
		if(sb.size()<(size_t)length)
		{
			AsyncRecv(q);
			return;			 
		}
	}
	else if(flags.get(FLAG_REQUEST_CHUNKED))
	{
		System::LogTrace("chunked at post?");
	}
	else 
	{
		HandleQuery(sb);
	}

	if(!boundary.empty())		
	{
		const char* p1=sb.c_str();
		const char* p2=p1+sb.size();
		intptr_t ln=boundary.size();

		if(memcmp(p1,boundary.c_str(),ln)!=0)
		{
			Disconnect();
			return;
		}
		p1+=ln+2;


		const char* d1=p1;
		const char* d2=p1;

		while(d2<p2)
		{
			if(*d2++!='\n') continue;

			if(d2-d1<=ln || memcmp(d1,boundary.c_str(),ln)!=0)
			{
				d1=d2;
				continue;
			}

			_ParseMultipartFormdata((char*)p1,(char*)d1-2);

			if(d2==p2)
			{
				if(d2-d1!=ln+4||d2[-2]!='\r'||d2[-3]!='-'||d2[-4]!='-')
				{
					System::LogTrace("invalid multipart formdata");
				}
			}
			else
			{
				if(d2-d1!=ln+2||d2[-2]!='\r')
				{
					System::LogTrace("invalid multipart formdata");
				}
			}

			p1=d1=d2;
		}
	}
	else
	{
		HandleQuery(sb);
	}

	HandleRequest();

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

void SessionHttp::HandleFile(StringBuffer<char>& sb2,const String& filepath)
{
	if(chunked_stream.Open(filepath,FileAccess::FLAG_RD))
	{

		int64_t sz=chunked_stream.Size();
		if(sz>1024*4)
		{
			flags.add(FLAG_RESPONSE_CHUNKED);
			return;
		}

		sb2.resize(sz);
		int32_t s1=chunked_stream.Read(sb2.data(),sz);
		if(s1==sz)
		{
			return;
		}
		System::LogTrace("invalid stream read");
		sb2.clear();

	}

	SessionHttp::HandleContent(sb2);

}


class DLLIMPEXP_EWA_BASE SessionHttpRequest : public CallableTableEx
{
public:
	SessionHttpRequest(SessionHttpEwsl& o)
	{
		value["uri"].reset(o.uri);
		value["method"].reset(o.method);
		value["anchor"].reset(o.anchor);
		value["query"].ref<VariantTable>().swap(o.query);
	}
};

class DLLIMPEXP_EWA_BASE SessionHttpResponse : public CallableTableEx
{
public:
	SessionHttpResponse()
	{
		value["buffer"].kptr(new CallableWrapT<StringBuffer<char> >);
		value["status"].reset(200);
		value["type"].reset("");
	}
};


DataPtrT<CallableTableEx> http_server_objects(new CallableTableEx);

void SessionHttpEwsl::HandleContent(StringBuffer<char>& sb2)
{

	String filepath=Target.server_path+uri;

	int i=-1;
	for(const char* p=uri.c_str();*p;p++)
	{
		if(*p=='.') i=p-uri.c_str();
	}

	if(i>0 && uri.substr(i)==".ewsl")
	{
		DataPtrT<SessionHttpRequest> request(new SessionHttpRequest(*this));
		DataPtrT<SessionHttpResponse> response(new SessionHttpResponse);
		Variant session(Target.GetSession(cookie["KSessionId"]));

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
				sb2.swap(response->value["buffer"].ref<StringBuffer<char> >());
				if(sb2.empty())
				{
					String& fp(response->value["download"].ref<String>());
					if(fp!="")
					{
						HandleFile(sb2,fp);
					}
				}
			}
		}
		else
		{
			sb2<<"server_error";		
		}

		return;
	}
	else
	{
		if(!cookie["KSessionId"].empty())
		{
			Target.GetSession(cookie["KSessionId"]);
		}

		HandleFile(sb2,filepath);
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