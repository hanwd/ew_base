#include "ewa_base/basic/system.h"
#include "ewa_base/basic/stream.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/basic/lockguard.h"
#include "ewa_base/basic/atomic.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/util/strlib.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/codecvt.h"
#include "ewa_base/basic/console.h"
#include "ewa_base/basic/file.h"
#include "ewa_base/collection/linear_buffer.h"

#include "../threading/thread_impl.h"

#include "system_data.h"

#ifdef EW_WINDOWS
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <ctime>
#include <cstdlib>
#include <cstdio>

#ifdef EW_WINDOWS
#include <windows.h>
#else
#include <cerrno>
#include <dlfcn.h>
#endif


#ifdef EWA_BASE_DLL
BOOL APIENTRY DllMain(HANDLE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
	EW_UNUSED(hModule);
	EW_UNUSED(ul_reason_for_call);
	EW_UNUSED(lpReserved);

	return TRUE;
}
#endif


EW_ENTER


int64_t System::GetMemTotalPhys()
{
	return SystemData::current().m_nMemTotalPhys;
}

int64_t System::GetMemAvailPhys()
{
	return SystemData::current().m_nMemAvailPhys;
}

int64_t System::GetMemTotalVirtual()
{
	return SystemData::current().m_nMemTotalVirtual;
}

int64_t System::GetMemAvailVirtual()
{
	return SystemData::current().m_nMemAvailVirtual;
}

void System::Update()
{
	SystemData::current().update_system_info();
}

int System::GetCpuCount()
{
	return SystemData::current().m_nCpuCount;
}

int System::GetPageSize()
{
	return SystemData::current().m_nPageSize;
}

int System::GetCacheLineSize()
{
	return SystemData::current().m_nCacheLine;
}

String System::GetEnv(const String& name,const String& value_if_not_found)
{
	const char* value=::getenv(name.c_str());
	return value?String(value):value_if_not_found;
}

String System::GetUsername()
{
	return GetEnv("USERNAME");
}


int System::GetPid()
{
#ifdef EW_WINDOWS
	return ::GetCurrentProcessId();
#else
	return getpid();
#endif
}

double System::GetCpuTime()
{
#ifdef EW_WINDOWS
	FILETIME k[4];

	LARGE_INTEGER tbeg;
	QueryPerformanceCounter(&tbeg);

	if(GetProcessTimes(GetCurrentProcess(),k+0,k+1,k+2,k+3)!=FALSE)
	{
		double t1=(double)(k[3].dwLowDateTime|((unsigned long long)k[3].dwHighDateTime<<32))*0.0000001;
		double t2=(double)(k[2].dwLowDateTime|((unsigned long long)k[2].dwHighDateTime<<32))*0.0000001;
		return t1+t2;
	}
	else
	{
		return 0;
	}
#else
	double t0=((double)clock())/double(CLOCKS_PER_SEC);
	return t0;
#endif
}


void System::DebugBreak()
{
#ifdef EW_WINDOWS
	::DebugBreak();
#else
	__asm__ volatile("int $0x03");
#endif
}

void System::Exit(int v)
{
	System::LogTrace("System::Exit(%d)",v);
	ThreadManager::current().close();
	::exit(v);
}


#ifdef EW_WINDOWS

String GetModulePathImpl()
{
	wchar_t buf[MAX_PATH]={0};
	::GetModuleFileNameW(NULL,buf,MAX_PATH);
	return buf;
}

const String& System::GetModulePath()
{
	static String sModulePath(GetModulePathImpl());
	return sModulePath;
}


class SerializerReaderProcess : public IStreamData
{
public:
	HANDLE hReader, hWriter;
	PROCESS_INFORMATION pi;

	int recv(char* buf,size_t len)
	{
		DWORD nRead(0);
		if(::ReadFile(hReader,buf,len,&nRead,NULL)==FALSE)
		{
			if(::GetLastError()==0x6d) // pipe closed
			{
				return 0;
			}

			flags.add(FLAG_READER_FAILBIT);
			System::CheckError("File::Read Error");
			return -1;
		}
		return nRead;
	}

	int send(const char* buf,size_t len)
	{
		DWORD nWrite(0);
		if(::WriteFile(hWriter,buf,len,&nWrite,NULL)==FALSE)
		{
			flags.add(FLAG_WRITER_FAILBIT);
			System::CheckError("File::Write Error");
			return -1;
		}
		return nWrite;
	}

	SerializerReaderProcess()
	{
		hReader=hWriter=NULL;
		ZeroMemory(&pi,sizeof(pi));
	}

