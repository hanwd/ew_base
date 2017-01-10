#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/codecvt.h"
#include "ewa_base/basic/file.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/basic/lookuptable.h"
#include "ewa_base/logging/logger.h"

EW_ENTER


template<typename T>
StringBuffer<T>::StringBuffer(const T* p1)
{
	basetype::assign(p1,std::char_traits<T>::length(p1));
}

template<typename T>
StringBuffer<T>& StringBuffer<T>::operator=(const T* p1)
{
	basetype::assign(p1,std::char_traits<T>::length(p1));
	return *this;
}

template<typename T,unsigned N>
class StringBufferHelperN
{
public:
	static void equal(StringBuffer<T>& h,const String& o)
	{
		IConv::utf8_to_unicode(h,o.c_str(),o.size());
	}

	static void eqadd(StringBuffer<T>& h,const String& o)
	{
		StringBuffer<T> tmp;
		IConv::utf8_to_unicode(tmp,o.c_str(),o.size());
		h+=tmp;
	}
};


template<typename T>
class StringBufferHelperN<T,1>
{
public:
	static void equal(StringBuffer<T>& h,const String& o)
	{
		h.assign((T*)o.c_str(),o.size());
	}

	static void eqadd(StringBuffer<T>& h,const String& o)
	{
		h.append((T*)o.c_str(),o.size());
	}
};



template<typename T>
class StringBufferHelper : public StringBufferHelperN<T,sizeof(T)>
{
public:

};


template<typename T>
StringBuffer<T>& StringBuffer<T>::operator=(const String& o)
{
	StringBufferHelper<T>::equal((*this),o);
	return *this;
}

template<typename T>
StringBuffer<T>& StringBuffer<T>::operator+=(const String& o)
{
	StringBufferHelper<T>::eqadd((*this),o);
	return *this;
}

//TEXTFILE BOM
//UTF8 EF BB BF
//UTF-16 (BE)	FE FF
//UTF-16 (LE)	FF FE
//UTF-32 (BE)	00 00 FE FF
//UTF-32 (LE)	FF FE 00 00

static const unsigned char bom_utf8[]= {0xEF,0xBB,0xBF};
static const unsigned char bom_utf16_le[]= {0xFF,0xFE};
static const unsigned char bom_utf16_be[]= {0xFE,0xFF};
static const unsigned char bom_utf32_le[]= {0xFF,0xFE,0x00,0x00};
static const unsigned char bom_utf32_be[]= {0x00,0x00,0xFE,0xFF};

template<typename T>
bool StringBuffer<T>::save(const String& file,int type)
{
	bool writebom=(type&FILE_TYPE_TEXT_BOM)!=0;
	type=type&FILE_TYPE_MASK;

	File ofs;

	if(!ofs.open(file,FLAG_FILE_WC|FLAG_FILE_TRUNCATE))
	{
		return false;
	}

	//ofs.truncate(0);

	switch(type)
	{
	case FILE_TYPE_BINARY:
	{
		ofs.write((char*)data(),sizeof(T)*size());
		return ofs.good();
	}
	case FILE_TYPE_TEXT_ANSI:
	{

		StringBuffer<char> sb;
		if(sizeof(T)==1)
		{
			if(!IConv::utf8_to_ansi(sb,(const char*)data(),size()))
			{
				return false;
			}
		}
		else if(sizeof(T)==2)
		{
			if(!IConv::unicode_to_ansi(sb,(uint16_t*)data(),size()))
			{
				return false;
			}
		}
		else if(sizeof(T)==4)
		{
			if(!IConv::unicode_to_ansi(sb,(uint32_t*)data(),size()))
			{
				return false;
			}
		}

		ofs.write((char*)sb.data(),sb.size());
		return ofs.good();
	}
	break;
	case FILE_TYPE_TEXT:
	case FILE_TYPE_TEXT_UTF8:
	{

		if(writebom)
		{
			ofs.write((char*)bom_utf8,3);
		}

		if(sizeof(T)==1)
		{
			ofs.write((char*)data(),size());
			return ofs.good();
		}

		StringBuffer<char> sb;
		if(sizeof(T)==2)
		{
			if(!IConv::unicode_to_utf8(sb,(uint16_t*)data(),size()))
			{
				return false;
			}
		}
		else if(sizeof(T)==4)
		{
			if(!IConv::unicode_to_utf8(sb,(uint32_t*)data(),size()))
			{
				return false;
			}
		}

		ofs.write(sb.data(),sb.size());
		return ofs.good();
	}
	break;
	case FILE_TYPE_TEXT_UTF16_BE:
	case FILE_TYPE_TEXT_UTF16_LE:
	{

		StringBuffer<unsigned short> wb;
		if(sizeof(T)==1)
		{
			if(!IConv::utf8_to_unicode(wb,(const char*)data(),size()))
			{
				return false;
			}
		}
		else if(sizeof(T)==2)
		{
			wb.append((unsigned short*)data(),size());
		}
		else
		{
			return false;
		}


		if(type==FILE_TYPE_TEXT_UTF16_BE)
		{
			char* p=(char*)wb.data();
			for(size_t i=0; i<wb.size(); i++)
			{
				std::swap(p[2*i+0],p[2*i+1]);
			}
		}

		if(writebom)
		{
			ofs.write((char*)(type==FILE_TYPE_TEXT_UTF16_BE?bom_utf16_be:bom_utf16_le),2);
		}

		ofs.write((char*)wb.data(),wb.size()*2);
		return ofs.good();

	}
	break;
	default:
		System::SetLastError("Invalid type in StringBuffer<T>::save");
		return false;
	};

	return true;
}


