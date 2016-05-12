#include "ewa_base/basic/file.h"
#include <cstdio>

#ifdef EW_WINDOWS
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <cerrno>
#include <unistd.h>
#endif

EW_ENTER


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

bool Stream::Open(const String& filename_,int flag_)
{
	String fn=fn_encode(filename_);

	StreamDataFile* streamfile=new StreamDataFile;
	m_refData.reset(streamfile);

	HANDLE hFile=(HANDLE)CreateFileA(
					 fn.c_str(),
					 FileAccess::makeflag(flag_,GENERIC_READ,GENERIC_WRITE),
					 FILE_SHARE_READ,
					 NULL,
					 (flag_&FileAccess::FLAG_CR)?OPEN_ALWAYS:OPEN_EXISTING,
					 NULL,
					 NULL
				 );


	if (hFile == INVALID_HANDLE_VALUE)
	{
		System::CheckError("File::Open error");
		return false;
	}

	if(flag_&FileAccess::FLAG_APPEND)
	{
		streamfile->Seek(0,SEEKPOS_END);
	}

	streamfile->native_handle(hFile);

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


void File::Truncate(size_t size_)
{
	StreamDataFile* streamfile=dynamic_cast<StreamDataFile*>(m_refData.get());
	if(!streamfile) return;

	streamfile->Seek(size_,SEEKPOS_BEG);
	if(!SetEndOfFile(streamfile->native_handle()))
	{
		System::CheckError("File::Truncate Error");
	}
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
		if((flag_&FileAccess::FLAG_CR)==0)
		{
			System::CheckError("File::Open Error");
			return false;
		}
		fd=::open(filename.c_str(),shm_fileflag(flag_)|O_CREAT,0777);
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



void File::Truncate(size_t size_)
{
	StreamDataFile* streamfile=dynamic_cast<StreamDataFile*>(m_refData.get());
	if(!streamfile) return;

	streamfile->Seek(size_,FILEPOS_BEG);
	ftruncate(streamfile->native_handle(),size_);
}

#endif

File::File(){}

File::File(const String& filename_,int op)
{
	Open(filename_,op);
}


bool File::Rename(const String& oldname_,const String& newname_)
{
	String oldname=fn_encode(oldname_);
	String newname=fn_encode(newname_);

	int bRet=::rename(oldname.c_str(),newname.c_str());

	return bRet==0;
}

bool File::Remove(const String& filename_)
{
	String fn=fn_encode(filename_);
	int bRet=::remove(fn.c_str());
	return bRet==0;
}


EW_LEAVE
