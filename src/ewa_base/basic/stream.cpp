#include "ewa_base/basic/stream.h"

EW_ENTER


int32_t StreamData::Read(char*,size_t)
{
	return -1;
}

int32_t StreamData::Write(const char*,size_t)
{
	return -1;
}

void StreamData::Flush()
{

}

void StreamData::Close()
{
	flags.del(FLAG_FAIL_BITS);
}

int64_t StreamData::Size()
{
	return -1;
}

int64_t StreamData::Seek(int64_t pos,int t)
{
	return -1;
}

int64_t StreamData::Tell()
{
	return Seek(0,SEEKTYPE_CUR);
}

void StreamData::Rewind()
{
	Seek(0,SEEKTYPE_BEG);
}

Stream::Stream()
{

}

Stream::Stream(StreamData* p):basetype(p)
{

}

int32_t Stream::Read(char* buf,size_t len)
{
	if(!m_refData) return -1;
	return m_refData->Read(buf,len);
}

int32_t Stream::Write(const char* buf,size_t len)
{
	if(!m_refData) return -1;
	int writed=0;
	int rc;

	while(len>0)
	{
		rc=m_refData->Write(buf,len);
		if(rc>0)
		{
			buf+=rc;
			len-=rc;
			writed+=rc;
		}
		else if(rc==0)
		{
			return writed;
		}
		else
		{
			return -1;
		}
	};

	return writed;

}

void Stream::Flush()
{
	if(!m_refData) return;
	m_refData->Flush();
}

void Stream::Close()
{
	m_refData.reset(NULL);
}

int64_t Stream::Size()
{
	if(!m_refData) return -1;
	return m_refData->Size();
}

int64_t Stream::Seek(int64_t pos,int t)
{
	if(!m_refData) return -1;
	return m_refData->Seek(pos,t);
}

int64_t Stream::Tell()
{
	if(!m_refData) return -1;
	return m_refData->Tell();
}

bool Stream::Good()
{
	return m_refData && !m_refData->flags.get(StreamData::FLAG_FAIL_BITS);
}

void Stream::Rewind()
{
	if(!m_refData) return;
	m_refData->Rewind();
}

StreamDataHandle::StreamDataHandle(const StreamDataHandle& o)
{
	m_pHandle=KO_Policy_handle::duplicate(o.m_pHandle);
}

StreamDataHandle& StreamDataHandle::operator=(const StreamDataHandle& o)
{
	if(this==&o) return *this;
	Close();
	m_pHandle=KO_Policy_handle::duplicate(o.m_pHandle);
	return *this;
}

StreamDataHandle::StreamDataHandle()
{
	m_pHandle=KO_Policy_handle::invalid_value();
}

StreamDataHandle::~StreamDataHandle()
{
	Close();
}

StreamDataHandle::StreamDataHandle(KO_Policy_handle::type v):m_pHandle(v)
{

}

KO_Handle<KO_Policy_handle>::type StreamDataHandle::native_handle()
{
	return m_pHandle;
}

void StreamDataHandle::native_handle(KO_Policy_handle::type v)
{
	if(m_pHandle==v) return;
	if(m_pHandle!=KO_Policy_handle::invalid_value())
	{
		KO_Policy_handle::destroy(m_pHandle);
	}
	m_pHandle=v;
}

void StreamDataHandle::Close()
{
	if(m_pHandle!=KO_Policy_handle::invalid_value())
	{
		KO_Policy_handle::destroy(m_pHandle);
		m_pHandle=KO_Policy_handle::invalid_value();	
	}
}

#ifdef EW_WINDOWS


void StreamDataHandle::Flush()
{
	if(!::FlushFileBuffers(m_pHandle))
	{
		System::CheckError("File::Flush Error");
	}
}

int32_t StreamDataHandle::Read(char* buf,size_t len)
{
	if(flags.get(FLAG_READ_FAIL_BIT)) return -1;

	DWORD nRead(0);
	if(::ReadFile(m_pHandle,buf,len,&nRead,NULL)==FALSE)
	{
		flags.add(FLAG_READ_FAIL_BIT);
		System::CheckError("File::Read Error");
		return -1;
	}
	return nRead;
}

int32_t StreamDataHandle::Write(const char* buf,size_t len)
{
	if(flags.get(FLAG_WRITE_FAIL_BIT)) return -1;

	DWORD nWrite(0);
	if(::WriteFile(m_pHandle,buf,len,&nWrite,NULL)==FALSE)
	{
		flags.add(FLAG_WRITE_FAIL_BIT);
		System::CheckError("File::Write Error");
		return -1;
	}
	return nWrite;
}

#else

void StreamDataHandle::Flush()
{
	::fsync(impl);
}

