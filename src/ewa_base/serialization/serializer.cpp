#include "ewa_base/serialization/serializer.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/codecvt.h"

EW_ENTER


template<typename A>
void serial_helper_func<A,String>::g(SerializerReader &ar,type &val)
{
	int n=ar.size_count(val.size());

	arr_1t<char> vect;
	vect.resize(n);

	ar.checked_recv(vect.data(),n);

	if(ar.flags.get(Serializer::STRCVT_UTF8))
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

	if(ar.flags.get(Serializer::STRCVT_UTF8))
	{
		StringBuffer<char> sb;
		if(!IConv::ansi_to_utf8(sb,val.c_str(),val.size()))
		{
			ar.errstr("invalid string");
			return;
		}
		p=sb.data();
		n=ar.size_count(sb.size());
	}
	else
	{
		p=val.c_str();
		n=ar.size_count(val.size());
	}

	if(n>0)
	{
		ar.checked_send((char *)p,n);
	}

}

template class DLLIMPEXP_EWA_BASE serial_helper_func<SerializerWriter,String>;
template class DLLIMPEXP_EWA_BASE serial_helper_func<SerializerReader,String>;

Serializer::Serializer(int t)
	:type(t)
{
	gver=0;
}


SerializerEx::SerializerEx(int t)
	:Serializer(t)
{

}

serializer_cached_objects::serializer_cached_objects()
{
	m_aObjects.insert(NULL);
}

serializer_cached_objects::~serializer_cached_objects()
{

}

//int32_t serializer_cached_objects::get(Object* dat)
//{
//	return get((void*)dat);
//}
//
//int32_t serializer_cached_objects::put(Object* dat)
//{
//	return put((void*)dat);
//}

void *serializer_cached_objects::get(int32_t idx)
{
	if(idx<0||idx>=(int)m_aObjects.size())
	{
		return NULL;
	}

	return m_aObjects.get(idx);
}

int32_t serializer_cached_objects::get(void *dat_)
{
	return m_aObjects.find1(dat_);
}

int32_t serializer_cached_objects::put(void *dat_)
{
	return m_aObjects.insert(dat_);
}

void serializer_cached_objects::clear()
{
	m_aObjects.clear();
	m_aObjects.insert(NULL);
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

EW_LEAVE
