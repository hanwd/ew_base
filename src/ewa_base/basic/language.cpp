
#include "ewa_base/basic/string.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/collection/indexer_map.h"
#include "ewa_base/collection/linear_buffer.h"
#include "ewa_base/ipc/shm.h"
#include "ewa_base/basic/pointer.h"
#include "ewa_base/basic/codecvt.h"
#include "ewa_base/basic/fso.h"
#include "ewa_base/basic/language.h"
#include "ewa_base/util/strlib.h"
#include "system_data.h"


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
		if(!vt.load(file,FILE_TYPE_TEXT))
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

	String sLanguage;

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

		return bt.save(file);
	}

};


bool Language::AddCatalog(const indexer_map<String,String>& mp)
{
	return ((LanguageImpl*)m_pimpl.get())->AddCatalog(mp);
}

bool Language::GetCatalog(indexer_map<String,String>& mp)
{
	return ((LanguageImpl*)m_pimpl.get())->GetCatalog(mp);
}

bool Language::AddCatalog(const String& file)
{
	return ((LanguageImpl*)m_pimpl.get())->AddCatalog(file);
}

bool Language::Save(const String& file)
{
	return ((LanguageImpl*)m_pimpl.get())->SaveMo(file);
}

void Language::Clear()
{
	((LanguageImpl*)m_pimpl.get())->Clear();
}

Language::Language()
{
	m_pimpl.reset(new LanguageImpl);
}


const String& Language::Translate(const String& msg) const
{
	return ((const LanguageImpl*)m_pimpl.get())->Translate(msg);
}

const String& Language::Translate(const String& msg,const String& def) const
{
	return ((const LanguageImpl*)m_pimpl.get())->Translate(msg,def);
}


bool Language::SetLanguage(const String& s)
{
	LanguageImpl& impl(*(LanguageImpl*)m_pimpl.get());
	impl.sLanguage=s;
	impl.Clear();
	bool flag=impl.AddCatalog("languages\\"+s+"\\default.po");
	_language_updated();
	return flag;

}

String Language::GetLanguage()
{
	return ((LanguageImpl*)m_pimpl.get())->sLanguage;
}

Language& Language::operator=(const Language& o)
{
	m_pimpl=o.m_pimpl;
	_language_updated();
	return *this;
}

Language& Language::current()
{
	static Language gInstance;
	return gInstance;
}


void Language::_language_updated()
{
	if(this!=&current()) return;

	SystemData& sdata(SystemData::current());
	sdata.sLanguage=GetLanguage();
#ifdef EW_WINDOWS
	sdata.nLangId=string_to_lower(sdata.sLanguage)=="english"?MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT):0;
#endif
}

arr_1t<String> Language::GetLanguages()
{
	String langdir=System::GetModulePath()+"/languages";
	arr_1t<FileItem> items=FSLocal::current().FindFilesEx(langdir);

	arr_1t<String> langs;
	langs.push_back("English");

	for(size_t i=0;i<items.size();i++)
	{
		if(!items[i].flags.get(FileItem::IS_FOLDER)) continue;
		if(items[i].filename.c_str()[0]=='.') continue;
		if(string_to_lower(items[i].filename)=="english") continue;
		langs.push_back(items[i].filename);
	}

	return langs;
}



const String& Translate(const String& msg)
{
	return Language::current().Translate(msg);
}

const String& Translate(const String& msg,const String& def)
{
	return Language::current().Translate(msg,def);
}


EW_LEAVE
