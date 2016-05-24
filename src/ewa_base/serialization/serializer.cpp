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
	Exception::XError(msg);
}

void Serializer::errver()
{
	errstr("invalid_version");
}

bool Serializer::good() const
{
	return true;
}

template<typename A>
void serial_helper_func<A,String>::g(SerializerReader &ar,type &val)
{
	int n=ar.size_count(val.size());

	arr_1t<char> vect;
	vect.resize(n);

	ar.checked_recv(vect.data(),n);

	if(ar.flags.get(Serializer::FLAG_ENCODING_UTF8))
	{
		StringBuffer<char> sb;
		if(!IConv::utf8_to_ansi(sb,vect.data(),vect.size()))
		{
			ar.errstr("invalid string");
		}
		val=sb;
	}
	else
	{
		val.assign(vect.data(),n);
	}

}

template<typename A>
void serial_helper_func<A,String>::g(SerializerWriter &ar,type &val)
{
	const char *p;
	int n;

	if(ar.flags.get(Serializer::FLAG_ENCODING_UTF8))
	{
		StringBuffer<char> sb;
		if(!IConv::ansi_to_utf8(sb,val.c_str(),val.size()))
		{
			ar.errstr("invalid string");
			return;
		}
		p=sb.data();
		n=ar.size_count(sb.size());
		if(n>0)	ar.checked_send((char *)p,n);
	}
	else
	{
		p=val.c_str();
		n=ar.size_count(val.size());
		if(n>0)	ar.checked_send((char *)p,n);
	}

}

template class DLLIMPEXP_EWA_BASE serial_helper_func<SerializerWriter,String>;
template class DLLIMPEXP_EWA_BASE serial_helper_func<SerializerReader,String>;

Serializer::Serializer(int t)
	:type(t)
{
	flags.clr(FLAG_ENCODING_UTF8);
	gver=0;
}


SerializerEx::SerializerEx(int t)
	:Serializer(t)
{

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

	void update(){chksum=crc32(this,(char*)&chksum-(char*)this);}
	bool check(){ return chksum==crc32(this,(char*)&chksum-(char*)this);}
};

SerializerReader& SerializerReader::handle_head()
{
	serial_header header;
	checked_recv((char*)&header,sizeof(header));
	if(!header.check())
	{
		errstr("invalid header");
	}

	flags.clr(header.flags);
	global_version(header.gver);

	if(!flags.get(Serializer::FLAG_OFFSET_TABLE)) return *this;

	int64_t pos=tell();
	if(pos>0 &&seek(header.offset))
	{
		cached_objects.aOffset.resize(header.size);
		checked_recv((char*)cached_objects.aOffset.data(),sizeof(CachedObjectManager::PtrOffset)*cached_objects.aOffset.size());
		if(!seek(pos)) errstr("seek error");
	}	

	return *this;
}

SerializerReader& SerializerReader::handle_tail()
{
	cached_objects.handle_pending(*this);
	return *this;
}



SerializerWriter& SerializerWriter::handle_head()
{
	serial_header header;
	header.flags=flags.val()&~FLAG_OFFSET_TABLE;
	header.gver=global_version();
	header.update();
	checked_send((char*)&header,sizeof(header));
	return *this;
}

SerializerWriter& SerializerWriter::handle_tail()
{
	cached_objects.handle_pending(*this);

	if(!flags.get(FLAG_OFFSET_TABLE)) return *this;

	serial_header header;
	header.flags=flags.val();
	header.gver=global_version();

	header.offset=tell();
	if(header.offset>0)
	{
		checked_send((char*)cached_objects.aOffset.data(),sizeof(CachedObjectManager::PtrOffset)*cached_objects.aOffset.size());
		header.size=cached_objects.aOffset.size();

		if(seek(0))
		{
			header.update();
			checked_send((char*)&header,sizeof(header));		
		}
	}

	return *this;
}


void SerializerReader::checked_recv(char* data,size_t size)
{
	size_t n=recv(data,size);
	if(n!=size)
	{
		errstr("invalid received size");
	}
}

SerializerReader &SerializerReader::tag(char ch)
{
	char tmp;
	checked_recv(&tmp,1);
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
	checked_recv(&tmp[0],n+1);
	if(tmp[n]!='\0'||strcmp(msg,&tmp[0])!=0)
	{
		errstr("invalid_tag");
	}
	return *this;
}

int SerializerReader::size_count(int)
{
	int32_t n[2];
	checked_recv((char*)n,sizeof(int32_t)*2);
	if(n[0]!=~n[1]||n[0]<0)
	{
		this->errstr("invalid size_count");
	}
	return n[0];
}

int SerializerReader::local_version(int v)
{
	int32_t vh(-1);
	checked_recv((char *)&vh,sizeof(int32_t));
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
	checked_recv((char*)&sz,4);
	if(sz>1024) errstr("invalid_object_type");

	StringBuffer<char> sb;
	sb.resize(sz);
	checked_recv(sb.data(),sz);

	return sb;
}

String SerializerWriter::object_type(const String& name)
{
	uint32_t sz(name.size());
	checked_send((char*)&sz,4);	
	checked_send(name.c_str(),sz);
	return name;
}

void SerializerWriter::checked_send(const char* data,size_t size)
{
	size_t n=send(data,size);
	if(n!=size)
	{
		errstr("invalid received size");
	}
}

