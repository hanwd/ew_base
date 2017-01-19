#include "ewa_base/net/session_http.h"
#include "ewa_base/scripting/callable_buffer.h"
#include "ewa_base/scripting/executor.h"
#include "ewa_base/util/strlib.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/basic/scanner_helper.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE SessionHttpRequest : public CallableTableEx
{
public:
	SessionHttpRequest(SessionHttp& o)
	{
		value["uri"].reset(o.uri);
		value["method"].reset(o.method);
		value["anchor"].reset(o.anchor);
		value["clientip"].reset(o.sk_local.peer.host());
		value["query"].ref<VariantTable>().swap(o.query);
	}
};

class CallableFunctionResponseRedirect : public CallableFunction
{
public:
	XopInst inst[2];
	CallableFunctionResponseRedirect()
	{
		inst[0].op=XOP_EXIT;
		inst[1].op=XOP_EXIT;
	}

	int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		VariantTable& tb(ewsl.ci1.nbp[StackState1::SBASE_THIS].ref<VariantTable>());
		tb["status"].reset(301);
		tb["location"]=ewsl.ci0.nbx[1];
		ewsl.ci0.nip=inst;
		return 0;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionResponseRedirect, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionResponseRedirect, ObjectInfo);

class DLLIMPEXP_EWA_BASE SessionHttpResponse : public CallableTableEx
{
public:
	SessionHttpResponse()
	{
		value["buffer"].kptr(new CallableWrapT<StringBuffer<char> >);
		value["status"].reset(200);
		value["type"].reset("");
		value["redirect"].reset(CallableFunctionResponseRedirect::sm_info.CreateObject());
	}
};


void SessionHttp_Response_404(SessionHttp& http)
{
	http.httpstatus=404;

	StringBuffer<char>& sb(http.sb);

	sb<<"\r\n";
	sb<<"<HTML><HEAD>";
	sb<<"<meta http-equiv=Content-Type content=\"text/html;charset=utf-8\">";
	sb<<"</HEAD>";
	sb<<"<BODY><PRE>";
	sb<<"404 page not found";
	sb<<"\r\n";
	sb<<"</PRE></BODY></HTML>\r\n";

	return;

}

void SessionHttp_Response_FILE(SessionHttp& http)
{

	if(!http.chunked_stream.open(http.filepath,FLAG_FILE_RD))
	{
		http.httpstatus=404;
		return;
	}

	http.length=http.chunked_stream.sizeg();
	if(http.length<0)
	{
		System::LogError("invalid file size");
		http.Disconnect();
		return;
	}

	//SessionHttp::map_type::iterator it=http.props.find("Range");
	//if(it!=http.props.end())
	//{

	//}

	if(http.length>1024*8)
	{
		http.flags.add(SessionHttp::FLAG_RESPONSE_CHUNKED);
		return;
	}

	http.sb.resize(http.length);
	int s1=http.chunked_stream.recv(http.sb.data(),http.length);
	if(s1==http.length)
	{
		return;
	}

	System::LogTrace("invalid stream read");
	http.sb.clear();

}


void SessionHttp_Response_EWSL(SessionHttp& http)
{
	DataPtrT<SessionHttpRequest> request(new SessionHttpRequest(http));
	DataPtrT<SessionHttpResponse> response(new SessionHttpResponse);
	Variant session(http.Target.GetSession(http.cookie["KSessionId"]));

	Executor ewsl;
	ewsl.push(session);
	ewsl.push(request);
	ewsl.push(response);
	ewsl.push(http.Target.server_objects);

	bool flag=ewsl.execute_file(http.filepath,4);

	http.httpstatus=variant_cast<int>(response->value["status"]);
	if(http.httpstatus==301)
	{
		http.props["Location"]=variant_cast<String>(response->value["location"]);
	}
	else if(flag)
	{
		http.sb.swap(response->value["buffer"].ref<StringBuffer<char> >());
		if(http.sb.empty())
		{
			http.filepath=response->value["download"].ref<String>();

			String filename = response->value["filename"].ref<String>();
			if (!filename.empty())
			{
				const char* p1 = filename.c_str();
				const char* p2 = p1 + ::strlen(p1);
				while (p2 > p1 && p2[-1] != '\\' && p2[-1] != '/') p2--;
				http.extraheader << "Content-Disposition: attachment; filename=\"" << p2 << "\";\r\n";
			}

			SessionHttp_Response_FILE(http);
		}
	}
	else
	{
		http.httpstatus=501;
		http.sb<<"server_error";
	}

}


