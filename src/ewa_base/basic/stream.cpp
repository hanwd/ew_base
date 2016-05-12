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
	return Seek(0,SEEKPOS_CUR);
}

void StreamData::Rewind()
{
	Seek(0,SEEKPOS_BEG);
}

Stream::Stream()
{
	m_refData.reset(new StreamData);
}

Stream::Stream(StreamData* streamdata)
{
	m_refData.reset(streamdata?streamdata:new StreamData);
}

int32_t Stream::Read(char* buf,size_t len)
{
	return m_refData->Read(buf,len);
}

int32_t Stream::Write(const char* buf,size_t len)
{
	return m_refData->Write(buf,len);
}

void Stream::Flush()
{
	m_refData->Flush();
}

void Stream::Close()
{
	m_refData->Close();
}

int64_t Stream::Size()
{
	return m_refData->Size();
}

int64_t Stream::Seek(int64_t pos,int t)
{
	return m_refData->Seek(pos,t);
}

int64_t Stream::Tell()
{
	return m_refData->Tell();
}

bool Stream::Good()
{
	return !m_refData->flags.get(StreamData::FLAG_FAILBIT);
}

void Stream::Rewind()
{
	m_refData->Rewind();
}

void Stream::swap(Stream& other)
{
	m_refData.swap(other.m_refData);
}



StreamDataHandle::StreamDataHandle()
{

}

StreamDataHandle::StreamDataHandle(KO_Handle<KO_Policy_handle> impl):m_pHandle(impl)
{

}

KO_Handle<KO_Policy_handle>::type StreamDataHandle::native_handle()
{
	return m_pHandle;
}

void StreamDataHandle::native_handle(KO_Handle<KO_Policy_handle>::const_reference v)
{
	m_pHandle.reset(v);
}

void StreamDataHandle::Close()
{
	m_pHandle.close();
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
	DWORD nRead(0);
	if(::ReadFile(m_pHandle,buf,len,&nRead,NULL)==FALSE)
	{
		flags.add(FLAG_FAILBIT);
		System::CheckError("File::Read Error");
		return -1;
	}
	return nRead;
}

int32_t StreamDataHandle::Write(const char* buf,size_t len)
{
	DWORD nWrite(0);
	if(::WriteFile(m_pHandle,buf,len,&nWrite,NULL)==FALSE)
	{
		flags.add(FLAG_FAILBIT);
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
	int nLen= ::read(m_pHandle,buf,len);
	if(nLen<0)
	{
		System::CheckError("File::Read Error");
	}
	return nLen;
}

int32_t StreamDataHandle::Write(const char* buf,size_t len)
{
	int nLen=::write(m_pHandle,buf,len);
	if(nLen<0)
	{
		System::CheckError("File::Write Error");
	}
	return nLen;
}

#endif



EW_LEAVE
