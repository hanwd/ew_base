#include "ewa_base/serialization/serializer.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/codecvt.h"

EW_ENTER


Serializer::internal_head Serializer::head;
Serializer::internal_tail Serializer::tail;

int Serializer::global_version()
{
	return gver;
}

void Serializer::global_version(int v)
{
	gver=v;
}

SerializerWriter& Serializer::writer()
{
	if(!is_writer())
	{
		errstr("NOT_WRITER");
	}
	return *(SerializerWriter*)this;
}

SerializerReader& Serializer::reader()
{
	if(!is_reader())
	{
		errstr("NOT_READER");
	}
	return *(SerializerReader*)this;
}


void Serializer::errstr(const String& msg)
{
	flags.add(FLAG_READER_FAILBIT|FLAG_WRITER_FAILBIT);
	Exception::XError(msg);
}

void Serializer::errver()
{
	errstr("invalid_version");
}


Serializer::Serializer(int t)
	:type(t)
{
	gver=0;
}


SerializerEx::SerializerEx(int t)
	:Serializer(t)
{
	stream_data(NULL);
}

void SerializerEx::stream_data(DataPtrT<IStreamData> p)
{
	p_stream_data=p;
	if(!p_stream_data)
	{
		p_stream_data.reset(IStreamData::invalid_stream_data);
	}
}

CachedObjectManager& SerializerEx::cached_objects()
{
	if(!p_cached_objects)
	{
		p_cached_objects.reset(new CachedObjectManager);
	}
	return *p_cached_objects;
}

class serial_header
{
public:
	serial_header()
	{
		offset=0;
		size=0;
	}

	char tags[4];
	int32_t flags;
	int32_t gver;
	int32_t size;
	int64_t offset;
	int32_t chksum;
	int32_t padding;

	void update(){chksum=(int32_t)crc32(this,(char*)&chksum-(char*)this);}
	bool check(){ return chksum==(int32_t)crc32(this,(char*)&chksum-(char*)this);}
};



SerializerReader& SerializerReader::handle_head()
{
	serial_header header;
	recv_checked((char*)&header,sizeof(header));
	if(!header.check())
	{
		errstr("invalid header");
	}

	flags.clr(header.flags);
	global_version(header.gver);

	if(!flags.get(Serializer::FLAG_OFFSET_TABLE)) return *this;

	int64_t pos=tellg();
	if(pos>0 && seekg(header.offset,SEEKTYPE_BEG)>=0)
	{
		cached_objects().aOffset.resize(header.size);
		recv_checked((char*)cached_objects().aOffset.data(),sizeof(CachedObjectManager::PtrOffset)*cached_objects().aOffset.size());
		if(seekg(pos,SEEKTYPE_BEG)<0) errstr("seek error");
	}
	else
	{
		flags.del(FLAG_OFFSET_TABLE);
	}

	return *this;
}

SerializerReader& SerializerReader::handle_tail()
{
	cached_objects().handle_pending(*this);
	return *this;
}



SerializerWriter& SerializerWriter::handle_head()
{
	serial_header header;
	header.flags=flags.val()&~((1<<9)-1);
	header.gver=global_version();
	header.update();
	send_checked((char*)&header,sizeof(header));
	return *this;
}

SerializerWriter& SerializerWriter::handle_tail()
{
	cached_objects().handle_pending(*this);

	if(!flags.get(FLAG_OFFSET_TABLE))
	{
		return *this;
	}

	serial_header header;
	header.flags=flags.val()&~((1<<8)-1);
	header.gver=global_version();

	header.offset=tellp();
	if(header.offset<0)
	{
		return *this;
	}

	send_checked((char*)cached_objects().aOffset.data(),sizeof(CachedObjectManager::PtrOffset)*cached_objects().aOffset.size());
	header.size=cached_objects().aOffset.size();

	if(seekp(0,SEEKTYPE_BEG)==0)
	{
		header.update();
		send_checked((char*)&header,sizeof(header));
	}

	return *this;
}


void SerializerReader::recv_checked(char* data,int size)
{
	while(size>0)
	{
		int n=recv(data,size);
		if(n==size) return;
		if(n<=0)
		{
			errstr("invalid received size");
		}
		size-=n;
		data+=n;
	}
}

SerializerReader &SerializerReader::tag(char ch)
{
	char tmp;
	recv_checked(&tmp,1);
	if(tmp==ch)
	{
		errstr("invalid_tag");
	}
	return *this;
}

SerializerReader &SerializerReader::tag(const char *msg)
{
	int n=::strlen(msg);
	arr_1t<char> tmp;
	tmp.resize(n+1);
	recv_checked(&tmp[0],n+1);
	if(tmp[n]!='\0'||strcmp(msg,&tmp[0])!=0)
	{
		errstr("invalid_tag");
	}
	return *this;
}

int SerializerReader::size_count(int)
{
	int32_t n[2];
	recv_checked((char*)n,sizeof(int32_t)*2);
	if(n[0]!=~n[1]||n[0]<0)
	{
		this->errstr("invalid size_count");
	}
	return n[0];
}

int SerializerReader::local_version(int v)
{
	int32_t vh(-1);
	recv_checked((char *)&vh,sizeof(int32_t));
	if(vh<0||vh>v)
	{
		errstr("invalid_version");
		return -1;
	}
	return vh;
}

String SerializerReader::object_type(const String& )
{
	uint32_t sz;
	recv_checked((char*)&sz,4);
	if(sz>1024) errstr("invalid_object_type");

	StringBuffer<char> sb;
	sb.resize(sz);
	recv_checked(sb.data(),sz);

	return sb;
}

String SerializerWriter::object_type(const String& name)
{
	uint32_t sz(name.size());
	send_checked((char*)&sz,4);
	send_checked(name.c_str(),sz);
	return name;
}


void SerializerWriter::send_checked(const char* data,int size)
{
	while(size>0)
	{
		int n=send(data,size);
		if(n==size) return;
		if(n<=0)
		{
			errstr("invalid received size");
		}
		size-=n;
		data+=n;
	}
}

SerializerWriter &SerializerWriter::tag(char ch)
{
	send_checked(&ch,1);
	return *this;
}

SerializerWriter &SerializerWriter::tag(const char *msg)
{
	int n=::strlen(msg);
	send_checked((char *)msg,n+1);
	return *this;
}

int SerializerWriter::size_count(int n)
{
	int32_t v[2]={n,~n};
	send_checked((char *)&v,sizeof(int32_t)*2);
	return n;
}

int SerializerWriter::local_version(int v)
{
	int32_t vh(v);
	send_checked((char *)&vh,sizeof(int32_t));
	return v;
}


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


ObjectData* SerializerReader::read_object(int val)
{
	if(!flags.get(FLAG_OFFSET_TABLE)) return NULL;
	return cached_objects().read_object(*this,val);
}


template<typename A>
void serial_helper_func<A,String>::g(SerializerReader &ar,type &val)
{
	int n=ar.size_count(val.size());

	arr_1t<char> vect;
	vect.resize(n);

	ar.recv_checked(vect.data(),n);
	val.assign(vect.data(),n);

}


template<typename A>
void serial_helper_func<A,String>::g(SerializerWriter &ar,type &val)
{
	const char *p=val.c_str();
	int n=ar.size_count(val.size());
	if(n>0)	ar.send_checked((char *)p,n);
}

template class DLLIMPEXP_EWA_BASE serial_helper_func<SerializerWriter,String>;
template class DLLIMPEXP_EWA_BASE serial_helper_func<SerializerReader,String>;


EW_LEAVE