SerializerWriter &SerializerWriter::tag(char ch)
{
	checked_send(&ch,1);
	return *this;
}

SerializerWriter &SerializerWriter::tag(const char *msg)
{
	int n=::strlen(msg);
	checked_send((char *)msg,n+1);
	return *this;
}

int SerializerWriter::size_count(int n)
{
	int32_t v[2]={n,~n};
	checked_send((char *)&v,sizeof(int32_t)*2);
	return n;
}

int SerializerWriter::local_version(int v)
{
	int32_t vh(v);
	checked_send((char *)&vh,sizeof(int32_t));
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


//
//void CachedObjectManager::test_ptr_type(SerializerReader& ar,ObjectData* ptr)
//{
//	const String& type(ptr->GetObjectName());
//	int32_t size=type.size();
//
//	ar.checked_recv((char*)&size,4);
//	if(size!=type.size())
//	{
//		ar.errstr("invalid_object");
//	}
//
//	StringBuffer<char> sb;sb.resize(size);
//	ar.checked_recv(sb.data(),size);
//
//	if(memcmp(type.c_str(),sb.data(),size)!=0)
//	{
//		ar.errstr("invalid_object");
//	}
//}
//
//
//void CachedObjectManager::test_ptr_type(SerializerWriter& ar,ObjectData* ptr)
//{
//	const String& type(ptr->GetObjectName());
//	int32_t size=type.size();
//	ar.checked_send((const char*)&size,4);
//	ar.checked_send(type.c_str(),size);
//}


void CachedObjectManager::load_ptr(SerializerReader& ar,ObjectData* &ptr)
{

	int32_t val;
	ar.checked_recv((char*)&val,4);

	if(val==0)
	{
		ptr=NULL;
		return;
	}

	int32_t cnt=(int32_t)aLoader.size();
	if(val>=cnt)
	{
		aOffset.resize(val+1);
		aLoader.resize(val+1);
	}

	PtrLoader &loader(aLoader[val]);
	if(loader.m_ptr)
	{
		ptr=loader.m_ptr.get();
		return;
	}

	String name=ar.object_type("");

	loader.m_ptr.reset(ObjectCreator::current().CreateT<ObjectData>(name));
	ptr=loader.m_ptr.get();

	aObject[loader.m_ptr.get()]=val;

	pendings.append(val);



	
}

Object* CachedObjectManager::create(const String& name)
{
	return ObjectCreator::current().Create(name);
}

void CachedObjectManager::save_ptr(SerializerWriter& ar,ObjectData* ptr)
{

	if(ptr==NULL)
	{
		int32_t val=0;
		ar.checked_send((char*)&val,4);
		return;
	}

	int32_t val=aObject[ptr];

	if(val==0)
	{
		val=(int32_t)aLoader.size();
		aOffset.push_back(PtrOffset());
		aLoader.push_back(PtrLoader());

		PtrLoader &loader(aLoader[val]);
		loader.m_ptr=ptr;

		pendings.append(val);
	}

	ar.checked_send((const char*)&val,4);
	ar.object_type(ptr->GetObjectName());

}

void CachedObjectManager::handle_pending(SerializerWriter& ar)
{
	while(!pendings.empty())
	{
		arr_1t<int32_t> tmp;tmp.swap(pendings);
		for(size_t i=0;i<tmp.size();i++)
		{
			int val=tmp[i];
			if(aLoader[val].flags.get(PtrLoader::IS_LOADED)) continue;	
			aLoader[val].flags.add(PtrLoader::IS_LOADED);
			aOffset[val].lo=ar.tell();
			ar.object_type(aLoader[val].m_ptr->GetObjectName());
			aLoader[val].m_ptr->Serialize(ar);
			aOffset[val].hi=ar.tell();			
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
			PtrLoader& loader(aLoader[tmp[i]]);
			PtrOffset& offset(aOffset[tmp[i]]);

			if(loader.flags.get(PtrLoader::IS_LOADED)) continue;
	
			loader.flags.add(PtrLoader::IS_LOADED);

			if(use_seek && !ar.seek(offset.lo))
			{
				ar.errstr("seek error");
			}

			ar.object_type(loader.m_ptr->GetObjectName());
			loader.m_ptr->Serialize(ar);

			if(use_seek && offset.hi!=ar.tell())
			{
				ar.errstr("read error");
			}
		}
	}


	
}

ObjectData* CachedObjectManager::read_object(SerializerReader& ar,int val)
{

	if(val<1||val>=(int)aOffset.size()) ar.errstr("invalid object index");

	int64_t sz=aOffset[val].lo;


	if((int)aLoader.size()<=val)
	{
		aLoader.resize(val+1);
	}

	if(aLoader[val].flags.get(PtrLoader::IS_LOADED))
	{
		return aLoader[val].m_ptr.get();
	}

	if(!aLoader[val].m_ptr)
	{
		if(!ar.seek(sz)) return NULL;
		ObjectData* ptr=NULL;
		load_ptr(ar,ptr);
	}

	pendings.clear();
	pendings.append(val);

	handle_pending(ar,true);
	return aLoader[val].m_ptr.get();

}


ObjectData* SerializerReader::read_object(SerializerReader& ar,int val)
{
	if(!flags.get(FLAG_OFFSET_TABLE)) return NULL;
	return cached_objects.read_object(ar,val);
}

EW_LEAVE
