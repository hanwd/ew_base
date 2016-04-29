#include "ewa_base/basic/object.h"
#include "ewa_base/basic/object_ex.h"
#include "ewa_base/collection/indexer_map.h"


EW_ENTER


class CreatorUseInfo : public Creator
{
public:

	CreatorUseInfo(int n)
	{
		kmap.rehash(n);
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


	void CreatorUseInfo::Register(ObjectInfo *info)
	{
		if(!info)
		{
			System::LogTrace("info=NULL in CreatorUseInfo::Register");
			return;
		}

		String name=info->GetName();
		if(name.empty())
		{
			return;
		}

		kmap[name]=info;
	}

	ObjectInfo *CreatorUseInfo::GetInfo(const String &name)
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

ObjectInfo::ObjectInfo(const String& s)
:m_sClassName(s)
{
	if(!m_sClassName.empty())
	{
		CreatorUseInfo::current().Register(this);
	}
}

ObjectInfo::~ObjectInfo(){}

Object* ObjectInfo::GetCachedInstance(){ return NULL; }

Object* ObjectInfo::GetHelperObject(const String&){return NULL;}

const String& ObjectInfo::GetName() const
{
	return m_sClassName;
}


EW_LEAVE
