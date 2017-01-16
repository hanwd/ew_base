#include "ewa_base/basic/object.h"
#include "ewa_base/basic/object_ex.h"
#include "ewa_base/collection/indexer_map.h"
#include "ewa_base/basic/system.h"


EW_ENTER


class DLLIMPEXP_EWA_BASE CreatorUseInfo : public Creator
{
public:

	CreatorUseInfo(int n)
	{
		kmap.rehash(n);
	}

	~CreatorUseInfo()
	{
		if (!kmap.empty())
		{
			System::LogTrace("kmap is not empty at "__FUNCTION__);
		}
	}

	Object *Create(const String &name)
	{
		ObjectInfo *info=kmap[name];
		if(!info)
		{
			return NULL;
		}
		return info->CreateObject();
	}

	void Remove(ObjectInfo* info)
	{
		dlink.erase(info);

		String name = info->GetName();
		if (name.empty())
		{
			return;
		}
		kmap.erase(name);
	}

	void Append(ObjectInfo *info)
	{
		if(!info)
		{
			System::LogTrace("info=NULL in CreatorUseInfo::Register");
			return;
		}

		dlink.push_front(info);

		String name=info->GetName();
		if(name.empty())
		{
			return;
		}

		if (kmap.find(name)!=kmap.end())
		{
			System::LogTrace("%s already register",name);
		}

		kmap[name]=info;
	}

	ObjectInfo *GetInfo(const String &name)
	{
		int id=kmap.find1(name);
		if(id<0) return NULL;
		return kmap.get(id).second;
	}

	static CreatorUseInfo& current()
	{
		static DataPtrT<CreatorUseInfo> pInstance(new CreatorUseInfo(1024));
		return *pInstance;
	}


	indexer_map<String,ObjectInfo*> kmap;

	DLinkT<ObjectInfo> dlink;
};

ObjectCreator::ObjectCreator()
{
	SetData(&CreatorUseInfo::current());
}

ObjectCreator& ObjectCreator::current()
{
	static ObjectCreator gInstance;
	return gInstance;
}

Object* ObjectCreator::Create(const String& name)
{
	return m_refData ? m_refData->Create(name) : NULL;
}


ObjectInfo::ObjectInfo(const String& s)
:m_sClassName(s)
{
	CreatorUseInfo::current().Append(this);
}

ObjectInfo::~ObjectInfo()
{
	CreatorUseInfo::current().Remove(this);
}

void InvokeParam::OnInvoke(ObjectInfo* pinfo)
{
	pinfo->DoInvoke(*this);
}

void ObjectInfo::Invoke(int t)
{
	InvokeParam ipm(t);
	Invoke(ipm);
}

void ObjectInfo::Invoke(const String& s)
{
	InvokeParam ipm(InvokeParam::TYPE_CALL_ARGV);
	ipm.argvs.assign(1,s);
	Invoke(ipm);
}

void ObjectInfo::Invoke(InvokeParam& ipm)
{
	CreatorUseInfo& ci(CreatorUseInfo::current());
	for(auto it=ci.dlink.begin();it!=ci.dlink.end();++it)
	{
		ipm.OnInvoke(*it);
	}
}



void ObjectInfo::DoInvoke(InvokeParam&)
{

}

Object* ObjectInfo::CreateObject()
{
	return NULL;
}

Object* ObjectInfo::GetCachedInstance()
{ 
	return NULL;
}

Object* ObjectInfo::GetHelperObject(const String&)
{
	return NULL;
}

const String& ObjectInfo::GetName() const
{
	return m_sClassName;
}


EW_LEAVE
