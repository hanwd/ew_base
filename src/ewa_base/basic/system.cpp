#include "ewa_base/basic/system.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/basic/lockguard.h"
#include "ewa_base/basic/atomic.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/util/strlib.h"

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

	ew::System::LogTrace("dll_main: %d",(int)ul_reason_for_call);

	return TRUE;
}
#endif


EW_ENTER

class SystemInfo
{
public:

	int64_t m_nMemTotalPhys;
	int64_t m_nMemAvailPhys;
	int64_t m_nMemTotalVirtual;
	int64_t m_nMemAvailVirtual;
	int64_t m_nMemTotalPageFile;
	int64_t m_nMemAvailPageFile;

	int m_nCpuCount;
	int m_nCacheLine;
	int m_nPageSize;

	void update();

	static SystemInfo& current()
	{
		static SystemInfo si;
		return si;
	}

	SystemInfo();

};


#ifdef EW_MSVC

SystemInfo::SystemInfo()
{

	SYSTEM_INFO info;
	GetSystemInfo(&info);
	m_nPageSize=info.dwPageSize;
	m_nCpuCount=info.dwNumberOfProcessors;

	size_t line_size = 0;
	DWORD buffer_size = 0;
	DWORD i = 0;
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;

	GetLogicalProcessorInformation(0, &buffer_size);

	EW_ASSERT(buffer_size<4096);

	char _local_buffer[4096];
	buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *) _local_buffer;

	GetLogicalProcessorInformation(&buffer[0], &buffer_size);

	for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i)
	{
		if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1)
		{
			line_size = buffer[i].Cache.LineSize;
			break;
		}
	}

	m_nCacheLine=(int)line_size;

	update();
}

#elif defined(EW_LINUX)

SystemInfo::SystemInfo()
{

	m_nPageSize=sysconf(_SC_PAGE_SIZE);;
	m_nCpuCount=sysconf(_SC_NPROCESSORS_ONLN);
	m_nCacheLine=-1;

	FILE * p = 0;
	p = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");

	if (p)
	{
		fscanf(p, "%d", &m_nCacheLine);
		fclose(p);
	}

	update();

}
#else

SystemInfo::SystemInfo()
{
}

#endif


#ifdef EW_MSVC

void SystemInfo::update()
{
	MEMORYSTATUSEX ms;
	ms.dwLength=sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&ms);

	m_nMemTotalPhys=ms.ullTotalPhys;
	m_nMemAvailPhys=ms.ullAvailPhys;
	m_nMemTotalVirtual=ms.ullTotalVirtual;
	m_nMemAvailVirtual=ms.ullAvailVirtual;
	m_nMemTotalPageFile=ms.ullTotalPageFile;
	m_nMemAvailPageFile=ms.ullAvailPageFile;

}
#else

void SystemInfo::update()
{

}

#endif // EW_MSVC


int64_t System::GetMemTotalPhys()
{
	return SystemInfo::current().m_nMemTotalPhys;
}

int64_t System::GetMemAvailPhys()
{
	return SystemInfo::current().m_nMemAvailPhys;
}

int64_t System::GetMemTotalVirtual()
{
	return SystemInfo::current().m_nMemTotalVirtual;
}

int64_t System::GetMemAvailVirtual()
{
	return SystemInfo::current().m_nMemAvailVirtual;
}

void System::Update()
{
	SystemInfo::current().update();
}

int System::GetCpuCount()
{
	return SystemInfo::current().m_nCpuCount;
}

int System::GetPageSize()
{
	return SystemInfo::current().m_nPageSize;
}

int System::GetCacheLineSize()
{
	return SystemInfo::current().m_nCacheLine;
}

String System::GetEnv(const String& name,const String& value_if_not_found)
{
	const char* value=::getenv(name.c_str());
	return value?String(value):value_if_not_found;
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
	::exit(v);
}


#ifdef EW_WINDOWS

String GetModulePathImpl()
{
	char buf[MAX_PATH]={0};
	::GetModuleFileNameA(NULL,buf,MAX_PATH);
	return buf;
}