SessionManager::SessionManager()
{
	handler_map["HTM"].bind(&SessionHttp_Response_FILE,_1);
	handler_map["HTML"].bind(&SessionHttp_Response_FILE,_1);
	handler_map["JS"].bind(&SessionHttp_Response_FILE,_1);
	handler_map["JSON"].bind(&SessionHttp_Response_FILE,_1);
	handler_map["JPG"].bind(&SessionHttp_Response_FILE,_1);
	handler_map["JPEG"].bind(&SessionHttp_Response_FILE,_1);
	handler_map["BMP"].bind(&SessionHttp_Response_FILE,_1);
	handler_map["GIF"].bind(&SessionHttp_Response_FILE,_1);
	handler_map["ICO"].bind(&SessionHttp_Response_FILE,_1);
	handler_map["EXE"].bind(&SessionHttp_Response_FILE,_1);

	handler_map["EWSL"].bind(&SessionHttp_Response_EWSL,_1);
}

void SessionManager::HandleRequest(SessionHttp& http)
{
	if(http.uri=="/") http.uri="/default.ewsl";

	http.filepath=server_path+http.uri;
	String exts=string_to_upper(string_split(http.filepath,".").back());
	handler_maptype::iterator it=handler_map.find(exts);

	if(it!=handler_map.end())
	{
		(*it).second(http);
	}
	else
	{
		http.httpstatus=404;
	}

	if(http.httpstatus==404)
	{
		SessionHttp_Response_404(http);
	}
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

	for(session_maptype::iterator it=session_map.begin();it!=session_map.end();)
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
	session_maptype::iterator it=session_map.find(cookie_id);
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


SessionHttp::SessionHttp(SessionManager& t):Target(t)
{
	phase=0;
	length=-1;
	httpstatus=200;
	lines.push_back(0);
}



void SessionHttp::HandleRequest()
{
	if(phase++!=1)
	{
		System::LogTrace("phase != 1");
	}

	sb.clear();

	length=-1;
	Target.HandleRequest(*this);

	StringBuffer<char> sb1;

	sb1<<"HTTP/1.1 "<<httpstatus<<" OK\r\n";
	sb1<<"Cache-Control:no-cache\r\n";

	for(map_type::const_iterator it=cookie.begin();it!=cookie.end();++it)
	{
		sb1<<"Set-Cookie: "<<(*it).first<<"="<<string_escape((*it).second)<<"; path=/; \r\n";
	}

	if (!extraheader.empty())
	{
		sb1 << extraheader;
	}

	if(httpstatus==301)
	{
		sb1<<"Location: " <<props["Location"]<<"\r\n\r\n";
	}
	else if(length>0)
	{
		sb1<<"Content-Length:"<<length<<"\r\n\r\n";
	}
	else if(flags.get(FLAG_RESPONSE_CHUNKED))
	{
		EW_ASSERT(sb.empty());
		sb1<<"Transfer-Encoding: chunked\r\n\r\n";
	}
	else
	{
		sb1<<"Content-Length:"<<sb.size()<<"\r\n\r\n";
	}


	if(httpstatus!=301)
	{
		sb1<<sb;
	}

	sb1.swap(sb);
	AsyncSend(sb.c_str(),sb.size(),1);

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

		ScannerHelper<const char*>::skip<lkt_not_colon>(p2);
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
			String boundary="--"+String(p1+strlen("boundary="));
			multipart_formdata.reset(new MultiPartFormData(*this,boundary));
		}
	}
}