int32_t StreamDataHandle::Read(char* buf,size_t len)
{
	if(flags.get(FLAG_READ_FAIL_BIT)) return -1;

	int nLen= ::read(m_pHandle,buf,len);
	if(nLen<0)
	{
		flags.add(FLAG_READ_FAIL_BIT);
		System::CheckError("File::Read Error");
	}
	return nLen;
}

int32_t StreamDataHandle::Write(const char* buf,size_t len)
{
	if(flags.get(FLAG_WRITE_FAIL_BIT)) return -1;

	int nLen=::write(m_pHandle,buf,len);
	if(nLen<0)
	{
		flags.add(FLAG_WRITE_FAIL_BIT);
		System::CheckError("File::Write Error");
	}
	return nLen;
}

#endif


class StreamDataFile : public StreamDataHandle
{
public:

	virtual int64_t Size();
	virtual int64_t Seek(int64_t pos,int t);
};



#ifdef EW_WINDOWS

String fn_encode(const String& oldname_)
{
	return oldname_;
}


bool Stream::SaveToBuffer(StringBuffer<char>& sb)
{
	if(!Good()) return false;

	char buffer[1024*32];
	while(1)
	{
		int rc=Read(buffer,sizeof(buffer));
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

	return false;
}

bool Stream::LoadFromBuffer(StringBuffer<char>& sb)
{
	if(!Good()) return false;
	Write(sb.data(),sb.size());
	return true;
}


bool Stream::LoadFromFile(const String& filename_)
{
	if(!Good()) return false;

	File file;
	if(!file.Open(filename_,FileAccess::FLAG_RD))
	{
		return false;
	}

	char buffer[1024*32];
	while(1)
	{
		int rc=file.Read(buffer,sizeof(buffer));
		if(rc>0)
		{
			Write(buffer,rc);
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

bool Stream::SaveToFile(const String& filename_)
{
	if(!Good()) return false;

	File file;
	if(!file.Open(filename_,FileAccess::FLAG_WC|FileAccess::FLAG_TRUNCATE))
	{
		return false;
	}

	char buffer[1024*32];
	while(1)
	{
		int rc=Read(buffer,sizeof(buffer));
		if(rc>0)
		{
			file.Write(buffer,rc);
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

bool Stream::Open(const String& filename_,int flag_)
{
	String fn=fn_encode(filename_);

	StreamDataFile* streamfile=new StreamDataFile;
	SetData(streamfile);

	File file;

	if(!file.Open(filename_,flag_))
	{
		streamfile->flags.add(StreamData::FLAG_FAIL_BITS);
		return false;
	}

	File::impl_type& impl(reinterpret_cast<File::impl_type&>(file));
	streamfile->native_handle(impl.release());

	return true;
}

int64_t StreamDataFile::Size()
{
	FileAccess::LargeInteger tmp;
	tmp.d[0]=::GetFileSize(m_pHandle,&tmp.d[1]);
	return tmp.dval;
}

int64_t StreamDataFile::Seek(int64_t pos,int t)
{
	LARGE_INTEGER li;
	li.QuadPart=pos;
	li.LowPart = SetFilePointer (m_pHandle,
								 li.LowPart,
								 &li.HighPart,
								 t);

	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
		System::CheckError("File::Seek Error");
		return -1;
	}

	return li.QuadPart;
}

#else


String fn_encode(const String& oldname_)
{
	return oldname_;
}

static int shm_fileflag(int flag_)
{
	int acc=0;

	if(flag_&FileAccess::FLAG_WR)
	{
		if(flag_&FileAccess::FLAG_RD)
		{
			acc|=O_RDWR;
		}
		else
		{
			acc|=O_WRONLY;
		}
	}

	return acc;
}

bool Stream::Open(const String& filename_,int flag_)
{
	String filename=fn_encode(filename_);

	StreamDataFile* streamfile=new StreamDataFile;
	m_refData.reset(streamfile);

	int fd=::open(filename.c_str(),shm_fileflag(flag_),0777);
	if(fd<0)
	{
		if((flag_&FileAccess::FLAG_CR)!=0)
		{
			fd=::open(filename.c_str(),shm_fileflag(flag_)|O_CREAT,0777);
		}
		if(fd<0)
		{
			System::CheckError("File::Open Error");
			return false;
		}
	}
	if(flag_&FileAccess::FLAG_APPEND)
	{
		streamfile->Seek(0,FILEPOS_END);
	}
	streamfile->native_handle(fd);
	return true;
}

int64_t StreamDataFile::Size()
{
	struct stat statbuf;
	if(fstat(m_pHandle,&statbuf)<0)
	{
		System::CheckError("File::Size Error");
		return -1;
	}

	return statbuf.st_size;
}


int64_t StreamDataFile::Seek(int64_t pos,int t)
{
	return lseek(m_pHandle,pos,t);
}


#endif



EW_LEAVE
