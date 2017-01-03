#include "xml_parser.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/basic/exception.h"
#include "ewa_base/basic/clock.h"
#include "ewa_base/basic/codecvt.h"
#include "ewa_base/basic/console.h"
#include "ewa_base/logging/logger.h"

#include <fstream>

EW_ENTER

extern uint16_t g_uni_table[1024*64];

template<unsigned N>
class lkt_name
{
public:
	static const int value=(N!='\0' && N!='=' && N!='!' && N!='>' && N!='<' && N!='?' && N!='/' && N!='\\') && (!lkt_whitespace<N>::value);
};

template<unsigned N>
class lkt_attr_value
{
public:
	static const int value = lkt_name<N>::value || N == '-' || N == '=';
};

template<unsigned N>
class lkt_attr_name
{
public:
	static const int value = lkt_name<N>::value || N == '-';
};

template<unsigned N>
class lkt_not_gt
{
public:
	static const int value=!(N=='\0'||N=='>');
};


XmlParser::XmlParser(XmlDocument& xmldoc_):xmldoc(xmldoc_)
{
	flags.add(FLAG_KEEP_COMMENT|FLAG_KEEP_PI);
}

bool XmlParser::parse_document()
{
	xmldoc.DeleteChildren();
	xmldoc.DeleteAttributes();

	pcur=pbeg;
	pend=pcur+size;
	nodes.clear();

	nodes.push_back(&xmldoc);

	try
	{
		parse_subnodes();
	}
	catch(std::exception& e)
	{
		this_logger().LogError(e.what());
		nodes.clear();
		return false;
	}

	nodes.clear();
	return true;
}


void XmlParser::kerror(const String& msg)
{
	size_t pos=pcur-pbeg;
	(void)&pos;
	String desc=String::Format("xml_parser: %s",msg);
	Exception::XError(desc.c_str());
}

void XmlParser::kexpected(const String& msg)
{
	size_t pos=pcur-pbeg;
	(void)&pos;
	String desc=String::Format("xml_parser: %s expected",msg);
	Exception::XError(desc.c_str());
}


inline void  XmlParser::string_assign(String& s0,mychar_ptr p1,mychar_ptr p2)
{
	if(p1[0]=='\''||p1[0]=='\"')
	{
		p1=p1+1;
		p2=p2-1;
	}

	tempbuf.reserve(p2-p1);

	// &amp; &apos; &quot; &gt; &lt; &#...;
	mychar* dest=tempbuf.data();
	while(p1!=p2)
	{
		if(p1[0]=='&')
		{
			switch(p1[1])
			{
			case 'a':
				if(p1[2]=='m'&&p1[3]=='p'&&p1[4]==';')
				{
					*dest++='&';
					p1+=5;
				}
				else if(p1[2]=='p'&&p1[3]=='o'&&p1[4]=='s'&&p1[5]==';')
				{
					*dest++='\'';
					p1+=6;
				}
				else
				{
					pcur=p1;
					kexpected("&amp; or &apos;");
					return;
				}
				break;
			case 'g':
				if(p1[2]=='t'&&p1[3]==';')
				{
					*dest++='>';
					p1+=4;
				}
				else
				{
					pcur=p1;
					kexpected("&gt;");
					return;
				}
				break;
			case 'l':
				if(p1[2]=='t'&&p1[3]==';')
				{
					*dest++='<';
					p1+=4;
				}
				else
				{
					pcur=p1;
					kexpected("&lt;");
					return;
				}
				break;
			case 'q':
				if(p1[2]=='u'&&p1[3]=='o'&&p1[4]=='t'&&p1[5]==';')
				{
					*dest++='\"';
					p1+=6;
				}
				else
				{
					pcur=p1;
					kexpected("&quot;");
					return;
				}
				break;
			case '#':
			{
				uint32_t code = 0;
				if(p1[2]=='x')
				{
					p1+=3;
					for(;;)
					{
						unsigned char digit =lookup_table<lkt_number16b>::test(p1[0]);
						if (digit == 0xFF) break;
						code=(code<<4)+digit;
						p1+=1;
					}
				}
				else
				{
					p1+=2;
					for(;;)
					{
						unsigned char digit =lookup_table<lkt_number10b>::test(p1[0]);
						if (digit == 0xFF) break;
						code=(code*10)+digit;
						p1+=1;
					}
				}
				if(p1[0]!=';')
				{
					pcur=p1;
					kexpected(";");
					return;
				}
				p1+=1;

#ifdef EW_XML_UTF8
				if (code < 0x80)
				{
					dest[0]=code;
					dest+=1;
				}
				else if (code < 0x800)
				{
					dest[1] = ((code | 0x80) & 0xBF);
					code >>= 6;
					dest[0] = (code | 0xC0);
					dest+=2;
				}
				else if (code < 0x10000)
				{
					dest[2] = ((code | 0x80) & 0xBF);
					code >>= 6;
					dest[1] = ((code | 0x80) & 0xBF);
					code >>= 6;
					dest[0] = (code | 0xE0);
					dest+=3;
				}
				else if (code < 0x110000)
				{
					dest[3] = ((code | 0x80) & 0xBF);
					code >>= 6;
					dest[2] = ((code | 0x80) & 0xBF);
					code >>= 6;
					dest[1] = ((code | 0x80) & 0xBF);
					code >>= 6;
					dest[0] = (code | 0xF0);
					dest+=4;
				}
				else
				{
					pcur=p1;
					kerror("invalid unicode number");
					return;
				}
#else
				if(code<0x81)
				{
					*dest++=code;
				}
				else if((code&0xFFFF0000)!=0)
				{
					System::LogWarning("IConv_unicode_to_gbk: unkown unicode character %x",(uint32_t)code);

					dest[0]='?';
					dest[1]='?';
					dest+=2;
				}
				else
				{
					uint16_t val=g_uni_table[code];
					if(val!=0)
					{
						dest[0]=(((unsigned char*)&val)[0]);
						dest[1]=(((unsigned char*)&val)[1]);
						dest+=2;
					}
					else
					{
						*dest++=code&0x7F;
					}
				}
#endif

			}
			break;
			default:
				*dest++=*p1++;
				break;
			}
		}
		else
		{
			*dest++=*p1++;
		}
	}

	s0.assign(tempbuf.data(),dest);
}