	bool Execute(const String& s)
	{

		STARTUPINFOW si= {sizeof(STARTUPINFO)};

		StringBuffer<wchar_t> sb(s);
		sb.push_back(0);

		HANDLE hReader0, hWriter0;
		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, FALSE };

		if (!::CreatePipe(&hReader0, &hWriter, &sa, 0))
		{
			System::CheckError("");
			return false;
		}

		if (!::CreatePipe(&hReader, &hWriter0, &sa, 0))
		{
			::CloseHandle(hReader0);
			System::CheckError("");
			return false;
		}


		HANDLE hProcess=GetCurrentProcess();
		::DuplicateHandle(hProcess,hWriter0,hProcess,&hWriter0,0,TRUE,DUPLICATE_SAME_ACCESS|DUPLICATE_CLOSE_SOURCE);
		::DuplicateHandle(hProcess,hReader0,hProcess,&hReader0,0,TRUE,DUPLICATE_SAME_ACCESS|DUPLICATE_CLOSE_SOURCE);


		if(hWriter==NULL)
		{
			::CloseHandle(hReader0);
			::CloseHandle(hWriter0);
			return false;
		}

		si.hStdOutput = hWriter0;
		si.hStdError = hWriter0;
		si.hStdInput = hReader0;

		si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;

		if(!::CreateProcessW(NULL,sb.data(),NULL,NULL,TRUE,0,NULL,NULL,&si,&pi))
		{
			System::LogTrace("System::Exectue:%s FAILED",s);
			::CloseHandle(hWriter0);
			::CloseHandle(hReader0);
			return false;
		}

		::CloseHandle(hReader0);
		::CloseHandle(hWriter0);

		return true;

	}

	~SerializerReaderProcess()
	{
		::CloseHandle(pi.hProcess);
		::CloseHandle(pi.hThread);
		::CloseHandle(hReader);
		::CloseHandle(hWriter);
	}
};

bool System::ExecuteRedirect(const String& s, Stream& stream)
{

	System::LogTrace("System::Exectue:%s", s);

	DataPtrT<SerializerReaderProcess> proc(new SerializerReaderProcess);

	if(proc->Execute(s))
	{		
		stream.assign_reader(proc);
		return true;
	}
	else
	{
		return false;
	}
}

bool System::Execute(const String& s, StringBuffer<char>& result)
{

	Stream stream;
	if (!ExecuteRedirect(s, stream))
	{
		return false;
	}

	stream.write_to_buffer(result);
	return true;
}



