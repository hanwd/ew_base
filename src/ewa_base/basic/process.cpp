#include "ewa_base/basic/process.h"

EW_ENTER



class DLLIMPEXP_EWA_BASE StreamDataPipe : public StreamData
{
public:

	virtual int32_t Read(char* buf,size_t len)
	{
		if(flags.get(FLAG_READ_FAIL_BIT)) return -1;

		DWORD nRead(0);
		if(::ReadFile(hReader,buf,len,&nRead,NULL)==FALSE)
		{
			flags.add(FLAG_READ_FAIL_BIT);
			System::CheckError("File::Read Error");
			return -1;
		}
		return nRead;
	}

	virtual int32_t Write(const char* buf,size_t len)
	{
		if(flags.get(FLAG_WRITE_FAIL_BIT)) return -1;

		DWORD nWrite(0);
		if(::WriteFile(hWriter,buf,len,&nWrite,NULL)==FALSE)
		{
			flags.add(FLAG_WRITE_FAIL_BIT);
			System::CheckError("File::Write Error");
			return -1;
		}
		return nWrite;
	}

	KO_Handle<KO_Policy_handle> hReader, hWriter;
};

class ProcessImpl
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


	void Kill(int r)
	{
		if(pi.hProcess==NULL) return;
		::TerminateProcess(pi.hProcess,r);
	}


	bool Redirect(KO_Handle<KO_Policy_handle>& h)
	{
		if(pi.hProcess!=NULL) return false;
		hWriter=h;
		hReader.close();
		return true;
	}

	bool Redirect()
	{
		if(pi.hProcess!=NULL) return false;

		hReader.close();
		hWriter.close();
		hStream.SetData(NULL);

		DataPtrT<StreamDataPipe> hPipe(new StreamDataPipe);

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
			hPipe->hReader.close();
			hWriter.close();
		}

		hStream.SetData(hPipe.get());
		return true;
	}

	~ProcessImpl()
	{
		Close();
	}

	void Wait()
	{
		if(pi.hProcess==NULL) return;
		::WaitForSingleObject(pi.hProcess,INFINITE);
		Close();
	}

	void Close()
	{
		hReader.close();
		hWriter.close();
		hStream.Close();

		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);
		pi.hThread=NULL;
		pi.hProcess=NULL;
	}

	bool Execute(const String& s)
	{
		if(pi.hProcess!=NULL) return false;

		STARTUPINFOA si= {sizeof(STARTUPINFO)};

		StringBuffer<char> sb(s);
		sb.push_back(0);


		HANDLE h1=KO_Policy_handle::duplicate(hReader,TRUE);
		HANDLE h2=KO_Policy_handle::duplicate(hWriter,TRUE);

		si.hStdOutput = h2;
		si.hStdError = h2;
		si.hStdInput = h1;

		si.dwFlags = (h1!=NULL||h2!=NULL)?STARTF_USESTDHANDLES:0;

		bool flag=::CreateProcessA(NULL,sb.data(),NULL,NULL,TRUE,0,NULL,NULL,&si,&pi)!=FALSE;

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

Process::Process()
{

}


bool Process::Redirect()
{
	if(!impl.ok()) impl.reset(new ProcessImpl);
	return ((ProcessImpl*)impl)->Redirect();
}

bool Process::Redirect(KO_Handle<KO_Policy_handle> h)
{
	if(!impl.ok()) impl.reset(new ProcessImpl);
	return ((ProcessImpl*)impl)->Redirect(h);
}

bool Process::Execute(const String& cmd)
{
	if(!impl.ok()) impl.reset(new ProcessImpl);
	return ((ProcessImpl*)impl)->Execute(cmd);
}

Stream Process::GetStream()
{
	if(!impl.ok()) return Stream();
	return ((ProcessImpl*)impl)->hStream;
}


void Process::Wait()
{
	if(!impl.ok()) return;
	((ProcessImpl*)impl)->Wait();
}

void Process::Close()
{
	if(!impl.ok()) return;
	((ProcessImpl*)impl)->Wait();
	impl.close();
}

void Process::Kill(int r)
{
	if(!impl.ok()) return;
	((ProcessImpl*)impl)->Kill(r);
}

void KO_Policy_pointer<ProcessImpl>::destroy(type& o)
{
	delete o;
	o=NULL;
}

template class KO_Policy_pointer<ProcessImpl>;


EW_LEAVE