template<unsigned N>
class lkt_string_key1
{
public:
	static const int value=!(N=='\0'||N=='\''||N=='\\');
};
inline void XmlParser::parse_string1()
{
	pcur+=1;
	for(;;)
	{
		helper::skip<lkt_string_key1>(pcur);
		if(pcur[0]=='\'')
		{
			pcur+=1;
			return;
		}
		else if(pcur[0]=='\\')
		{
			if(pcur[1]=='\0')
			{
				kerror("unexpected data end");
				return;
			}
			pcur+=2;
			continue;
		}
		else
		{
			kexpected("\'");
			return;
		}
	}
}

template<unsigned N>
class lkt_string_key2
{
public:
	static const int value=!(N=='\0'||N=='\"'||N=='\\');
};
inline void XmlParser::parse_string2()
{
	pcur+=1;
	for(;;)
	{
		helper::skip<lkt_string_key2>(pcur);
		if(pcur[0]=='\"')
		{
			pcur+=1;
			return;
		}
		else if(pcur[0]=='\\')
		{
			if(pcur[1]=='\0')
			{
				kerror("unexpected data end");
				return;
			}

			pcur+=2;
			continue;
		}
		else
		{
			kexpected("\"");
			return;
		}
	}
}


inline void XmlParser::parse_value()
{
	switch(pcur[0])
	{
	case '\'':
		parse_string1();
		break;
	case '\"':
		parse_string2();
		break;
	default:
		helper::skip<lkt_attr_value>(pcur);
		while (pcur[0] == '/'&&pcur[1] != '>')
		{
			pcur++;
			helper::skip<lkt_attr_value>(pcur);
		}
	}
}


inline void XmlParser::parse_comment_node()
{
	if(pcur[3]!='-')
	{
		kexpected("<!--");
	}
	pcur+=4;

	mychar_ptr p1=pcur;

	helper::skip<lkt_not_gt>(pcur);
	if(pcur[0]!='>')
	{
		kexpected(">");
	}

	if(pcur[-1]!='-'||pcur[-2]!='-')
	{
		pcur-=2;
		kexpected("-->");
	}

	mychar_ptr p2=pcur-2;

	if(flags.get(FLAG_KEEP_COMMENT))
	{
		XmlNode* pnode=CreateNode(XmlNode::XMLNODE_COMMENT);
		pnode->m_sValue.assign(p1,p2);

		add_node(pnode);
	}

	pcur+=1;
}


template<unsigned N>
class lkt_not_ltgt
{
public:
	static const int value=!(N=='\0'||N=='>'||N=='<');
};
inline void XmlParser::parse_doctype_node()
{
	pcur+=2;

	skip_tag(pcur,"DOCTYPE");

	size_t lt=1;

	for(;;)
	{
		helper::skip<lkt_not_ltgt>(pcur);
		if(pcur[0]=='>')
		{
			pcur+=1;
			if(--lt==0)
			{
				return;
			}
		}
		else if(pcur[0]=='<')
		{
			pcur+=1;
			++lt;
		}
		else
		{
			kexpected(">");
			return;
		}
	}


}