const String& System::GetModulePath()
{
	static String sModulePath(GetModulePathImpl());
	return sModulePath;
}

#ifdef EW_WINDOWS

class StreamDataProcess : public StreamData
{
public:
	HANDLE hReader, hWriter;
	PROCESS_INFORMATION pi;



	int32_t Read(char* buf,size_t len)
	{
		DWORD nRead(0);
		if(::ReadFile(hReader,buf,len,&nRead,NULL)==FALSE)
		{
			flags.add(FLAG_READ_FAIL_BIT);
			System::CheckError("File::Read Error");
			return -1;
		}
		return nRead;
	}

	int32_t Write(const char* buf,size_t len)
	{
		DWORD nWrite(0);
		if(::WriteFile(hWriter,buf,len,&nWrite,NULL)==FALSE)
		{
			flags.add(FLAG_WRITE_FAIL_BIT);
			System::CheckError("File::Write Error");
			return -1;
		}
		return nWrite;
	}
	StreamDataProcess()
	{
		hReader=hWriter=NULL;
		ZeroMemory(&pi,sizeof(pi));
	}

	bool Execute(const String& s)
	{

		STARTUPINFOA si= {sizeof(STARTUPINFO)};

		StringBuffer<char> sb(s);
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

		si.dwFlags = STARTF_USESTDHANDLES;

		if(!::CreateProcessA(NULL,sb.data(),NULL,NULL,TRUE,0,NULL,NULL,&si,&pi))
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

	~StreamDataProcess()
	{
		::CloseHandle(pi.hProcess);
		::CloseHandle(pi.hThread);
		::CloseHandle(hReader);
		::CloseHandle(hWriter);
	}
};

Stream System::ExecuteRedirect(const String& s,bool* status)
{

	System::LogTrace("System::Exectue:%s", s);

	StreamDataProcess* stream=new StreamDataProcess;
	Stream::impl_type stream_impl(stream);

	if(stream->Execute(s))
	{
		if(status) *status=true;
		return stream_impl;
	}
	else
	{
		if(status) *status=false;
		return Stream();
	}
}

bool System::Execute(const String& s, StringBuffer<char>& result)
{
	bool flag(false);
	Stream stream=ExecuteRedirect(s,&flag);
	if(!flag) return false;
	char buf[1024*4];
	while(1)
	{
		int rc=stream.Read(buf,sizeof(buf));
		if(rc<=0) break;
		result.append(buf,rc);
	}
	return true;

}

#endif


bool System::Execute(const String& s)
{
	STARTUPINFOA si= {sizeof(STARTUPINFO)};
	PROCESS_INFORMATION pi;
	StringBuffer<char> sb(s);
	sb.push_back(0);

	if(!::CreateProcessA(NULL,sb.data(),NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
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

#endif


#ifdef EW_WINDOWS

char* win_strerror(int ret)
{
	char* lpTStr(NULL);
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
				  NULL,
				  ret,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				  (LPSTR)&lpTStr,
				  0x100,
				  NULL);
	return lpTStr;
}

#endif

inline void System_DoCheckErrno(const String& msg)
{
	int ret=errno;
	if(ret!=0)
	{
		System::LogTrace("%s failed, code(%d): %s",msg,ret,strerror(ret));
	}
}

void System::CheckErrno(const String& msg)
{
	System_DoCheckErrno(msg);
}



void System::CheckError(const String& msg)
{
#ifdef EW_WINDOWS

	int ret=::GetLastError();
	if(ret!=0)
	{
		System::LogTrace("%s failed, code(%d): %s",msg,ret,win_strerror(ret));
	}
#else
	System_DoCheckErrno(msg);
#endif
}



extern AtomicSpin g_tSpinConsole;

class SystemLoggerData
{
public:

	SystemLoggerData()
	{
		bEnabled=true;		
	}

	~SystemLoggerData()
	{

	}

	static const char* GetMsgLevel(int lv)
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

	void LogImplV(int lv,const char* msg,va_list arg)
	{

		time_t tt=time(NULL);
		char buf2[1024];
		char buf1[256];
		strftime (buf1,256,"%Y-%m-%d %H:%M:%S ",localtime(&tt));

		::vsnprintf(buf2,1024,msg,arg);

		if(fp_logfile!=NULL)
		{
			LockGuard<AtomicSpin> lock1(spin);
			fprintf(fp_logfile,"%s %s:%s\n",buf1,GetMsgLevel(lv),buf2);
			//fflush(fp_logfile);
		}
		else
		{
			LockGuard<AtomicSpin> lock1(g_tSpinConsole);
			printf("%s %s:%s\n",buf1,GetMsgLevel(lv),buf2);
		}

		if(lv==LOGLEVEL_FATAL)
		{
			System::Exit(-1);
		}
	}

	bool SetLogFile(const char* fn,bool app)
	{
		if(fp_logfile)
		{
			::fclose(fp_logfile);
			fp_logfile=NULL;
		}

		if(fn[0]==0)
		{
			return true;
		}

		fp_logfile=::fopen(fn,app?"a":"w");
		return fp_logfile!=NULL;
	}

	FILE* fp_logfile;
	AtomicSpin spin;

	bool bEnabled;

	static SystemLoggerData& current()
	{
		static SystemLoggerData gInstance;
		return gInstance;
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


void System::DoLog(int lv,const char* msg,...)
{
	if(!SystemLoggerData::current().bEnabled)
	{
		return;
	}

	va_list arglist;
	va_start(arglist,msg);
	SystemLoggerData::current().LogImplV(lv,msg,arglist);
	va_end(arglist);
}


void DllModule::Close()
{
	impl.close();
}

#ifdef EW_WINDOWS

void KO_Policy_module::destroy(type& o)
{
	::FreeLibrary(*(HMODULE*)&o);
}

bool DllModule::Open(const String& dll)
{
	HMODULE p=::LoadLibraryA(dll.c_str());
	if(!p)
	{
		return false;
	}
	impl.reset(p);
	return true;
}



void* DllModule::GetSymbol(const String& s)
{
#ifdef EW_MSVC
	if(!impl.ok()) return NULL;
	return ::GetProcAddress(*(HMODULE*)&impl,s.c_str());
#else
    return NULL;
#endif
}

#else

void KO_Policy_module::destroy(type& o)
{
	dlclose(o);
}

bool DllModule::Open(const String& dll)
{
	void* p=dlopen(dll.c_str(),RTLD_LAZY);
	if(!p)
	{
		return false;
	}
	impl.reset(p);
	return true;
}

void* DllModule::GetSymbol(const String& s)
{
	if(!impl.ok()) return NULL;
	return dlsym(impl,s.c_str());
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

String System::MakePath(const String& file,const String& path)
{
	if(IsRelative(file))
	{
		return AdjustPath(path,true)+file;
	}
	else
	{
		return file;
	}
}

void PushFindItem(arr_1t<FindItem>& files,WIN32_FIND_DATAA& p)
{
	FindItem item;
	item.filename=p.cFileName;
	item.filesize=(uint64_t(p.nFileSizeHigh)<<32)|p.nFileSizeLow;
	item.flags.set(FindItem::IS_FOLDER,(p.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0);
	if(item.filename=="."||item.filename=="..") return;
	files.push_back(item);
}


arr_1t<FindItem> System::FindAllFiles(const String& folder, const String& pattern)
{
	arr_1t<FindItem> files;

#ifdef EW_WINDOWS


	String folder_pattern;
	if(pattern!="")
	{
		folder_pattern = AdjustPath(folder,true)+pattern;
	}
	else
	{
		folder_pattern = folder;
	}

	FILE_ATTRIBUTE_DIRECTORY;
	WIN32_FIND_DATAA p;
	HANDLE h = FindFirstFileA(folder_pattern.c_str(), &p);

	if (h != INVALID_HANDLE_VALUE)
	{
		PushFindItem(files,p);
		while (FindNextFileA(h, &p))
			PushFindItem(files,p);
	}
#endif

	return files;
}

EW_LEAVE
