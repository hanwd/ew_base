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

String fn_encode(const String& oldname_)
{
	return oldname_;
}


#ifdef EW_WINDOWS

bool File::open(const String& filename_,int flag_)
{
	String fn=fn_encode(filename_);
	close();

	DWORD option=(flag_&FLAG_FILE_CR)?OPEN_ALWAYS:OPEN_EXISTING;
	if(flag_&FLAG_FILE_TRUNCATE) option|=TRUNCATE_EXISTING;

	HANDLE hFile=(HANDLE)CreateFileW(
					 IConv::to_wide(fn).c_str(),
					 FileAccess::makeflag(flag_,GENERIC_READ,GENERIC_WRITE),
					 FILE_SHARE_READ|FILE_SHARE_WRITE,
					 NULL,
					 option,
					 NULL,
					 NULL
				 );


	if (hFile == INVALID_HANDLE_VALUE)
	{
		System::CheckError("File::Open error");

		flags.add(FLAG_READER_FAILBIT|FLAG_WRITER_FAILBIT);
		return false;
	}

	flags.del(FLAG_READER_FAILBIT|FLAG_WRITER_FAILBIT);
	impl.reset(hFile);

	if(flag_&FLAG_FILE_APPEND)
	{
		seek(0,SEEKTYPE_END);
	}

	return true;
}

int64_t File::size()
{
	FileAccess::LargeInteger tmp;
	tmp.d[0]=::GetFileSize(impl.get(),&tmp.d[1]);
	return tmp.dval;
}

int32_t File::read(char* buf,size_t len)
{
	DWORD nRead(0);
	if(::ReadFile(impl.get(),buf,len,&nRead,NULL)==FALSE)
	{
		flags.add(FLAG_READER_FAILBIT);
		System::CheckError("File::Read Error");
		return -1;
	}
	return nRead;
}

int32_t File::write(const char* buf,size_t len)
{
	DWORD nWrite(0);
	if(::WriteFile(impl.get(),buf,len,&nWrite,NULL)==FALSE)
	{
		flags.add(FLAG_WRITER_FAILBIT);
		System::CheckError("File::Write Error");
		return -1;
	}
	return nWrite;
}

bool File::eof()
{
	return size()==tell();
}

int64_t File::tell()
{
	return seek(0,SEEKTYPE_CUR);
}

int64_t File::seek(int64_t pos,int t)
{
	LARGE_INTEGER li;
	li.QuadPart=pos;
	li.LowPart = SetFilePointer (impl.get(),
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


void File::rewind()
{
	seek(0,SEEKTYPE_BEG);
}

void File::flush()
{
	if(!::FlushFileBuffers(impl.get()))
	{
		System::CheckError("File::Flush Error");
	}
}

void File::truncate(size_t size_)
{
	seek(size_,SEEKTYPE_BEG);
	if(!SetEndOfFile(impl.get()))
	{
		System::CheckError("File::Truncate Error");
	}
}

#else

static int shm_fileflag(int flag_)
{
	int acc=0;

	if(flag_&FLAG_FILE_WR)
	{
		if(flag_&FLAG_FILE_RD)
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

bool File::open(const String& filename_,int flag_)
{
	String filename=fn_encode(filename_);

	int fd=::open(filename.c_str(),shm_fileflag(flag_),0777);
	if(fd<0)
	{
		if((flag_&FLAG_FILE_CR)!=0)
		{
			fd=::open(filename.c_str(),shm_fileflag(flag_)|O_CREAT,0777);
		}

		if(fd<0)
		{
			flags.add(FLAG_WRITER_FAILBIT|FLAG_READER_FAILBIT);
			System::CheckError("File::Open Error");
			return false;
		}
	}

	flags.clr(0);
	impl.reset(fd);

	if(flag_&FLAG_FILE_APPEND)
	{
		seek(0,SEEKTYPE_END);
	}

	return true;
}

int64_t File::size()
{
	struct stat statbuf;
	if(fstat(impl,&statbuf)<0)
	{
		System::CheckError("File::Size Error");
		return -1;
	}

	return statbuf.st_size;
}



int32_t File::read(char* buf,size_t len)
{
	int nLen= ::read(impl,buf,len);
	if(nLen<0)
	{
		System::CheckError("File::Read Error");
		flags.add(FLAG_READER_FAILBIT);
	}
	return nLen;
}

int32_t File::write(const char* buf,size_t len)
{
	int nLen=::write(impl,buf,len);
	if(nLen<0)
	{
		System::CheckError("File::Write Error");
		flags.add(FLAG_WRITER_FAILBIT);
	}
	return nLen;
}

bool File::eof()
{
	return size()==tell();
}

int64_t File::tell()
{
	return seek(0,SEEKTYPE_CUR);
}

int64_t File::seek(int64_t pos,int t)
{
	return lseek(impl,pos,t);
}


void File::rewind()
{
	seek(0,SEEKTYPE_BEG);
}

void File::flush()
{
	::fsync(impl.get());
}

void File::truncate(size_t size_)
{
	seek(size_,SEEKTYPE_BEG);
	ftruncate(impl,size_);
}

#endif

File::File(){}

File::File(const String& filename_,int op)
{
	open(filename_,op);
}

File::~File()
{
	close();
}

void File::close()
{
	impl.reset();
	flags.clr(0);
}


EW_LEAVE