void SessionHttp::OnSendCompleted(TempOlapPtr& q)
{
	if(q->flags==0) return;

	if(!flags.get(FLAG_RESPONSE_CHUNKED))
	{
		Disconnect();
		return;
	}

	static const int CHUNKED_BUFFER_SIZE=1024*8;
	char buf[CHUNKED_BUFFER_SIZE];

	if(length<0)
	{

		char *p1=buf+16;
		int s1=CHUNKED_BUFFER_SIZE-18;

		int sz=chunked_stream.recv(p1,s1);
		if(sz>0)
		{
			char* p2=p1+sz;

			*--p1='\n';
			*--p1='\r';
			//int32_t tmp=sz;
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
	else
	{
		int sz=chunked_stream.recv(buf,CHUNKED_BUFFER_SIZE);
		if(sz>0)
		{
			if(length>sz)
			{
				length-=sz;
				AsyncSend(buf,sz);
			}
			else
			{
				flags.del(FLAG_RESPONSE_CHUNKED);
				AsyncSend(buf,length);
			}
		}
		else
		{
			System::LogTrace("stream read error");
			Disconnect();
		}
	}


}

MultiPartFormData::MultiPartFormData(SessionHttp& t,const String& s)
	:Target(t)
	,boundary(s)
	,length_max(t.length)
	,length_cur(0)
{
	phase=-1;
	length_tag=boundary.size();
}

const char* MultiPartFormData::_find_boundary(const char* p1,const char* p2)
{

	for(;p1<p2;p1++)
	{
		if(p1[0]!='\n'||p1[-1]!='\r') continue;
		const char* pt;
		if(p1[-2]!='-')
		{
			pt=p1-length_tag-1;
		}
		else if(p1[-3]=='-')
		{
			pt=p1-length_tag-3;
		}
		else
		{
			p1+=length_tag;
			continue;
		}

		if(memcmp(pt,boundary.c_str(),length_tag)==0)
		{
			return pt;
		}
	}

	return NULL;
}

void MultiPartFormData::_handle_phase1_line(const char* p1)
{

	const char* p2=strstr(p1,":");
	if(p2==NULL) return;

	String key=String(p1,p2);
	String value=String(p2+1);

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
				filename=IConv::from_unknown(fp);
			}
		}
	}
	else if(key=="Content-Type")
	{
		type=string_trim(value);
	}
}


void MultiPartFormData::_handle_phase1()
{

	char* ptmp=pcur;
	for(;ptmp<pend;)
	{
		if(*ptmp++!='\n') continue;
		if(ptmp-pcur==2)
		{
			pcur=ptmp;
			if(filename=="")
			{
				if(pcur==pend && length_cur==length_max)
				{
					phase=4;
				}
				else
				{
					phase=2;
				}
				return;
			}

			String tempfile=String::Format("httpd_temp/%lld.%s",(Clock::now().val/1000ll),filename);
			if(file.open(tempfile,FLAG_FILE_WC))
			{
				phase=3;
				file.truncate(0);
				VariantTable& tb(Target.query[name].ref<VariantTable>());
				tb["type"].reset(type);
				tb["filename"].reset(filename);
				tb["tempfile"].reset(tempfile);
			}
			else
			{
				System::LogTrace("cannot open tempfile:%s",tempfile);
				phase=5;
			}
			return;
		}
		else
		{
			ptmp[-2]=0;
			_handle_phase1_line(pcur);
			pcur=ptmp;
		}
	}

	if(pend-pcur>1024*4)
	{
		System::LogTrace("line too long");
		phase=5;
		return;
	}
	else
	{
		phase=9;
	}

}

