#include "ewa_base/net/session_http.h"

EW_ENTER


SessionHttp::SessionHttp(){phase=0;httpstatus=200;}

void SessionHttp::HandleHeader(StringBuffer<char>& sb1,int sz)
{
	sb1<<"HTTP/1.1 "<<httpstatus<<" OK\r\n";
	sb1<<"Cache-Control:no-cache\r\n";
	if(sz>0) sb1<<"Content-Length:"<<sz<<"\r\n";
	for(map_type::const_iterator it=cookie.begin();it!=cookie.end();++it)
	{
		sb1<<"Set-Cookie: "<<(*it).first<<"="<<string_escape((*it).second)<<"; path=/; domain=127.0.0.1\r\n";		
	}
	sb1<<"\r\n";	
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

	StringBuffer<char> sb1,sb2;

	HandleContent(sb2);

	if(httpstatus==301)
	{
		sb1<<"HTTP/1.1 301 Redirect\r\n";
		sb1<<"Location: " <<props["Location"]<<"\r\n";
		sb1<<"\r\n";
		sb1<<"\r\n";
	}
	else
	{
		IConv::ansi_to_utf8(sb2,sb2.data(),sb2.size());
		HandleHeader(sb1,sb2.size());
		sb1<<sb2;
	}

	AsyncSend(sb1.c_str(),sb1.size());	

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
			arr_1t<String> q2=string_split(temp_uri.substr(p2+1),"&");
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
		if(flags==1)
		{
			
		}
		else
		{
			
		}

		HandleRequest();
	}


}

void SessionHttp::OnConnected()
{
	AsyncRecv();
}



EW_LEAVE