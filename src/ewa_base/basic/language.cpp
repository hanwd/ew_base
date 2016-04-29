#include "ewa_base/basic/language.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/collection/indexer_map.h"
#include "ewa_base/collection/linear_buffer.h"
#include "ewa_base/ipc/shm.h"
#include "ewa_base/basic/pointer.h"
#include "ewa_base/basic/codecvt.h"


EW_ENTER


class const_char_pointer_map : public indexer_trait<const char*,const char*,int>
{
public:

	static int hashcode_key(const char* v)
	{
		hash_t<char*> h;
		return h(v);
	}

	static bool equal(const char* k,const char* v)
	{
		return strcmp(k,v)==0;
	}
};

//template<unsigned N>
//class lkt_white
//{
//public:
//	static const int value=!(N==' '||N=='\t'||N=='\r'||N=='\n');
//};
//
//template<unsigned N>
//class lkt_string
//{
//public:
//	static const int value=!(N=='\0'||N=='"');
//};

class LangData : public ObjectData
{
public:
	String m_sName;
	StringBuffer<char> m_aCont;


	bool ReadString(String& s,const char* &p1)
	{
		char ch;

		for(;;)
		{
			ch=*p1++;
			if(ch=='\0') return false;
			if(ch=='\"') break;
		}
		StringBuffer<char> sb;
		for(;;)
		{
			ch=*p1++;
			if(ch=='\\')
			{
				switch(ch=*p1++)
				{
				case 't':
					sb.push_back('\t');
					break;
				case 'n':
					sb.push_back('\n');
					break;
				case 'r':
					sb.push_back('\r');
					break;
				case '\'':
				case '\"':
					sb.push_back(ch);
					break;
				default:
					return false;
				}
				continue;
			}

			if(ch=='\"') break;
			if(ch=='\0') return false;

			sb.push_back(ch);
		}

		s=sb;
		return true;
	}


	bool LoadPo(const String& file)
	{

		m_sName=file;
		m_aCont.clear();

		StringBuffer<char> vt;
		if(!vt.load(file,FILE_TEXT))
		{
			return false;
		}

		const char* p1=vt.data();
		const char* p2=p1+vt.size();
		const char* pt=NULL;

		String tmp1,tmp2;

		while(p1<p2)
		{
			pt=strstr(p1,"msgid");
			if(!pt) return true;
			p1=pt+5;

			if(!ReadString(tmp1,p1))
			{
				return false;
			}

			pt=strstr(p1,"msgstr");
			if(!pt) return true;
			p1=pt+6;

			if(!ReadString(tmp2,p1))
			{
				return false;
			}

			if(tmp1==tmp2 || tmp1.empty() || tmp2.empty())
			{
				continue;
			}

			m_aCont.append(tmp1.c_str(),tmp1.size()+1);
			m_aCont.append(tmp2.c_str(),tmp2.size()+1);
		}

		return true;
	}

	bool LoadMo(const String& file)
	{
		m_sName=file;
		return m_aCont.load(file);
	}

	void AddPair(const std::pair<String,String>& kv)
	{
		m_aCont.append(kv.first.c_str(),kv.first.size()+1);
		m_aCont.append(kv.second.c_str(),kv.second.size()+1);
	}

};


class LanguageImpl : public ObjectData
{
public:

	arr_1t<DataPtrT<LangData> > aDatas;

	indexer_map<const char*,const char*,def_allocator,const_char_pointer_map> mapStrings;

	void Clear()
	{
		mapStrings.clear();
		aDatas.clear();
	}

	inline const String& Translate(const String& msg) const
	{
		int id=mapStrings.find1(msg.c_str());
		if(id<0) return msg;
		return *(String*)&mapStrings.get(id).second;
	}

	inline const String& Translate(const String& msg,const String& def) const
	{
		int id=mapStrings.find1(msg.c_str());
		if(id<0) return def;
		return *(String*)&mapStrings.get(id).second;
	}