inline void XmlParser::parse_cdata()
{
	pcur+=2;

	skip_tag(pcur,"[CDATA[");

	mychar_ptr tagvalue1=pcur;
	for(;;)
	{
		helper::skip<lkt_not_gt>(pcur);
		if(pcur[0]=='\0')
		{
			kexpected("]]>");
			return;
		}
		if(pcur[-1]==']'&&pcur[-2]==']')
		{
			break;
		}
		pcur+=1;
	}
	mychar_ptr tagvalue2=pcur-2;
	XmlNode* pnode=CreateNode(XmlNode::XMLNODE_CDATA);
	pnode->m_sValue.assign(tagvalue1,tagvalue2);
	nodes.back()->AppendChild(pnode);
	pcur+=1;
}



inline void XmlParser::parse_instruction_node()
{
	pcur+=2;
	mychar_ptr p1=pcur;
	helper::skip<lkt_name>(pcur);
	mychar_ptr p2=pcur;


	XmlNode* pnode=CreateNode(XmlNode::XMLNODE_PI);
	pnode->m_sName.assign(p1,p2);

	put_node(pnode);
	parse_attributes();

	if(pcur[0]!='?'||pcur[1]!='>')
	{
		kexpected("?>");
	}
	pcur+=2;

	pop_node();

	if(!flags.get(FLAG_KEEP_PI))
	{
		nodes.back()->RemoveChild(pnode);
	}

}

template<unsigned N>
class parse_data_key
{
public:
	static const int value=!(N=='\0'||N=='<');
};

inline void XmlParser::parse_subnodes()
{
	mychar_ptr tagvalue1,tagvalue2;
	for(;;)
	{
		tagvalue1=pcur;
		helper::skip<lkt_whitespace>(pcur);
		if(pcur[0]=='<')
		{
			switch(pcur[1])
			{
			case '/':
				pcur+=2;
				tagvalue1=pcur;
				helper::skip<lkt_name>(pcur);
				tagvalue2=pcur;
				helper::skip<lkt_whitespace>(pcur);
				if(pcur[0]!='>')
				{
					kexpected(">");
					return;
				}
				pcur+=1;

				//if (nodes.back()->GetName() != String(tagvalue1, tagvalue2))
				//{
				//	String value(tagvalue1, tagvalue2);
				//	pcur = pcur;
				//}
				return;

			case '!':
				switch(pcur[2])
				{
				case '-':
					parse_comment_node();
					break;
				case 'd':
				case 'D':
					parse_doctype_node();
					break;
				case '[':
					parse_cdata();
					break;
				default:
					kerror("unexpected character");
					return;
				}
				continue;
			case '?':
				parse_instruction_node();
				continue;
			default:
				parse_element_node();
				break;
			}
		}
		else
		{
			helper::skip<parse_data_key>(pcur);
			if(pcur[0]!='<')
			{
				if(pcur[0]=='\0')
				{
					return;
				}

				kexpected("<");
				return;
			}

			tagvalue2=pcur;
			if(nodes.back()->m_sValue.empty())
			{
				this->string_assign(nodes.back()->m_sValue,tagvalue1,tagvalue2);
			}

			XmlNode* pnode=CreateNode(XmlNode::XMLNODE_DATA);
			pnode->m_sValue.assign(tagvalue1,tagvalue2);
			nodes.back()->AppendChild(pnode);
		}
	}
}

inline void XmlParser::parse_attributes()
{
	for(;;)
	{

		helper::skip<lkt_whitespace>(pcur);
		if (!lookup_table<lkt_attr_name>::test(pcur[0])) return;

		mychar_ptr attrname1=pcur;
		helper::skip<lkt_attr_name>(pcur);
		mychar_ptr attrname2=pcur;

		AutoPtrT<XmlAttribute> attr(CreateAttr());

		helper::skip<lkt_whitespace>(pcur);

		if(pcur[0]=='=')
		{
			pcur+=1;
			helper::skip<lkt_whitespace>(pcur);

			mychar_ptr attrvalue1=pcur;
			parse_value();
			mychar_ptr attrvalue2=pcur;	

			string_assign(attr->m_sValue,attrvalue1,attrvalue2);
		}

		attr->m_sName.assign(attrname1,attrname2);
		nodes.back()->AppendAttribute(attr.release());

	}
}

