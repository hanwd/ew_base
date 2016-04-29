#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/codecvt.h"
#include "ewa_base/basic/file.h"
#include "ewa_base/logging/logger.h"

EW_ENTER

template<typename T>
template<typename G>
inline void StringBuffer<T>::_do_format_integer(G v_)
{
	T buf[32];
	T* p2=buf+31;
	T* p1=StringDetail::str_format(p2,v_);
	basetype::append(p1,p2-p1);
}

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
		IConv::ansi_to_unicode(h,o.c_str(),o.size());
	}

	static void eqadd(StringBuffer<T>& h,const String& o)
	{
		StringBuffer<T> tmp;
		IConv::ansi_to_unicode(tmp,o.c_str(),o.size());
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
	bool writebom=(type&FILE_TEXT_BOM)!=0;
	type=type&FILE_MASK;

	File ofs;

	if(!ofs.Open(file,FileAccess::FLAG_WR|FileAccess::FLAG_CR))
	{
		return false;
	}

	ofs.Truncate(0);

	switch(type)
	{
	case FILE_BINARY:
	{
		ofs.Write((char*)data(),sizeof(T)*size());
		return ofs.Good();
	}
	case FILE_TEXT_ANSI:
	{
		if(sizeof(T)==1)
		{
			ofs.Write((char*)data(),size());
			return ofs.Good();
		}
		StringBuffer<char> sb;
		if(!IConv::unicode_to_ansi(sb,data(),size()))
		{
			return false;
		}
		ofs.Write((char*)sb.data(),sb.size());
		return ofs.Good();
	}
	break;
	case FILE_TEXT:
	case FILE_TEXT_UTF8:
	{
#ifndef EW_WINDOWS
		if(sizeof(T)==1)
		{
			ofs.Write((char*)data(),size());
			return ofs.Good();
		}
#endif
		StringBuffer<char> sb;
		if(sizeof(T)==1)
		{
			if(!IConv::ansi_to_utf8(sb,(const char*)data(),size()))
			{
				return false;
			}
		}
		else if(sizeof(T)==sizeof(wchar_t))
		{
			if(!IConv::unicode_to_utf8(sb,data(),size()))
			{
				return false;
			}
		}

		if(writebom)
		{
			ofs.Write((char*)bom_utf8,3);
		}

		ofs.Write(sb.data(),sb.size());
		return ofs.Good();
	}
	break;
	case FILE_TEXT_UTF16_BE:
	case FILE_TEXT_UTF16_LE:
	{

		StringBuffer<unsigned short> wb;
		if(sizeof(T)==1)
		{
			if(!IConv::ansi_to_unicode(wb,(const char*)data(),size()))
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


		if(type==FILE_TEXT_UTF16_BE)
		{
			char* p=(char*)wb.data();
			for(size_t i=0; i<wb.size(); i++)
			{
				std::swap(p[2*i+0],p[2*i+1]);
			}
		}

		if(writebom)
		{
			ofs.Write((char*)(type==FILE_TEXT_UTF16_BE?bom_utf16_be:bom_utf16_le),2);
		}

		ofs.Write((char*)wb.data(),wb.size()*2);
		return ofs.Good();

	}
	break;
	default:

		this_logger().LogError("Invalid type in StringBuffer<T>::save");
		return false;
	};

	return true;
}


template<typename T>
bool StringBuffer<T>::load(const String& file,int type)
{

	File ifs;

	type=type&FILE_MASK;

	if(!ifs.Open(file))
	{
		this_logger().LogError("Cannot open file: %s",file);
		return false;
	}
	size_t sz=ifs.Size();

	if(type==FILE_BINARY)
	{
		if(sz%sizeof(T)!=0)
		{
			this_logger().LogError("Invalid filesize: %u",sz);
			return false;
		}

		size_t size=sz/sizeof(T);
		resize(size);
		ifs.Read((char*)data(),sz);

		return true;

	}


	unsigned char bom[4]= {1,1,1,1};
	ifs.Read((char*)bom,4);

	StringBuffer<char> sb;
	if(bom[0]==0xEF && bom[1]==0xBB && bom[2]==0xBF) // UTF8
	{
		size_type df=3;
		ifs.Seek(df,File::FILEPOS_BEG);
		size_type nz=sz-df;
		StringBuffer<char> kb;
		kb.resize(nz);
		ifs.Read((char*)kb.data(),nz);

		if(sizeof(T)==1)
		{
			if(!IConv::utf8_to_ansi(sb,kb.data(),nz))
			{
				return false;
			}
			sb.swap(*(StringBuffer<char>*)this);
			return true;
		}
		else
		{
			if(!IConv::utf8_to_unicode((*this),kb.data(),nz))
			{
				return false;
			}
			return true;
		}

	}
	else if( (bom[0]==0xFE && bom[1]==0xFF)||(bom[0]==0xFF && bom[1]==0xFE))
	{
		if((sz&0x1)!=0)
		{
			System::LogTrace("Invalid UTF-16 filesize: %u",sz);
			return false;
		}

		size_type df=2;
		ifs.Seek(df,File::FILEPOS_BEG);
		size_type nz=sz-df;
		StringBuffer<unsigned short> kb;
		kb.resize(nz>>1);
		ifs.Read((char*)kb.data(),nz);

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

		if(!IConv::unicode_to_ansi(sb,kb.data(),kb.size()))
		{
			return false;
		}

	}
	else if( (bom[0]==0xFF && bom[1]==0xFE && bom[2]==0 && bom[3]==0)||(bom[0]==0 && bom[1]==0 && bom[2]==0xFE && bom[3]==0xFF))
	{
		if((sz&0x3)!=0)
		{
			System::LogTrace("Invalid UTF-32 filesize:%u",sz);
			return false;
		}

		size_type df=4;
		ifs.Seek(df,File::FILEPOS_BEG);
		size_type nz=sz-df;
		StringBuffer<unsigned int> kb;
		kb.resize(nz>>2);
		ifs.Read((char*)kb.data(),nz);

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

		if(!IConv::unicode_to_ansi(sb,kb.data(),kb.size()))
		{
			return false;
		}
	}
	else
	{
		size_type df=0;
		ifs.Seek(df,File::FILEPOS_BEG);
		size_type nz=sz-df;
		sb.resize(nz);
		ifs.Read((char*)sb.data(),nz);

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

		if(t==1)
		{
			StringBuffer<char> kb;
			if(!IConv::utf8_to_ansi(kb,sb.data(),sb.size()))
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
	else
	{
		if(!IConv::ansi_to_unicode(*this,sb.data(),sb.size()))
		{
			return false;
		}
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


template class StringBuffer<char>;
template class StringBuffer<unsigned char>;
template class StringBuffer<short>;
template class StringBuffer<unsigned short>;
template class StringBuffer<int>;
template class StringBuffer<unsigned int>;

template class StringBuffer<wchar_t>;

EW_LEAVE
