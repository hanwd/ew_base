#include "ewa_base/basic/process.h"
#include "ewa_base/basic/system.h"

EW_ENTER



class DLLIMPEXP_EWA_BASE SerializerReaderPipe : public IStreamData
{
public:

	virtual int recv(char* buf,size_t len)
	{
		if(flags.get(FLAG_READER_FAILBIT)) return -1;
#ifdef EW_WINDOWS
		DWORD nRead(0);
		if(::ReadFile(hReader.get(),buf,len,&nRead,NULL)==FALSE)
		{
			flags.add(FLAG_READER_FAILBIT);
			System::CheckError("File::Read Error");
			return -1;
		}
		return nRead;
#else
		return 0;
#endif
	}

	virtual int send(const char* buf,size_t len)
	{
		if(flags.get(FLAG_WRITER_FAILBIT)) return -1;
#ifdef EW_WINDOWS
		DWORD nWrite(0);
		if(::WriteFile(hWriter.get(),buf,len,&nWrite,NULL)==FALSE)
		{
			flags.add(FLAG_WRITER_FAILBIT);
			System::CheckError("File::Write Error");
			return -1;
		}
		return nWrite;
#else
		return -1;
#endif
	}

	KO_Handle<KO_Policy_handle> hReader, hWriter;
};

#ifdef EW_WINDOWS
class ProcessImpl : public ObjectData
{
public:
	BitFlags flags;

	KO_Handle<KO_Policy_handle> hReader, hWriter;

	Stream hStream;

	PROCESS_INFORMATION pi;

	ProcessImpl()
	{
		ZeroMemory(&pi,sizeof(pi));
	}


	bool Kill(int r)
	{
		if(pi.hProcess==NULL) return true;
		return ::TerminateProcess(pi.hProcess,r)!=FALSE;
	}


	bool Redirect(KO_Handle<KO_Policy_handle>& h)
	{
		if(pi.hProcess!=NULL)
		{
			System::LogTrace("Process::Redirect failed, process already created!");
			return false;
		}
		hWriter=h;
		hReader.reset();
		return true;
	}

	bool Redirect()
	{
		if(pi.hProcess!=NULL) return false;

		hReader.reset();
		hWriter.reset();
		hStream.close();

		DataPtrT<SerializerReaderPipe> hPipe(new SerializerReaderPipe);

		HANDLE h1,h2;
		if(::CreatePipe(&h1,&h2,NULL,0))
		{
			hPipe->hReader.reset(h1);
			hWriter.reset(h2);
		}
		else
		{
			return false;
		}

		if(::CreatePipe(&h1,&h2,NULL,0))
		{
			hReader.reset(h1);
			hPipe->hWriter.reset(h2);
		}
		else
		{
			hPipe->hReader.reset();
			hWriter.reset();
		}

		hStream.assign_reader(hPipe);
		return true;
	}

	~ProcessImpl()
	{
		Close();
	}

	bool WaitFor(int ms)
	{
		if(pi.hProcess==NULL) return true;

		DWORD rc=::WaitForSingleObject(pi.hProcess,ms);
		if(rc==WAIT_TIMEOUT)
		{
			return false;
		}
		return true;
	}

	void Wait()
	{
		if(pi.hProcess==NULL) return;
		::WaitForSingleObject(pi.hProcess,INFINITE);
	}

	bool GetExitCode(int* code)
	{
		DWORD code2;
		if(GetExitCodeProcess(pi.hProcess,&code2))
		{
			if(code) *code=code2;
			return true;
		}
		return false;
	}

	void Close()
	{
		hReader.reset();
		hWriter.reset();
		hStream.close();
		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);
		pi.hThread=NULL;
		pi.hProcess=NULL;
	}

	bool Execute(const String& s)
	{
		if(pi.hProcess!=NULL) return false;

		STARTUPINFOW si= {sizeof(STARTUPINFO)};

		StringBuffer<wchar_t> sb(s);
		sb.push_back(0);


		HANDLE h1=KO_Policy_handle::duplicate(hReader.get(),TRUE);
		HANDLE h2=KO_Policy_handle::duplicate(hWriter.get(),TRUE);

		si.hStdOutput = h2;
		si.hStdError = h2;
		si.hStdInput = h1;

		si.dwFlags = (h1!=NULL||h2!=NULL)?STARTF_USESTDHANDLES:0;

		bool flag=::CreateProcessW(NULL,sb.data(),NULL,NULL,(si.dwFlags&STARTF_USESTDHANDLES)?TRUE:FALSE,0,NULL,NULL,&si,&pi)!=FALSE;

		::CloseHandle(h1);
		::CloseHandle(h2);

		if(!flag)
		{
			System::LogTrace("Process::Exectue:%s FAILED",s);
			return false;
		}

		return true;

	}


};
#else
class ProcessImpl : public ObjectData
{
public:
	BitFlags flags;