template<typename T>
bool StringBuffer<T>::load(const String& file,int type)
{

	File ifs;

	type=type&FILE_TYPE_MASK;

	if(!ifs.open(file))
	{
		return false;
	}
	size_t sz=ifs.size();

	if(type==FILE_TYPE_BINARY)
	{
		if(sz%sizeof(T)!=0)
		{
			System::SetLastError("Invalid filesize");
			return false;
		}

		size_t size=sz/sizeof(T);
		resize(size);
		ifs.read((char*)data(),sz);

		return true;

	}


	unsigned char bom[4]= {1,1,1,1};
	ifs.read((char*)bom,4);

	StringBuffer<char> sb;
	if(bom[0]==0xEF && bom[1]==0xBB && bom[2]==0xBF) // UTF8
	{
		size_type df=3;
		ifs.seek(df,SEEKTYPE_BEG);
		size_type nz=sz-df;
		StringBuffer<char> kb;
		kb.resize(nz);
		ifs.read((char*)kb.data(),nz);

		if(sizeof(T)==1)
		{
			sb.swap(*(StringBuffer<char>*)this);
			return true;
		}
		else if(sizeof(T)==2)
		{
			return IConv::utf8_to_unicode(*(StringBuffer<uint16_t>*)this,kb.data(),nz);
		}
		if(sizeof(T)==4)
		{
			return IConv::utf8_to_unicode(*(StringBuffer<uint32_t>*)this,kb.data(),nz);
		}
		else
		{
			System::SetLastError("Unkown Type for StringBuffer");
			return false;
		}

	}
	else if( (bom[0]==0xFE && bom[1]==0xFF)||(bom[0]==0xFF && bom[1]==0xFE))
	{
		if((sz&0x1)!=0)
		{
			System::SetLastError("Invalid UTF-16 Filesize");
			return false;
		}

		size_type df=2;
		ifs.seek(df,SEEKTYPE_BEG);
		size_type nz=sz-df;
		StringBuffer<unsigned short> kb;
		kb.resize(nz>>1);
		ifs.read((char*)kb.data(),nz);

		// CE D2
		// 11 62;
		// BD 70

		//unsigned short tag=*(unsigned short*)bom;
		unsigned short tag=*reinterpret_cast<unsigned short*>(bom);
		if(tag==0xFFFE) //BE
		{
			char *pc=(char*)kb.data();
			for(size_type i=0; i<nz; i+=2)
			{
				std::swap(pc[i],pc[i+1]);
			}
		}

		if(sizeof(T)==2)
		{
			kb.swap(*(StringBuffer<unsigned short>*)this);
			return true;
		}

		if(!IConv::unicode_to_utf8(sb,kb.data(),kb.size()))
		{
			return false;
		}

	}
	else if( (bom[0]==0xFF && bom[1]==0xFE && bom[2]==0 && bom[3]==0)||(bom[0]==0 && bom[1]==0 && bom[2]==0xFE && bom[3]==0xFF))
	{
		if((sz&0x3)!=0)
		{
			System::SetLastError("Invalid UTF-32 Filesize");
			return false;
		}

		size_type df=4;
		ifs.seek(df,SEEKTYPE_BEG);
		size_type nz=sz-df;
		StringBuffer<unsigned int> kb;
		kb.resize(nz>>2);
		ifs.read((char*)kb.data(),nz);

		uint32_t tag=*(uint32_t*)bom;

		if(tag==0xFFFE0000) //BE
		{
			char* pc=(char*)kb.data();
			for(size_type i=0; i<nz; i+=4)
			{
				std::swap(pc[i+0],pc[i+3]);
				std::swap(pc[i+1],pc[i+2]);
			}
		}

		if(sizeof(T)==4)
		{
			kb.swap(*(StringBuffer<unsigned int>*)this);
			return true;
		}

		if(!IConv::unicode_to_utf8(sb,kb.data(),kb.size()))
		{
			return false;
		}
	}
	else
	{
		size_type df=0;
		ifs.seek(df,SEEKTYPE_BEG);
		size_type nz=sz-df;
		sb.resize(nz);
		ifs.read((char*)sb.data(),nz);

		unsigned char* p=(unsigned char*)sb.data();
		int t=0;
		int n=0;

		for(size_type i=0; i<nz; i++)
		{
			unsigned c=p[i];
			if(c<0x80) continue;
			for(n=0; ((c<<=1)&0x80)>0; n++);
			if(n>3)
			{
				t=-1;
				break;
			}

			if(i+n>=nz)
			{
				t=-1;
				break;
			}

			t=1;
			for(int j=1; j<=n; j++)
			{
				if(((p[i+j])&0xC0)!=0x80)
				{
					t=-1;
					break;
				}
			}
			i+=n;
		}

		if(t==-1)
		{
			StringBuffer<char> kb;
			if(!IConv::gbk_to_utf8(kb,sb.data(),sb.size()))
			{
				return false;
			}
			kb.swap(sb);
		}
	}

	if(sizeof(T)==1)
	{
		sb.swap(*(StringBuffer<char>*)this);
	}
	else if(sizeof(T)==2)
	{
		return IConv::utf8_to_unicode(*(StringBuffer<uint16_t>*)this,sb.data(),sb.size());
	}
	else if(sizeof(T)==4)
	{
		return IConv::utf8_to_unicode(*(StringBuffer<uint32_t>*)this,sb.data(),sb.size());
	}
	return true;

}


