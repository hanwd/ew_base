#include "ewa_base/ipc/shm.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/basic/file.h"

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

#ifdef EW_WINDOWS
class ShareMem_detail : public SharedMem
{
public:
	typedef SharedMem::impl_type impl_type;


	static bool shm_create(impl_type& impl,const String& name_,size_t size_,int flag_)
	{

		HANDLE hMapFile;

		if(flag_&FileAccess::FLAG_CR)
		{
			FileAccess::LargeInteger tmp;
			tmp.dval=size_;

			hMapFile = CreateFileMappingA(
						   INVALID_HANDLE_VALUE,   // Use paging file - shared memory
						   NULL,                   // Default security attributes
						   PAGE_READWRITE,         // Allow read and write access
						   tmp.d[1],               // High-order DWORD of file mapping max size
						   tmp.d[0],               // Low-order DWORD of file mapping max size
						   name_.empty()?NULL:name_.c_str()
					   );


		}
		else
		{
			hMapFile = OpenFileMappingA(
						   FileAccess::makeflag(flag_,FILE_MAP_READ,FILE_MAP_WRITE),
						   FALSE,                  // Do not inherit the name
						   name_.c_str()			// File mapping name
					   );
		}

		if (hMapFile == NULL)
		{
			System::CheckError("CreateFileMapping/OpenFileMapping");
			return false;
		}

		char* pView = (char*)MapViewOfFile(
						  hMapFile,               // Handle of the map object
						  FileAccess::makeflag(flag_,FILE_MAP_READ,FILE_MAP_WRITE),					// access
						  0,                      // High-order DWORD of the file offset
						  0,					// Low-order DWORD of the file offset
						  size_               // The number of bytes to map to view
					  );

		if(!pView)
		{
			::CloseHandle(hMapFile);
			System::CheckError("MapViewOfFile");
			return false;
		}

		impl.m_pAddr=pView;
		impl.m_nSize=size_;
		impl.m_sName=name_;
		impl.m_pHandle.reset(hMapFile);

		return true;
	}

	static bool shm_openfile(impl_type& impl,const String& name_,size_t size_,int flag_)
	{

		KO_Handle<KO_Policy_handle> m_pExtraHandle;

		if(size_!=0 && (flag_&FileAccess::FLAG_WR)==0)
		{
			return false;
		}

		HANDLE hFile=(HANDLE)CreateFileA(
						 name_.c_str(),
						 FileAccess::makeflag(flag_,GENERIC_READ,GENERIC_WRITE),
						 FileAccess::makeflag(flag_,FILE_SHARE_READ,FILE_SHARE_WRITE),
						 NULL,
						 (flag_&FileAccess::FLAG_CR)?OPEN_ALWAYS:OPEN_EXISTING,
						 NULL,
						 NULL
					 );

		if (hFile == INVALID_HANDLE_VALUE)
		{
			System::CheckError("CreateFile");
			return false;
		}

		if(size_!=0)
		{
			SetFilePointer(hFile, size_, NULL, FILE_BEGIN);
			SetEndOfFile(hFile);
		}

		m_pExtraHandle.reset(hFile);

		FileAccess::LargeInteger tmp;

		tmp.d[0]=::GetFileSize(hFile,&tmp.d[1]);
		if(tmp.dval<=0)
		{
			System::CheckError("GetFileSize");
			return false;
		}
		size_=tmp.dval;

		HANDLE hMapFile = CreateFileMapping(
							  m_pExtraHandle,
							  NULL,                   // Default security attributes
							  flag_&FileAccess::FLAG_WR?PAGE_READWRITE:PAGE_READONLY,
							  tmp.d[1],               // High-order DWORD of file mapping max size
							  tmp.d[0],               // Low-order DWORD of file mapping max size
							  NULL
						  );

		if(hMapFile==NULL)
		{
			System::CheckError("CreateFileMapping");
			return false;
		}

		char* pView = (char*)MapViewOfFile(
						  hMapFile,               // Handle of the map object
						  FileAccess::makeflag(flag_,FILE_MAP_READ,FILE_MAP_WRITE),
						  0,						// High-order DWORD of the file offset
						  0,						// Low-order DWORD of the file offset
						  size_						// The number of bytes to map to view
					  );

		if(!pView)
		{
			::CloseHandle(hMapFile);
			System::CheckError("MapViewOfFile");
			return false;
		}

		impl.m_pAddr=pView;
		impl.m_nSize=size_;
		impl.m_sName=name_;
		impl.m_pHandle.reset(hMapFile);
		return true;
	}

	static void shm_close(impl_type& impl)
	{
		if(!impl.m_pAddr)
		{
			return;
		}

		impl.m_pHandle.close();
		if(UnmapViewOfFile(impl.m_pAddr)==0)
		{
			System::CheckError("UnmapViewOfFile");
		}

		impl.m_pAddr=NULL;
		impl.m_nSize=0;
		impl.m_sName="";

	}

};

