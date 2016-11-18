
#include "ewa_base/ipc/pipe.h"
#include "ewa_base/threading/thread.h"
#include "ewa_base/basic/codecvt.h"
#include "ewa_base/basic/system.h"

#ifdef EW_WINDOWS
#include <windows.h>
#else
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#include <sys/ioctl.h>


#endif


EW_ENTER

#ifdef EW_WINDOWS

class pipe_detail
{
public:
	typedef Pipe::impl_type impl_type;

	static bool link_pipe(impl_type& reader,impl_type& writer,int buflen)
	{
		HANDLE hReader,hWriter;

		bool flag=::CreatePipe(&hReader,&hWriter,NULL,buflen)!=0;
		if(flag)
		{
			reader.reset(hReader);
			writer.reset(hWriter);
		}
		else
		{
			reader.reset();
			writer.reset();
		}
		return flag;
	}

	static void close_pipe(impl_type& impl)
	{
		impl.reset();
	}

	static String make_pipename(const String& name_)
	{
		return "\\\\.\\pipe\\"+name_;
	}

	static bool create_namedpipe(impl_type& impl,const String& name_)
	{
		String name=make_pipename(name_);

		int tout=0;//NMPWAIT_WAIT_FOREVER;
		int nnum=PIPE_UNLIMITED_INSTANCES;

		HANDLE hPipe = CreateNamedPipeW(IConv::to_wide(name).c_str(), PIPE_ACCESS_DUPLEX,
										PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
										nnum, 0, 0, tout, NULL);

		if(hPipe==INVALID_HANDLE_VALUE)
		{
			return false;
		}

		impl.reset(hPipe);

		return true;
	}

	static bool accept_namedpipe(impl_type& impl)
	{
		BOOL r= ConnectNamedPipe((HANDLE)impl.get(), NULL);
		if(r!=FALSE) return true;
		int e=GetLastError();
		if(e==ERROR_PIPE_CONNECTED)
		{
			return true;
		}
		return false;

	}

	static void disconnect_namedpipe(impl_type& impl)
	{
		DisconnectNamedPipe((HANDLE)impl.get());
	}

	static bool connect_namedpipe(impl_type& impl,const String& name_,DWORD t)
	{
		String name=make_pipename(name_);
		(void)&impl;
		if(t==0) t=1;
		if (WaitNamedPipeW(IConv::to_wide(name).c_str(), t) == FALSE)
		{
			return false;
		}
		return true;
	}

	static bool open_namedpipe(impl_type& impl,const String& name_)
	{
		String name=make_pipename(name_);
		HANDLE hPipe = CreateFileW(IConv::to_wide(name).c_str(), GENERIC_READ | GENERIC_WRITE, 0,
								   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if(hPipe==INVALID_HANDLE_VALUE)
		{
			return false;
		}
		impl.reset(hPipe);
		return true;
	}

	static int read_pipe(impl_type& impl,char* buf_,int len_)
	{
		DWORD dwLen;
		if(ReadFile((HANDLE)impl.get(), buf_, len_, &dwLen, NULL)!=FALSE)
		{
			return dwLen;
		}
		else
		{
			return -1;
		}
	}

	static int write_pipe(impl_type& impl,const char* buf_,int len_)
	{
		DWORD dwLen;
		if(WriteFile((HANDLE)impl.get(), buf_, len_, &dwLen, NULL)!=FALSE)
		{
			return dwLen;
		}
		else
		{
			return -1;
		}
	}


};
#else

class pipe_detail
{
public:
	typedef Pipe::impl_type impl_type;

	static bool link_pipe(impl_type& reader,impl_type& writer,int buflen)
	{
		int fd[2];

		if(::pipe(fd)<0)
		{
			reader.reset();
			writer.reset();
			return false;
		}
		else
		{
			reader.reset(fd[0]);
			writer.reset(fd[1]);
			return true;
		}

	}

	//bool s_pipe(int fd[2])
	//{
	//	return socketpair(AF_UNIX,SOCK_STREAM,0,fd)>=0;
	//}

	static void close_pipe(impl_type& impl)
	{
		impl.reset();
		if(impl.serv.ok())
		{
			impl.serv.reset();
			String tmp=make_pipename(impl.name);
			unlink(tmp.c_str());
			impl.name="";
		}
	}

	static String make_pipename(const String& name_)
	{
		return "/var/tmp/pipe_"+name_;
	}

	static int unix_sockaddr(struct sockaddr_un& un,const String& name)
	{
		memset(&un, 0, sizeof(un));
		un.sun_family = AF_UNIX;
		strcpy(un.sun_path, name.c_str());

		#define EW_OFFSET(X,Y) ((char*)&(*(X*)NULL).Y-(char*)NULL)

		return EW_OFFSET(struct sockaddr_un, sun_path) + strlen(name.c_str());
	}