template<typename T>
T* StringBuffer<T>::c_str()
{
	if(this->empty())
	{
		this->reserve(1);
	}

	*this->end()=T();
	return data();
}



template<typename T>
const T* StringBuffer<T>::c_str() const
{
	if(this->empty())
	{
		return (const T*)const_empty_buffer;
	}

	*(T*)&(*this->end())=T();
	return data();
}

/*
template<typename T>
StringBuffer<T>& StringBuffer<T>::operator<<(bool v)
{
	if(v)
	{
		basetype::append("true",4);
	}
	else
	{
		basetype::append("false",5);
	}

	return (*this);
}

template<typename T>
StringBuffer<T>& StringBuffer<T>::operator<<(char v)
{
	basetype::append(&v,1);
	return (*this);
}
template<typename T>
StringBuffer<T>& StringBuffer<T>::operator<<(int32_t v)
{
	_do_format_integer(v);
	return (*this);
}
template<typename T>
StringBuffer<T>& StringBuffer<T>::operator<<(int64_t v)
{
	_do_format_integer(v);
	return (*this);
}
template<typename T>
StringBuffer<T>& StringBuffer<T>::operator<<(uint32_t v)
{
	_do_format_integer(v);
	return (*this);
}
template<typename T>
StringBuffer<T>& StringBuffer<T>::operator<<(uint64_t v)
{
	_do_format_integer(v);
	return (*this);
}
template<typename T>
StringBuffer<T>& StringBuffer<T>::operator<<(float v)
{
	(*this)+=String::Format("%f",v);
	return (*this);
}

template<typename T>
StringBuffer<T>& StringBuffer<T>::operator<<(const void* v)
{
	(*this)+=String::Format("%p",v);
	return (*this);
}

template<typename T>
StringBuffer<T>& StringBuffer<T>::operator<<(double v)
{
	(*this)+=String::Format("%g",v);
	return (*this);
}

template<typename T>
StringBuffer<T>& StringBuffer<T>::operator<<(const String& v)
{
	(*this)+=v;
	return *this;
}

template<typename T>
StringBuffer<T>& StringBuffer<T>::operator<<(const T* p)
{
	basetype::append(p,std::char_traits<T>::length(p));
	return (*this);
}

template<typename T>
StringBuffer<T>& StringBuffer<T>::operator<<(const StringBuffer& v)
{
	basetype::append(v.data(),v.size());
	return (*this);
}
*/

template<typename T>
bool StringBuffer<T>::enlarge_size_by(size_t sz)
{
	return impl.enlarge_size_by(sz);
}


template class StringBuffer<char>;
template class StringBuffer<unsigned char>;
template class StringBuffer<short>;
template class StringBuffer<unsigned short>;
template class StringBuffer<int>;
template class StringBuffer<unsigned int>;

template class StringBuffer<wchar_t>;

EW_LEAVE
