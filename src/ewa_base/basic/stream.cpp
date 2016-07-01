#include "ewa_base/basic/stream.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/object_ex.h"
#include "ewa_base/net/socket.h"
#include "ewa_base/serialization/serializer_stream.h"
#include "ewa_base/serialization/serializer_socket.h"

EW_ENTER


inline void set_invalid_stream_error()
{
#ifdef EW_WINDOWS
	::SetLastError(6);
#endif

	errno=5;
}

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

	void update(){chksum=crc32(this,(char*)&chksum-(char*)this);}
	bool check(){ return chksum==crc32(this,(char*)&chksum-(char*)this);}
};


int SerializerReader::recv(char* data,int size)
{
	flags.add(FLAG_READER_FAILBIT);
	set_invalid_stream_error();
	return -1;
}

int SerializerWriter::send(const char* data,int size)
{
	flags.add(FLAG_WRITER_FAILBIT);
	set_invalid_stream_error();
	return -1;
}


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

bool SerializerReader::recv_all(char* data,int size)
{
	while(size>0)
	{
		int n=recv(data,size);
		if(n==size) return true;
		if(n<=0)
		{
			return false;
		}
		size-=n;
		data+=n;
	}
	return true;
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

bool SerializerWriter::send_all(const char* data,int size)
{
	while(size>0)
	{
		int n=send(data,size);
		if(n==size) return true;
		if(n<=0)
		{
			return false;
		}
		size-=n;
		data+=n;
	}
	return true;
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


int64_t SerializerReader::sizeg()
{
	int64_t sz=tellg();
	if(sz<0) return -1;
	int64_t z1=seekg(0,SEEKTYPE_END);
	if(z1<0) return -1;
	int64_t z2=seekg(sz,SEEKTYPE_BEG);
	if(z1!=z2)
	{
		System::LogError("seekg failed");
	}
	return z1;
}

int64_t SerializerWriter::sizep()
{
	int64_t sz=tellp();
	if(sz<0) return -1;
	int64_t z1=seekp(0,SEEKTYPE_END);
	if(z1<0) return -1;
	int64_t z2=seekp(sz,SEEKTYPE_BEG);
	if(z1!=z2)
	{
		System::LogError("seekp failed");
	}
	return z1;
}


int64_t SerializerDuplex::size()
{
	int64_t sz=tell();
	if(sz<0) return -1;
	int64_t z1=seek(0,SEEKTYPE_END);
	if(z1<0) return -1;
	int64_t z2=seek(sz,SEEKTYPE_BEG);
	if(z1!=z2)
	{
		System::LogError("seek failed");
	}
	return z1;
}

SerializerDuplex& SerializerDuplex::invalid_duplex_serializer()
{
	static class X : public SerializerDuplex
	{
	public:
		X()
		{
			reader().flags.add(FLAG_READER_FAILBIT|FLAG_WRITER_FAILBIT);
			writer().flags.add(FLAG_READER_FAILBIT|FLAG_WRITER_FAILBIT);
		}

	}g_instance;
	return g_instance;
}


bool Stream::write_to_file(const String& fp,int flag)
{
	if(!hReader)
	{
		 set_invalid_stream_error();;
		return false;
	}

	File file;
	if(!file.open(fp,FLAG_FILE_WC|flag))
	{
		return false;
	}

	char buffer[1024*32];
	while(1)
	{
		int rc=hReader.get()->recv(buffer,sizeof(buffer));
		if(rc>0)
		{
			if(file.write(buffer,rc)!=rc)
			{
				return false;
			}
		}
		else if(rc==0)
		{
			return true;
		}
		else
		{
			break;
		}
	}

	return false;
}

bool Stream::write_to_writer(SerializerWriter& wr)
{
	if(!hReader)
	{
		 set_invalid_stream_error();;
		return false;
	}

	char buffer[1024*32];
	while(1)
	{
		int rc=hReader.get()->recv(buffer,sizeof(buffer));
		if(rc>0)
		{
			if(!wr.send_all(buffer,rc))
			{
				return false;
			}
		}
		else if(rc==0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

bool Stream::write_to_buffer(StringBuffer<char>& sb)
{
	if(!hReader)
	{
		 set_invalid_stream_error();;
		return false;
	}

	try
	{
		char buffer[1024*32];
		while(1)
		{
			int rc=hReader.get()->recv(buffer,sizeof(buffer));
			if(rc>0)
			{
				sb.append(buffer,rc);
			}
			else if(rc==0)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	catch(std::exception& e)
	{
		EW_UNUSED(e);
	}

	return false;
}

bool Stream::read_from_file(const String& fp)
{

	if(!hWriter)
	{
		set_invalid_stream_error();
		return false;
	}

	File file;
	if(!file.open(fp,FLAG_FILE_RD))
	{
		return false;
	}

	char buffer[1024*32];
	while(1)
	{
		int rc=file.read(buffer,sizeof(buffer));
		if(rc>0)
		{
			if(!hWriter.get()->send_all(buffer,rc))
			{
				return false;
			}
		}
		else if(rc==0)
		{
			return true;
		}
		else
		{
			break;		
		}

	}

	return false;
}

bool Stream::read_from_reader(SerializerReader& rd)
{
	if(!hWriter)
	{
		 set_invalid_stream_error();;
		return false;
	}

	char buffer[1024*32];
	while(1)
	{
		int rc=rd.recv(buffer,sizeof(buffer));
		if(rc>0)
		{
			if(!hWriter.get()->send_all(buffer,rc))
			{
				return false;
			}
		}
		else if(rc==0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

bool Stream::read_from_buffer(StringBuffer<char>& sb)
{
	if(!hWriter)
	{
		 set_invalid_stream_error();;
		return false;
	}

	return hWriter.get()->send_all(sb.data(),sb.size());
}


bool Stream::open(const String& fp,int fg)
{
	File file;
	if(!file.open(fp,fg))
	{
		close();
		return false;
	}

	assign(file);
	return true;
}

bool Stream::connect(const String& ip,int port)
{
	Socket socket;
	if(!socket.connect(ip,port))
	{
		return false;
	}

	assign(socket);
	return true;
}

void Stream::assign(File& file)
{
	assign(SharedPtrT<SerializerDuplex>(new SerializerFile(file)));
}

void Stream::assign(Socket& socket)
{
	assign(SharedPtrT<SerializerDuplex>(new SerializerSocket(socket)));
}

void Stream::assign(SharedPtrT<SerializerDuplex> p)
{
	if(!p) return;
	hReader.reset(&p->reader(),p.counter());
	hWriter.reset(&p->writer(),p.counter());
}


void Stream::assign_reader(SharedPtrT<SerializerReader> p1)
{
	hReader=p1;
}

void Stream::assign_writer(SharedPtrT<SerializerWriter> p1)
{
	hWriter=p1;
}


int64_t Stream::seekg(int64_t p,int t)
{
	return reader().seekg(p,t);
}

int64_t Stream::tellg()
{
	return reader().tellg();
}

int64_t Stream::sizeg()
{
	return reader().sizeg();
}

int64_t Stream::seekp(int64_t p,int t)
{
	return writer().seekp(p,t);
}

int64_t Stream::tellp()
{
	return writer().tellp();
}

int64_t Stream::sizep()
{
	return writer().sizep();
}

int Stream::send(const char* buf,int len)
{
	return writer().send(buf,len);
}

int Stream::recv(char* buf,int len)
{
	return reader().recv(buf,len);
}

bool Stream::send_all(const char* buf,int len)
{
	return writer().send_all(buf,len);
}

bool Stream::recv_all(char* buf,int len)
{
	return reader().recv_all(buf,len);
}

SerializerReader& Stream::reader()
{
	return hReader ? *hReader : SerializerDuplex::invalid_duplex_serializer().reader();
}

SerializerWriter& Stream::writer()
{
	return hWriter ? *hWriter : SerializerDuplex::invalid_duplex_serializer().writer();
}

void Stream::close()
{
	hReader.reset();
	hWriter.reset();
}


EW_LEAVE
