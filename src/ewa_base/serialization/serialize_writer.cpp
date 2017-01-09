#include "ewa_base/serialization/serializer.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/codecvt.h"

EW_ENTER



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





EW_LEAVE