	static bool create_namedpipe(impl_type& impl,const String& name_)
	{
		if(impl.serv.ok()) close_pipe(impl);

		String name=make_pipename(name_);
		int fd;
		struct sockaddr_un  un;

		if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		{
			return false;
		}

		impl.serv.reset(fd);
		unlink(name.c_str());

		socklen_t nlen=unix_sockaddr(un,name);

		if (bind(fd, (struct sockaddr *)&un, nlen) < 0)
		{
			return false;
		}

		if (listen(fd, 10) < 0)
		{
			return false;
		}

		impl.name=name_;
		return true;

	}

	static bool accept_namedpipe(impl_type& impl)
	{
		int   clifd;
		struct sockaddr_un  un;

		socklen_t len = sizeof(un);

		for(;;)
		{
			if(Thread::this_thread().test_canceled())
			{
				return false;
			}

			if ((clifd = accept(impl.serv.get(), (struct sockaddr *)&un, &len)) >= 0)
			{
				break;
			}

			return false;
		}

		impl.reset(clifd);

		return true;

	}


	static bool open_namedpipe(impl_type& impl,const String& name_)
	{
		String name=make_pipename(name_);
		int fd;
		struct sockaddr_un un;

		if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		{
			return false;
		}

		impl.reset(fd);
		socklen_t nlen=unix_sockaddr(un,make_pipename(String::Format("%s.%5d",name_,getpid())));
		if (bind(fd, (struct sockaddr *)&un, nlen) < 0)
		{
			return false;
		}
		unlink(un.sun_path);

		nlen=unix_sockaddr(un,name);

		if (connect(fd, (struct sockaddr *)&un, nlen) < 0)
		{
			return false;
		}

		return true;
	}


	static void disconnect_namedpipe(impl_type& impl)
	{
		impl.reset();
	}

	static bool connect_namedpipe(impl_type& impl,const String& name,int t)
	{
		return true;
	}


	static int read_pipe(impl_type& impl,char* buf_,int len_)
	{
		int dwRet=read(impl.get(),buf_,len_);
		if(dwRet>0)
		{
			return dwRet;
		}
		else
		{
			return -1;
		}
	}

	static int write_pipe(impl_type& impl,const char* buf_,int len_)
	{
		int dwRet=write(impl.get(),buf_,len_);
		if(dwRet>0)
		{
			return dwRet;
		}
		else
		{
			return -1;
		}
	}


};

#endif


Pipe::Pipe()
{

}

bool Pipe::LinkPipe(Pipe& reader,Pipe& writer,int buflen)
{
	if(!pipe_detail::link_pipe(reader.impl,writer.impl,buflen))
	{
		System::CheckError("Pipe::LinkPipe Error");
		return false;
	}
	return true;
}

bool NamedPipe::Create(const String& name)
{
	if(impl.ok())
	{
		return false;
	}

	if(!pipe_detail::create_namedpipe(impl,name))
	{
		System::CheckError("NamedPipe::Create Error");
		return false;
	}
	return true;

}

bool NamedPipe::Accept()
{
	return pipe_detail::accept_namedpipe(impl);
}

void NamedPipe::Disconnect()
{
	if(!impl.ok()) return;
	pipe_detail::disconnect_namedpipe(impl);
}

bool NamedPipe::Connect(const String& name,int t)
{
	if(impl.ok()) return false;
	int t0=t==-1?500:t;

	Thread& thread(Thread::this_thread());

	for(;;)
	{
		if(!pipe_detail::connect_namedpipe(impl,name,t0))
		{
			return false;
		}

		if(pipe_detail::open_namedpipe(impl,name))
		{
			break;
		}

		if(t!=-1||thread.test_canceled())
		{
			return false;
		}

		Thread::sleep_for(100);
	}

	return true;
}

bool NamedPipe::Connect(const String& name)
{
	return Connect(name,-1);
}

int Pipe::Recv(char* buf_,int len_)
{
	int n = pipe_detail::read_pipe(impl, buf_, len_);

	if (n<=0)
	{
		System::CheckError("Pipe::Recv Error");
		return 0;
	}
	return n;
}

int Pipe::Send(const char* buf_,int len_)
{
	int n = pipe_detail::write_pipe(impl, buf_, len_);
	if (n == len_)
	{
		return n;
	}
	else if (n<=0)
	{
		System::CheckError("Pipe::Send Error");
		return 0;
	}
	else
	{
		System::LogTrace("Pipe::Send partial");
		return n;
	}

}



void Pipe::Close()
{
	pipe_detail::close_pipe(impl);
}

Pipe::~Pipe()
{

}

EW_LEAVE
