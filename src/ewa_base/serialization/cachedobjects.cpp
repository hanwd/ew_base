#include "ewa_base/serialization/serializer.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/codecvt.h"

EW_ENTER



void CachedObjectManager::clear()
{
	aOffset.clear();
	aLoader.clear();
	aObject.clear();

	aOffset.resize(1);
	aLoader.resize(1);
}


ObjectData* CachedObjectManager::load_ptr(SerializerReader& ar,int32_t val)
{
	EW_ASSERT(val!=0);

	int32_t cnt=(int32_t)aLoader.size();
	if(val>=cnt)
	{
		aLoader.resize(val+1);
	}

	String name=ar.object_type("");

	PtrLoader &loader(aLoader[val]);
	if(!loader.m_ptr)
	{
		loader.m_ptr.reset(ObjectCreator::current().CreateT<ObjectData>(name));
		aObject[loader.m_ptr.get()]=val;
		pendings.append(val);
	}

	return loader.m_ptr.get();
}

Object* CachedObjectManager::create(const String& name)
{
	return ObjectCreator::current().Create(name);
}


void CachedObjectManager::save_ptr(SerializerWriter& ar,ObjectData* ptr,bool write_index)
{
	EW_ASSERT(ptr!=NULL);

	int32_t &val(aObject[ptr]);

	if(val==0)
	{
		val=(int32_t)aLoader.size();
		aOffset.push_back(PtrOffset());
		aLoader.push_back(PtrLoader());
		aLoader[val].m_ptr.reset(ptr);
		pendings.append(val);
	}

	if(write_index)
	{
		ar.send_checked((const char*)&val,4);
	}
	ar.object_type(ptr->GetObjectName());

}

void CachedObjectManager::handle_pending(SerializerWriter& ar)
{
	while(!pendings.empty())
	{
		arr_1t<int32_t> tmp;tmp.swap(pendings);
		for(size_t i=0;i<tmp.size();i++)
		{
			int32_t val=tmp[i];
			if(aLoader[val].flags.get(PtrLoader::IS_LOADED)) continue;
			aLoader[val].flags.add(PtrLoader::IS_LOADED);
			aOffset[val].lo=ar.tellp();
			ar.object_type(aLoader[val].m_ptr->GetObjectName());
			aLoader[val].m_ptr->Serialize(ar);
			aOffset[val].hi=ar.tellp();
		}
	}

}

void CachedObjectManager::handle_pending(SerializerReader& ar,bool use_seek)
{

	while(!pendings.empty())
	{
		arr_1t<int32_t> tmp;tmp.swap(pendings);

		for(size_t i=0;i<tmp.size();i++)
		{
			int val=tmp[i];
			PtrLoader& loader(aLoader[val]);

			if(loader.flags.get(PtrLoader::IS_LOADED)) continue;

			loader.flags.add(PtrLoader::IS_LOADED);

			if(use_seek && !ar.seekg(aOffset[val].lo,SEEKTYPE_BEG))
			{
				ar.errstr("seek error");
			}

			ar.object_type(loader.m_ptr->GetObjectName());
			loader.m_ptr->Serialize(ar);

			if(use_seek && aOffset[val].hi!=ar.tellg())
			{
				ar.errstr("read error");
			}
		}
	}

}

size_t CachedObjectManager::shrink()
{
	size_t _nCount=0;
	for(size_t _nLastCount=-1;_nLastCount!=_nCount;)
	{
		_nLastCount=_nCount;
		for(size_t i=1;i<aLoader.size();i++)
		{
			if(!aLoader[i].m_ptr || aLoader[i].m_ptr->GetRef()>1) continue;
			aObject.erase(aLoader[i].m_ptr.get());
			aLoader[i].flags.del(PtrLoader::IS_LOADED);
			aLoader[i].m_ptr.reset(NULL);
			_nCount++;
		}	
	}
	return _nCount;
}

ObjectData* CachedObjectManager::read_object(SerializerReader& ar,int val)
{

	if(val<1||val>=(int)aOffset.size()) ar.errstr("invalid object index");

	int64_t sz=aOffset[val].lo;

	if((int)aLoader.size()<=val)
	{
		aLoader.resize(val+1);
	}
	else if(aLoader[val].flags.get(PtrLoader::IS_LOADED))
	{
		return aLoader[val].m_ptr.get();
	}

	if(!aLoader[val].m_ptr)
	{
		if(!ar.seekg(sz,SEEKTYPE_BEG))
		{
			ar.errstr("seek error");
			return NULL;
		}
		load_ptr(ar,val);
	}

	pendings.clear();
	pendings.append(val);
	handle_pending(ar,true);

	return aLoader[val].m_ptr.get();

}

EW_LEAVE
