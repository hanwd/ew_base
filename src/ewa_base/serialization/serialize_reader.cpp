#include "ewa_base/serialization/serializer.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/codecvt.h"
#include "ewa_base/scripting/variant.h"
#include "ewa_base/scripting/callable_table.h"

EW_ENTER


void SerializerReader::load(Variant& val_)
{
	arr_1t<Variant> val;
	load(val);

	if (!val.empty())
	{
		val_ = val[0];
	}
	else
	{
		val_.clear();
	}
}

void SerializerReader::load(arr_1t<Variant>& val)
{

	handle_head();

	if (filetype == "ew_array")
	{
		(*this) & val;
	}
	else if (filetype == "ew_table")
	{
		val.resize(1);
		(*this) & val[0].ref<VariantTable>();
	}
	else if (filetype == "ew_var")
	{
		val.resize(1);
		(*this) & val[0];
	}
	else
	{
		errstr("invalid filetype");
	}

	handle_tail();
}

void SerializerReader::load(VariantTable& val)
{
	handle_head();

	if (filetype == "ew_array")
	{
		arr_1t<Variant> var;
		(*this) & var;
		handle_tail();

		val.clear();

		if (var.size() == 1 && var[0].kptr() && var[0].kptr()->ToTable())
		{
			val = var[0].kptr()->ToTable()->value;
		}
		else
		{
			for (size_t i = 0; i < var.size(); i++)
			{
				val[String::Format("value_%d", i)] = var[i];
			}
		}


	}
	else if (filetype == "ew_table")
	{
		(*this) & val;
		handle_tail();
	}
	else if (filetype == "ew_var")
	{
		Variant var;
		(*this) & var;
		handle_tail();

		if (var.kptr() && var.kptr()->ToTable())
		{
			val = var.kptr()->ToTable()->value;
		}
		else
		{
			val.clear();
			val["value_0"] = var;
		}

	}
	else
	{
		errstr("invalid filetype");
	}


}


SerializerReader& SerializerReader::handle_head(const char* p)
{
	serial_header header;
	recv_checked((char*)&header,sizeof(header));
	if(!header.check())
	{
		errstr("invalid header");
	}

	if (p)
	{
		int n = ::strlen(p);
		if (n > 8) n = 8;
		if (::memcmp(header.tags, p, n) != 0)
		{
			errstr("invalid filetype");
		}
	}

	filetype.assign(header.tags, 8);

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
	cached_objects().clear();
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

ObjectData* SerializerReader::read_object(int val)
{
	if(!flags.get(FLAG_OFFSET_TABLE)) return NULL;
	return cached_objects().read_object(*this,val);
}


EW_LEAVE