inline void XmlParser::parse_element_node()
{
	pcur+=1;

	mychar_ptr tagname1=pcur;
	helper::skip<lkt_name>(pcur);
	mychar_ptr tagname2=pcur;

	XmlNode* pnode=CreateNode();
	pnode->m_sName.assign(tagname1,tagname2);

	put_node(pnode);

	parse_attributes();

	switch(pcur[0])
	{
	case '/':
		if(pcur[1]!='>')
		{
			kexpected(">");
		}
		pcur+=2;
		break;
	case '>':
		pcur+=1;
		parse_subnodes();
		break;
	default:
		kexpected(">");
	}

	pop_node();
}


bool XmlParser::LoadStr(const char* pstr_,size_t size_)
{
	if(pstr_[size_]=='\0'||(size_>0&&pstr_[size_-1]!='\0'))
	{
		pbeg=(mychar_ptr)pstr_;
		size=size_;
	}
	else
	{
		buffer.assign(pstr_,size_);
		pbeg=buffer.c_str();
		size=buffer.size();
	}

	return parse_document();
}

bool XmlParser::LoadXml(const String& f,int t)
{
	if(!buffer.load(f,t))
	{
		return false;
	}

	size=buffer.size();
	pbeg=buffer.c_str();

	return parse_document();
}

bool XmlParser::SaveXml(const String& s)
{
	std::ofstream ofs(s.c_str());
	if(!ofs.good()) return false;

	savebuf.clear();
	savebuf<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

#ifdef EW_WINDOWS
	StringBuffer<char> tmpb;
	if(IConv::ansi_to_utf8(tmpb,savebuf.data(),savebuf.size()))
	{
		tmpb.swap(savebuf);
	}
	else
	{
		System::LogTrace("IConv::ansi_to_utf8 failed in XmlParser::SaveXml");
	}
#endif

	for(XmlNode* pnode=xmldoc.GetFirstChild(); pnode; pnode=pnode->GetNext())
	{
		savenode(pnode,0);
	}

	ofs.write(savebuf.data(),savebuf.size());
	ofs.close();

	return true;
}

void XmlParser::savestring(const String& v)
{

	const char* p1=v.c_str();
	const char px[]="&quot;";
	for(;;)
	{
		const char* p2=::strstr(p1,"\"");
		if(p2==NULL)
		{
			savebuf<<p1;
			return;
		}
		savebuf.append(p1,p2-p1);
		savebuf.append(px,6);
		p1=p2+1;
	}
}

void XmlParser::savenode(XmlNode* pnode,int lv)
{

	if(pnode->GetType()==XmlNode::XMLNODE_DATA)
	{
		savebuf<<pnode->GetValue();
		return;
	}

	if(pnode->GetType()==XmlNode::XMLNODE_CDATA)
	{
		savebuf<<"\n<![CDATA[[";
		savebuf<<pnode->GetValue();
		savebuf<<"]]>\n";
		return;
	}

	tabindent(lv);

	if(pnode->GetType()==XmlNode::XMLNODE_COMMENT)
	{
		savebuf<<"<!--";
		savebuf<<pnode->GetValue();
		savebuf<<"-->\n";
		return;
	}

	if(pnode->GetType()==XmlNode::XMLNODE_PI)
	{

		if(pnode->GetName()=="xml")
		{
			return;
		}

		savebuf<<"<?"<<pnode->GetName();
		for(XmlAttribute* pattr=pnode->GetFirstAttribute(); pattr; pattr=pattr->GetNext())
		{
			savebuf<<" "<<pattr->GetName()<<"=\"";
			savestring(pattr->GetValue());
			savebuf<<"\"";
		}

		savebuf<<"?>\n";
		return;
	}


	savebuf<< "<" << pnode->GetName();

	for(XmlAttribute* pattr=pnode->GetFirstAttribute(); pattr; pattr=pattr->GetNext())
	{
		savebuf<<" "<<pattr->GetName()<<"=\"";
		savestring(pattr->GetValue());
		savebuf<<"\"";
	}

	XmlNode* fnode=pnode->GetFirstChild();

	if(fnode==NULL)
	{
		savebuf<<" />\n";
		return;
	}

	savebuf<<">";

	if(fnode->GetNext()==NULL && fnode->GetType()==XmlNode::XMLNODE_DATA)
	{
		savenode(fnode);
	}
	else
	{
		if(fnode->GetType()==XmlNode::XMLNODE_ELEMENT)
		{
			savebuf<<"\n";
		}
		for(XmlNode* tnode=fnode; tnode; tnode=tnode->GetNext())
		{
			savenode(tnode,lv+1);
		}

		tabindent(lv);
	}
	savebuf<< "</" << pnode->GetName()<<">"<<"\n";

}

EW_LEAVE
