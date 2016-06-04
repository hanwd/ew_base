
#include "ewa_base/net/http.h"
#include "ewa_base/util/strlib.h"
EW_ENTER

HttpBase::HttpBase():type(-1){}

void HttpBase::clear()
{
	head="";
	prop.clear();
	data.clear();
}

bool HttpBase::parse(const StringBuffer<char>& sb)
{

	const char* cont=sb.c_str();
	const char* orig=cont;
	const char* cend=cont+sb.size();

	const char* pbeg=cont;
	const char* pmid=NULL;

	while(cont<cend)
	{
		char ch=*cont++;
		if(ch==':')
		{
			if(pmid==NULL) pmid=cont-1;
			continue;
		}
		if(ch!='\n')
		{
			continue;
		}

		if(pbeg==orig) // first line
		{
			head.assign(pbeg,cont);
			pbeg=cont;
		}
		else if(pmid!=NULL)
		{
			const char* pend=cont[-2]=='\r'?cont-2:cont-1;
			String pp=String(pbeg,pmid);

			if(pp=="Set-Cookie")
			{
				pp=String(pmid+2,pend);
				arr_1t<String> s1 = string_split(pp, ";");

				Cookie item;

				for (size_t i = 0; i < s1.size(); ++i)
				{
					int eq = s1[i].find('=');
					if (eq > 0)
					{
						String p1 = s1[i].substr(s1[i].c_str()[0]==' ', eq);
						String p2 = s1[i].substr(eq + 1);

						if (i == 0)
						{
							item.name = p1;
							item.value = p2;
						}
						else if (p1 == "path")
						{
							item.path = p2;
						}
						else if (p1 == "domain")
						{
							item.domain = p2;
						}
					}

				}

				if (item.name != "")
				{
					cookie[item.name] = item;
				}

			}
			else
			{
				prop[pp].assign(pmid+2,pend);
			}
			pbeg=cont;
			pmid=NULL;
		}
		else
		{
			break;
		}
	}

	int id=prop.find1("Transfer-Encoding");
	if(id>=0 && prop.get(id).second=="chunked")
	{
		int len=0;
		while(cont<cend)
		{
			char ch=*cont++;
			if((ch>='0'&&ch<='9')||(ch>='a'&&ch<='f')||(ch>='A'&&ch<='F'))
			{
				if(ch>='0'&&ch<='9')
				{
					len=len*16+ch-'0';
				}
				else if(ch>='a'&&ch<='f')
				{
					len=len*16+10+ch-'a';
				}
				else
				{
					len=len*16+10+ch-'A';
				}
			}
			else if(ch==0x0d&&cont[0]==0x0a)
			{
				cont++;
				if(len>cend-cont)
				{
					len=cend-cont;
				}
				data.append(cont,len);
				cont+=len;
				continue;
			}
			else
			{
				break;
			}

		}
	}
	else
	{
		data.assign(cont,cend);
	}

	return true;

}


HttpResponse::HttpResponse()
{
		
}

bool HttpResponse::parse(const StringBuffer<char>& sb)
{
	EW_FUNCTION_TRACER(LOGLEVEL_TRACE);

	type=-1;
	if(!HttpBase::parse(sb)) return false;
	const char* pbeg=head.c_str();
	const char* cont=head.c_str()+head.size();

	for(const char* h1=pbeg+4;h1<cont;h1++)
	{
		if(*h1!=' ') continue;
		for(const char* h2=h1+1;h2<cont;h2++)
		{
			if(*h2!=' ') continue;
			String(h1+1,h2).ToNumber(&type);
			break;
		}
		break;
	}

	return true;
}

HttpRequest::HttpRequest()
{
	type = HTTP_GET;
	port = 80;
}


void HttpRequest::clear()
{
	HttpBase::clear();

	type=HTTP_GET;

	prop["Accept"];
	prop["Accept-Encoding"]="identity";
	prop["Accept-Charset"];
	prop["Accept-Language"]="zh_cn";
	prop["Authorization"];
	prop["Host"];
	prop["User-Agent"];
	prop["Connection"];
}

DataPtrT<HttpResponse> HttpRequest::Execute()
{
	EW_FUNCTION_TRACER(LOGLEVEL_TRACE);

	if(prop["Host"].empty()) return false;

	StringBuffer<char> sb;
	if (type == HTTP_GET)
	{
		sb<<"GET "<<uri<<" HTTP/1.1\r\n";
	}
	else if (type == HTTP_POST)
	{
		sb<<"POST "<<uri<<" HTTP/1.1\r\n";
	}
	else
	{
		return NULL;
	}

	for(size_t i=0;i<prop.size();i++)
	{
		if(prop.get(i).second.empty()) continue;
		sb<<prop.get(i).first<<": "<<prop.get(i).second<<"\r\n";
	}

	for(size_t i=0;i<cookie.size();i++)
	{
		if(cookie.get(i).second.value.empty()) continue;
		sb << "Cookie:" << cookie.get(i).first << "=" << cookie.get(i).second.value << "\r\n";
	}

	sb<<"\r\n";
	if (!postdata.empty())
	{
		arr_1t<String> pd;
		std::for_each(postdata.begin(), postdata.end(), [&pd](indexer_map<String, String>::value_type& v)
		{
			pd.push_back(v.first + "=" + v.second);
		});

		sb << string_join(pd.begin(), pd.end(), "&");
		sb<<"\r\n";
	}


	Socket socket;
	if (!socket.connect(prop["Host"], port))
	{
		return NULL;
	}

	int sz=socket.send(sb.c_str(),sb.size());
	if(sz!=(int)sb.size())
	{
		sz=sz;
	}

	DataPtrT<HttpResponse> res(new HttpResponse);
	StringBuffer<char> buf;
	buf.resize(1024*64);

	sb.clear();

	for(;;)
	{	
		int len=socket.recv(buf.data(),buf.size());

		if(len<0)
		{
			break;
		}
		else if(len==0)
		{
			break;
		}

		if(sb.empty())
		{
			socket.shutdown(1);
		}

		sb.append(buf.data(),len);
	}

	socket.shutdown();

	//sb.save("http_res.txt");

	if(!res->parse(sb))
	{
		return NULL;
	}

	return res;		
}

EW_LEAVE
