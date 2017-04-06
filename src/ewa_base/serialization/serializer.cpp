#include "ewa_base/serialization/serializer.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/codecvt.h"

EW_ENTER

SerializerHelper::SerializerHelper(Serializer& ar) :m_ar(ar), ver(-1)
{

}

SerializerHelper::SerializerHelper(SerializerHelper& sh) : m_ar(sh.m_ar), ver(-1)
{

}


Serializer& SerializerHelper::ref(int v)
{
	EW_ASSERT(ver == -1);
	ver = m_ar.local_version(v);
	return m_ar;
}

serial_header::serial_header()
{
	offset = 0;
	size = 0;
}
void serial_header::update()
{ 
	chksum = (int32_t)crc32(this, (char*)&chksum - (char*)this); 
}

bool serial_header::check()
{ 
	return chksum == (int32_t)crc32(this, (char*)&chksum - (char*)this); 
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
	return static_cast<SerializerWriter&>(*this);
}

SerializerReader& Serializer::reader()
{
	if(!is_reader())
	{
		errstr("NOT_READER");
	}
	return static_cast<SerializerReader&>(*this);
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
	,gver(0)
{

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