void MultiPartFormData::_handle_phase2()
{
	//StringBuffer<char>& sb(Target.sb);

	const char* pt=_find_boundary(pcur+length_tag,pend);
	if(pt==NULL)
	{
		phase+=8;
		return;
	}

	//int sz=pt-2-pcur;

	Target.query[name].ref<String>().assign(pcur,pt-2);

	pcur=(char*)pt+2;

	//if(pend-pcur==2&&pcur[0]=='\r'&&pcur[1]=='\n')
	//{
	//	phase=4;
	//	return;
	//}

	phase=0;

}

void MultiPartFormData::_handle_phase3()
{
	//StringBuffer<char>& sb(Target.sb);

	const char* pt=_find_boundary(pcur+length_tag,pend);
	if(pt!=NULL)
	{
		int sz=pt-2-pcur;
		file.write(pcur,sz);
		pcur=(char*)pt+2;
		file.close();
		phase=0;

		return;
	}

	int sz=(pend-pcur-length_tag)&~4095;
	if(sz>0)
	{
		file.write(pcur,sz);
		pcur+=sz;
	}
	phase+=8;

}

void MultiPartFormData::HandleData(TempOlapPtr& q)
{

	StringBuffer<char>& sb(Target.sb);

	if(phase<0)
	{
		phase=1;
		length_cur=sb.size();
	}
	else
	{
		length_cur+=q->size;
	}

	pbeg=sb.data();
	pend=sb.data()+sb.size();
	pcur=pbeg;


	while(1)
	{
		switch(phase)
		{
		case 0:
			if(length_cur==length_max && pend-pcur==2&&pcur[0]=='\r'&&pcur[1]=='\n')
			{
				phase=4;
			}
			type="";
			name="";
			filename="";
		case 1:
			_handle_phase1();
			break;
		case 2:
			_handle_phase2();
			break;
		case 3:
			_handle_phase3();
			break;
		case 4:
			Target.HandleRequest();
			return;
		case 9:
		case 10:
		case 11:

			if(length_cur==length_max)
			{
				Target.HandleRequest();
				if(pend-pcur>2)
				{
					System::LogTrace("unepected data end");
				}
				return;
			}

			sb.erase(sb.begin(),sb.begin()+(pcur-pbeg));
			Target.AsyncRecv(q);
			phase-=8;
			return;
		default:
			Target.Disconnect();
			return;
		}
	}

}


void SessionHttp::_HandleData(TempOlapPtr& q,size_t s1)
{

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

	if(phase!=1||method!="POST")
	{
		System::LogTrace("invalid state");
		Disconnect();
		return;
	}

	if(multipart_formdata)
	{
		multipart_formdata->HandleData(q);
		return;
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

	HandleQuery(sb);
	HandleRequest();
}

void SessionHttp::OnRecvCompleted(TempOlapPtr& q)
{
	size_t s1=sb.size();
	sb.append(q->buffer,q->size);
	_HandleData(q,s1);
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


void SessionHttpServer::NewSession(PerIO_socket& sk)
{
	sk.sock.block(false);
	DataPtrT<SessionHttp> kjobd_worker(new SessionHttp(*Target));
	kjobd_worker->sk_local.swap(sk);
	StartSession(kjobd_worker.get(),hiocp);
}


class CallableFunctionAbondonSession : public CallableFunction
{
public:

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		SessionData* p=dynamic_cast<SessionData*>(ewsl.ci0.nbx[1].kptr());
		if(p) p->abondon();
		ewsl.ci0.nbx[1].reset(p!=NULL);
		return 1;
	}
};

SessionHttpServer::SessionHttpServer()
{
	Target.reset(new SessionManager);
	Target->server_objects.reset(new CallableTableEx);
	Target->server_objects->value["abondon_session"].reset(new CallableFunctionAbondonSession);
}




void SessionHttpServer::Register(const String& name,Variant& object)
{
	Target->server_objects->value[name]=object;
}

EW_LEAVE
