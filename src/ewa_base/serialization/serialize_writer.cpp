#include "ewa_base/serialization/serializer.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/codecvt.h"

EW_ENTER


void SerializerWriter::save(const Variant& val)
{
	handle_head("ew_var");
	(*this) & val;
	handle_tail();
}

void SerializerWriter::save(const arr_1t<Variant>& val)
{
	handle_head("ew_array");
	(*this) & val;
	handle_tail();
}

void SerializerWriter::save(const VariantTable& val)
{
	handle_head("ew_table");
	(*this) & val;
	handle_tail();
}


SerializerWriter& SerializerWriter::handle_head(const char* p)
{
	serial_header header;
	if (p)
	{
		int n = ::strlen(p);
		if (n > 8) n = 8;
		::memcpy(header.tags, p, n);
	}
	else
	{
		::memcpy(header.tags, "ew_files", 8);
	}

	filetype.assign(header.tags, 8);

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
		cached_objects().clear();
		return *this;
	}

	serial_header header;
	header.flags=flags.val()&~((1<<8)-1);
	header.gver=global_version();

	header.offset=tellp();
	if(header.offset<0)
	{
		cached_objects().clear();
		return *this;
	}

	send_checked((char*)cached_objects().aOffset.data(),sizeof(CachedObjectManager::PtrOffset)*cached_objects().aOffset.size());
	header.size=cached_objects().aOffset.size();

	if(seekp(0,SEEKTYPE_BEG)==0)
	{
		header.update();
		send_checked((char*)&header,sizeof(header));
	}

	cached_objects().clear();
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