#else


class ShareMem_detail : public SharedMem
{
public:
	typedef SharedMem::impl_type impl_type;

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

	static bool shm_create(impl_type& impl,const String& name_,size_t size_,int flag_)
	{

		KO_Handle<KO_Policy_handle> hfd;
		int fd=-1;

		int map_type=MAP_SHARED;

		if(!name_.empty())
		{
			int oflag=shm_fileflag(flag_);
			if(flag_&FileAccess::FLAG_CR)
			{
				oflag|=O_CREAT;
			}

			fd=shm_open(name_.c_str(),oflag,0777);
			if(fd<0)
			{
				System::CheckError("shm_open");
				return false;
			}
			hfd.reset(fd);

			if(flag_&FileAccess::FLAG_CR)
			{
				ftruncate(fd,size_);
			}
			else
			{
				struct stat statbuf;
				if(fstat(fd,&statbuf)<0)
				{
					return false;
				}
				if((size_t)statbuf.st_size!=size_)
				{
					return false;
				}
			}
		}
		else
		{
			map_type=MAP_PRIVATE|MAP_ANONYMOUS;
		}

		void* _mem=mmap(0,size_,FileAccess::makeflag(flag_,PROT_READ,PROT_WRITE),map_type,fd,0);
		if(_mem==MAP_FAILED)
		{
			System::CheckError("mmap");
			return false;
		}

		impl.m_pAddr=(char*)_mem;
		impl.m_nSize=size_;
		impl.m_sName=name_;

		return true;
	}

	static bool shm_openfile(impl_type& impl,const String& name_,size_t size_,int flag_)
	{
		KO_Handle<KO_Policy_handle> hfd;

		int fd=::open(name_.c_str(),shm_fileflag(flag_),0777);
		if(fd<0)
		{
			if((flag_&FileAccess::FLAG_CR)==0||size_==0)
			{
				return false;
			}
			fd=::open(name_.c_str(),shm_fileflag(flag_)|O_CREAT,0777);
			if(fd<0)
			{
				System::CheckError("open");
				return false;
			}
		}
		hfd.reset(fd);
		if(size_!=0)
		{
			ftruncate(fd,size_);
		}

		struct stat statbuf;
		if(fstat(fd,&statbuf)<0)
		{
			return false;
		}

		size_=statbuf.st_size;

		void* _mem=mmap(0,size_,FileAccess::makeflag(flag_,PROT_READ,PROT_WRITE),MAP_SHARED,fd,0);
		if(_mem==MAP_FAILED)
		{
			System::CheckError("mmap");
			return false;
		}

		impl.m_pAddr=(char*)_mem;
		impl.m_nSize=size_;
		impl.m_sName=name_;

		return true;

	}

	static void shm_close(impl_type& impl)
	{
		if(!impl.m_pAddr)
		{
			return;
		}

		munmap(impl.m_pAddr,impl.m_nSize);
		impl.m_pAddr=NULL;
		impl.m_nSize=0;

	}

};

#endif


SharedMem::SharedMem()
{

}

SharedMem::~SharedMem()
{
	Close();
}

bool SharedMem::Alloc(size_t size_,int flag_)
{
	return Create("",size_,flag_);
}

bool SharedMem::Open(const String& name_,size_t size_,int flag_)
{
	ShareMem_detail::shm_close(impl);

	if(!ShareMem_detail::shm_create(impl,name_,size_,flag_))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool SharedMem::Create(const String& name_,size_t size_,int flag_)
{
	ShareMem_detail::shm_close(impl);

	if(!ShareMem_detail::shm_create(impl,name_,size_,flag_|FileAccess::FLAG_CR))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool SharedMem::OpenFile(const String& name_,size_t size_,int flag_)
{
	ShareMem_detail::shm_close(impl);

	if(!ShareMem_detail::shm_openfile(impl,name_,size_,flag_))
	{
		return false;
	}
	else
	{
		return true;
	}
}

void SharedMem::Close()
{
	ShareMem_detail::shm_close(impl);
}



IShmMemory::~IShmMemory()
{
	Close();
}

bool IShmMemory::Create(const String& name,size_t sz)
{
	if(mem.Open(name,sizeof(IShmHeader)))
	{
		pHeader=(IShmHeader*)mem.data();
		sz=pHeader->size;
		mem.Close();
	}

	m_nShift=0;
	if(!mem.Create(name,sz))
	{
		return false;
	}

	pHeader=(IShmHeader*)mem.data();
	pHeader->size=sz;
	return true;
}

char* IShmMemory::allocate(size_t s,size_t al)
{
	if(!mem.data())
	{
		return NULL;
	}

	size_t kk=(m_nShift+al-1)&~(al-1);

	if(kk+s>mem.size())
	{
		return NULL;
	}

	char* p= mem.data()+kk;
	m_nShift=kk+s;
	return p;
}

void IShmMemory::Close()
{
	mem.Close();
}

EW_LEAVE