bool System::Execute(const String& s)
{
	STARTUPINFOW si= {sizeof(STARTUPINFO)};
	PROCESS_INFORMATION pi;
	StringBuffer<wchar_t> sb(s);
	sb.push_back(0);

	if(!::CreateProcessW(NULL,sb.data(),NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
	{
		System::LogTrace("System::Exectue:%s FAILED",s);
		return false;
	}

	System::LogTrace("System::Exectue:%s",s);

	::CloseHandle(pi.hProcess);
	::CloseHandle(pi.hThread);

	return true;


}

#else

String GetModulePathImpl()
{
	return "";
}

const String& System::GetModulePath()
{
	static String sModulePath(GetModulePathImpl());
	return sModulePath;
}

bool System::Execute(const String& s)
{
	int pid;
	if((pid=fork())==-1)
	{
		System::LogTrace("System::Exectue:%s FAILED",s);
		return false;
	}
	if(pid==0)
	{
		execlp(s.c_str(),"",NULL);
		exit(-1);
	}
	else
	{
		System::LogTrace("System::Exectue:%s",s);
		return true;
	}
}


Stream System::ExecuteRedirect(const String& s,bool* status)
{

	if(status) *status=false;
	return Stream();
}

bool System::Execute(const String& s, StringBuffer<char>& result)
{
	bool flag(false);
	Stream stream=ExecuteRedirect(s,&flag);

	if(!flag) return false;

	stream.write_to_buffer(result);
	return true;

}

#endif


void System::CheckErrno(const String& msg)
{
	int ret=errno;
	if(ret!=0)
	{
		String errmsg=ret==-1?ThreadImpl::this_data().last_error:IConv::from_ansi(strerror(ret));
		System::LogTrace("%s failed, code(%d): %s",msg,ret,errmsg);
	}
}

void System::SetLastError(const String& msg)
{

#ifdef EW_WINDOWS
	::SetLastError(-1);
#endif

	errno=-1;

	ThreadImpl::this_data().last_error=msg;
}

String System::GetLastError()
{
#ifdef EW_WINDOWS

	int ret=::GetLastError();
	if(ret==-1)
	{
		return ThreadImpl::this_data().last_error;
	}
	else if(ret!=0)
	{
		wchar_t* _lpError(NULL);

		bool lang_en=false;
		DWORD langid=SystemData::current().nLangId;

		DWORD msglen=FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
							NULL,
							ret,
							langid,
							(LPWSTR)&_lpError,
							0x100,
							NULL);
		EW_UNUSED(msglen);
		return _lpError;
	}

	return "";

#else
	int ret=errno;
	if(ret==-1)
	{
		return ThreadImpl::this_data().last_error;
	}
	else if(ret!=0)
	{
		return strerror(ret);
	}
	return "";
#endif
}

void System::CheckError(const String& msg)
{
#ifdef EW_WINDOWS

	int ret=::GetLastError();
	if(ret!=0)
	{
		String errstr=GetLastError();
		System::LogTrace("%s failed, code(%d): %s",msg,ret,errstr);
	}
#else
	System::CheckErrno(msg);
#endif
}



extern AtomicSpin g_tSpinConsole;

class SystemLoggerData;

SystemLoggerData* g_pSystemLoggerData;

class SystemLoggerData
{
public:

	char buffer[1024*4];
	LinearBuffer<char> lbuf;
	AtomicSpin spin;

	RefCounter counter;
	File fp;

	bool bEnabled;


	static SystemLoggerData& current()
	{
		if (!g_pSystemLoggerData)
		{
			static char data[sizeof(SystemLoggerData)];
			g_pSystemLoggerData=new(data) SystemLoggerData();
		}
		return *g_pSystemLoggerData;

	}



	SystemLoggerData()
	{
		bEnabled=true;
		lbuf.assign(buffer,sizeof(buffer)-1);
		counter.IncUseCount();
	}

	~SystemLoggerData()
	{

	}

	static const char* str_level(int lv)
	{
		switch(lv)
		{
		case LOGLEVEL_DEBUG:
			return "debug";
		case LOGLEVEL_ERROR:
			return "error";
		case LOGLEVEL_TRACE:
			return "trace";
		case LOGLEVEL_FATAL:
			return "fetal";
		default:
			return "other";
		}
	}	

	void DoLog(int lv,const char* p)
	{
		if(!bEnabled) return;

		LockGuard<AtomicSpin> lock1(spin);

		lbuf.rewind();

		lbuf.send(str_level(lv));
		lbuf.send(" ",1);

		time_t tt=time(NULL);
		size_t nd=::strftime (lbuf.gend(),lbuf.wr_free(),"%Y-%m-%d %H:%M:%S ",localtime(&tt));
		
		lbuf.wr_flip(nd);
		lbuf.send(": ",2);
		lbuf.send(p);
		lbuf.send("\r\n",2);
	

		if(fp.ok())
		{
			fp.write(lbuf.gbeg(),lbuf.rd_free());
			fp.flush();
		}
		else
		{
			Console::Write(buffer);
		}

		if(lv==LOGLEVEL_FATAL)
		{
			System::Exit(-1);
		}
	}

	bool SetLogFile(const char* fn,bool app)
	{
		if (!fp.open(fn, FLAG_FILE_WC | (app ? FLAG_FILE_APPEND : 0)))
		{
			return false;
		}
		((KO_Handle<KO_Policy_handle>&)fp).reset(&counter);
		return true;
	}


};


bool System::SetLogFile(const String& fn,bool app)
{
	return SystemLoggerData::current().SetLogFile(fn.c_str(),app);
}

void System::SetLogEnable(bool f)
{
	SystemLoggerData::current().bEnabled=f;
}


void System::DoLog2(int lv,const char* msg)
{
	SystemLoggerData::current().DoLog(lv,msg);
}


void DllModule::Close()
{
	impl.reset();
}

DllModule::DllModule()
{
	flags.clr(FLAG_INITAL_OPEN);
}

bool DllModule::Open(const String& dll,int f)
{
	flags.clr(f);
	flags.del(FLAG_INITAL_OPEN);

	arr_1t<String> files;
	if (flags.get(FLAG_SEARCH_PATHS))
	{
		if (sizeof(intptr_t) == 4)
		{
			files.append(dll);
			files.append("x86/"+dll);
		}
		else
		{
			files.append(dll);
			files.append("x64/"+dll);
		}		
	}
	else
	{
		files.append(dll);
	}

	KO_Policy_module::type p = KO_Policy_module::invalid_value();
	for (size_t i = 0; i < files.size(); i++)
	{
		p = KO_Policy_module::open(files[i]);
		if (p!=KO_Policy_module::invalid_value())
		{
			impl.reset(p);
			return true;
		}
	}

	return false;
}

#ifdef EW_WINDOWS

void KO_Policy_module::destroy(type& o)
{
	::FreeLibrary(*(HMODULE*)&o);
}

KO_Policy_module::type KO_Policy_module::open(const String& file)
{
	return LoadLibraryW(IConv::to_wide(file).c_str());
}

void* DllModule::GetSymbol(const String& s)
{
	if(!impl.ok()) return NULL;

	void * pfunc = ::GetProcAddress(*(HMODULE*)&impl, s.c_str());

	if (!pfunc && flags.get(FLAG_ON_SYMBOL_NOT_FOUND_CLOSE))
	{
		Close();
	}
	return pfunc;
}

#else

void KO_Policy_module::destroy(type& o)
{
	dlclose(o);
}

KO_Policy_module::type KO_Policy_module::open(const String& file)
{
	return dlopen(file.c_str(),RTLD_LAZY)
}

void* DllModule::GetSymbol(const String& s)
{
	if(!impl.ok()) return NULL;
	return dlsym(impl.get(),s.c_str());
}

#endif



FunctionTracer::FunctionTracer(const char* s,int lv) :func(s),level(lv)
{
	System::DoLog(level,"%s Enter", func);
}

FunctionTracer::~FunctionTracer()
{
	System::DoLog(level,"%s Leave", func);
}



char System::GetPathSep()
{
#ifdef EW_WINDOWS
	return '\\';
#else
	return '/';
#endif
}

bool System::IsPathSep(char ch)
{
	return ch=='/'||ch=='\\';
}

String System::AdjustPath(const String& path,bool sep)
{
	char ch=GetPathSep();

	size_t _n=path.size();
	if(_n==0)
	{
		return "";
	}
	else if(IsPathSep(path.c_str()[_n-1]))
	{
		if(!sep)
		{
			return path.substr(0,_n-1);
		}
	}
	else if(sep)
	{
		return path+String(&ch,1);
	}

	return path;
}

bool System::IsRelative(const String& file)
{
	if(IsPathSep(file.c_str()[0])) return false;
	return file.find(":")<0;
}

String NormalPath(const String& file_)
{
	String file(file_);
	file.replace('\\','/');
	arr_1t<String> arr=string_split(file,"/");

	arr_1t<String> tmp;

	for(size_t i=0;i<arr.size();i++)
	{
		if(arr[i]=="")
		{
			if(i==0) tmp.push_back("");
		}
		else if (arr[i] == ".")
		{
			continue;
		}
		else if(arr[i]!="..")
		{
			tmp.push_back(arr[i]);
		}
		else if(tmp.empty()||tmp.back()=="..")
		{
			tmp.push_back(arr[i]);
		}
		else
		{
			tmp.pop_back();
		}
	}

	return string_join(tmp.begin(),tmp.end(),"/");
}

String System::MakePath(const String& file,const String& path)
{
	
	if(IsRelative(file))
	{
		return NormalPath(AdjustPath(path,true)+file);
	}
	else if(file=="/"||file=="\\")
	{
		int pos=path.find(':');
		if(pos<0)
		{
			return "/";
		}
		else
		{
			return path.substr(0,pos);
		}
	}
	else
	{
		return NormalPath(file);
	}
}

String System::GetResdataPath()
{
	return SystemData::current().m_sResdataPath;
}

bool System::SetResdataPath(const String& s)
{
	SystemData::current().m_sResdataPath = MakePath(s, GetCwd());
	return true;
}

String System::GetAppdataPath()
{
	return SystemData::current().m_sAppdataPath;
}

bool System::SetAppdataPath(const String& s)
{
	SystemData::current().m_sAppdataPath = MakePath(s, GetCwd());
	return true;
}

String System::MakeResdataPath(const String& file)
{
	return MakePath(file, GetResdataPath());
}

String System::MakeAppdataPath(const String& file)
{
	return MakePath(file, GetAppdataPath());
}

String System::GetCwd()
{
#ifdef EW_WINDOWS
	static const int BUFFER_LENGTH=1024*2;
	wchar_t path[BUFFER_LENGTH];
	DWORD rc=::GetCurrentDirectoryW(BUFFER_LENGTH,path);
	return path;
#else
	const int maxpath=1024;
	char buffer[maxpath];
	return getcwd(buffer,maxpath);
#endif
}

bool System::SetCwd(const String& s)
{

#ifdef EW_WINDOWS
	return ::SetCurrentDirectoryW(IConv::to_wide(s).c_str())!=FALSE;
#else
	return false;
#endif
}


size_t System::Backtrace(void** stack,size_t frames)
{
#ifdef EW_MSVC
	return CaptureStackBackTrace(0,frames,stack,NULL);
#elif defined(EW_WINDOWS)
    return 0;
#else
	//return backtrace(stack,frames);
	return 0;
#endif
}




EW_LEAVE