	bool AddCatalog(const indexer_map<String,String>& mp)
	{
		DataPtrT<LangData> pData(new LangData);
		for(size_t i=0;i<mp.size();i++)
		{
			pData->AddPair(mp.get(i));
		}
		return AddCatalog(pData);
	}

	bool GetCatalog(indexer_map<String,String>& mp)
	{
		mp.clear();
		mp.reserve(mapStrings.size());

		for(size_t i=0;i<mapStrings.size();i++)
		{
			mp[mapStrings.get(i).first]=mapStrings.get(i).second;
		}
		return true;
	}

	bool AddCatalog(const String& file)
	{
		DataPtrT<LangData> pData(new LangData);

		if(file.size()>3 && file.substr(file.size()-3,3)!=".po")
		{
			if(!pData->LoadMo(file))
			{
				return false;
			}
		}
		else
		{
			if(!pData->LoadPo(file))
			{
				return false;
			}
		}

		return AddCatalog(pData);
	}



	bool AddCatalog(DataPtrT<LangData>& pData)
	{
		StringBuffer<char>& aCont(pData->m_aCont);

		StringBuffer<unsigned> vp;
		unsigned j=0;
		for(unsigned i=0; i<aCont.size(); i++)
		{
			if(aCont[i]!='\0')
			{
				continue;
			}
			vp.push_back(j);
			j=i+1;
		}

		mapStrings.rehash(vp.size()/2);
		for(size_t i=0; i<vp.size(); i+=2)
		{
			mapStrings[&aCont[vp[i]]]=&aCont[vp[i+1]];
		}

		aDatas.push_back(pData);
		return true;
	}

	bool SaveMo(const String& file)
	{
		StringBuffer<char> bt;
		size_t sz=mapStrings.size();
		for(size_t i=0; i<sz; i++)
		{
			const char* k=mapStrings.get(i).first;
			const char* v=mapStrings.get(i).second;
			bt.append(k,::strlen(k)+1);
			bt.append(v,::strlen(v)+1);
		}

		StringBuffer<char> bs;

		if(!IConv::ansi_to_utf8(bs,bt.data(),bt.size()))
		{
			return false;
		}

		return bs.save(file);
	}

};


bool Language::AddCatalog(const indexer_map<String,String>& mp)
{
	return ((LanguageImpl*)impl)->AddCatalog(mp);
}

bool Language::GetCatalog(indexer_map<String,String>& mp)
{
	return ((LanguageImpl*)impl)->GetCatalog(mp);
}

bool Language::AddCatalog(const String& file)
{
	return ((LanguageImpl*)impl)->AddCatalog(file);
}

bool Language::Save(const String& file)
{
	return ((LanguageImpl*)impl)->SaveMo(file);
}

void Language::Clear()
{
	((LanguageImpl*)impl)->Clear();
}

Language::Language()
{
	impl=new LanguageImpl;
	((LanguageImpl*)impl)->IncRef();
}

Language::~Language()
{
	((LanguageImpl*)impl)->DecRef();
}

void Language::Ref(Language& lang)
{
	if(impl==lang.impl) return;

	((LanguageImpl*)impl)->DecRef();
	impl=lang.impl;
	((LanguageImpl*)impl)->IncRef();
}

const String& Language::Translate(const String& msg) const
{
	return ((const LanguageImpl*)impl)->Translate(msg);
}

const String& Language::Translate(const String& msg,const String& def) const
{
	return ((const LanguageImpl*)impl)->Translate(msg,def);
}

Language& Language::current()
{
	static Language gInstance;
	return gInstance;
}


DLLIMPEXP_EWA_BASE const String& Translate(const String& msg)
{
	return Language::current().Translate(msg);
}

DLLIMPEXP_EWA_BASE const String& Translate(const String& msg,const String& def)
{
	return Language::current().Translate(msg,def);
}

EW_LEAVE