	KO_Handle<KO_Policy_handle> hReader, hWriter;

	Stream hStream;

	//PROCESS_INFORMATION pi;

	ProcessImpl()
	{
		//ZeroMemory(&pi,sizeof(pi));
	}


	bool Kill(int r)
	{
		//if(pi.hProcess==NULL) return true;
		//return ::TerminateProcess(pi.hProcess,r)!=FALSE;
		return false;
	}


	bool Redirect(KO_Handle<KO_Policy_handle>& h)
	{
	/*
		if(pi.hProcess!=NULL)
		{
			System::LogTrace("Process::Redirect failed, process already created!");
			return false;
		}
		hWriter=h;
		hReader.reset();
	*/
		return true;
	}

	bool Redirect()
	{
	/*
		if(pi.hProcess!=NULL) return false;

		hReader.reset();
		hWriter.reset();
		hStream.close();

		SharedPtrT<SerializerReaderPipe> hPipe(new SerializerReaderPipe);

		HANDLE h1,h2;
		if(::CreatePipe(&h1,&h2,NULL,0))
		{
			hPipe->hReader.reset(h1);
			hWriter.reset(h2);
		}
		else
		{
			return false;
		}

		if(::CreatePipe(&h1,&h2,NULL,0))
		{
			hReader.reset(h1);
			hPipe->hWriter.reset(h2);
		}
		else
		{
			hPipe->hReader.reset();
			hWriter.reset();
		}

		hStream.assign_reader(hPipe);
		*/
		return true;
	}

	~ProcessImpl()
	{
		Close();
	}

	bool WaitFor(int ms)
	{
	/*
		if(pi.hProcess==NULL) return true;

		DWORD rc=::WaitForSingleObject(pi.hProcess,ms);
		if(rc==WAIT_TIMEOUT)
		{
			return false;
		}
		*/
		return true;
	}

	void Wait()
	{
		//if(pi.hProcess==NULL) return;
		//::WaitForSingleObject(pi.hProcess,INFINITE);
	}

	bool GetExitCode(int* code)
	{
	/*
		DWORD code2;
		if(GetExitCodeProcess(pi.hProcess,&code2))
		{
			if(code) *code=code2;
			return true;
		}
		*/
		return false;
	}

	void Close()
	{
		hReader.reset();
		hWriter.reset();
		hStream.close();
		/*
		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);
		pi.hThread=NULL;
		pi.hProcess=NULL;
		*/
	}

	bool Execute(const String& s)
	{
	/*
		if(pi.hProcess!=NULL) return false;

		STARTUPINFOW si= {sizeof(STARTUPINFO)};

		StringBuffer<wchar_t> sb(s);
		sb.push_back(0);


		HANDLE h1=KO_Policy_handle::duplicate(hReader.get(),TRUE);
		HANDLE h2=KO_Policy_handle::duplicate(hWriter.get(),TRUE);

		si.hStdOutput = h2;
		si.hStdError = h2;
		si.hStdInput = h1;

		si.dwFlags = (h1!=NULL||h2!=NULL)?STARTF_USESTDHANDLES:0;

		bool flag=::CreateProcessW(NULL,sb.data(),NULL,NULL,(si.dwFlags&STARTF_USESTDHANDLES)?TRUE:FALSE,0,NULL,NULL,&si,&pi)!=FALSE;

		::CloseHandle(h1);
		::CloseHandle(h2);

		if(!flag)
		{
			System::LogTrace("Process::Exectue:%s FAILED",s);
			return false;
		}
*/
		return true;

	}


};
#endif



Process::Process()
{

}


ProcessImpl& Process::impl()
{
	if(!m_impl)
	{
		m_impl.reset(new ProcessImpl);
	}
	return static_cast<ProcessImpl&>(*m_impl);
}

bool Process::Redirect()
{
	return impl().Redirect();
}

bool Process::Redirect(KO_Handle<KO_Policy_handle> h)
{
	return impl().Redirect(h);
}

bool Process::Execute(const String& cmd)
{
	return impl().Execute(cmd);
}

Stream Process::GetStream()
{
	if(!m_impl) return Stream();
	return impl().hStream;
}


void Process::Wait()
{
	if(!m_impl) return;
	impl().Wait();
}

bool Process::GetExitCode(int* code)
{
	if(!m_impl) return false;
	return impl().GetExitCode(code);
}

bool Process::WaitFor(int ms)
{
	if(!m_impl) return true;
	return impl().WaitFor(ms);
}

void Process::Close()
{
	m_impl.reset(NULL);
}

bool Process::Kill(int r)
{
	if(!m_impl) return true;
	return impl().Kill(r);
}


EW_LEAVE
