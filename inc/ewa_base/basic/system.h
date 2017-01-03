#ifndef __H_EW_BASIC_SYSTEM__
#define __H_EW_BASIC_SYSTEM__

#include "ewa_base/config.h"
#include "ewa_base/basic/string_detail.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/platform.h"
#include "ewa_base/basic/bitflags.h"
#include "ewa_base/basic/stream.h"
#include "ewa_base/basic/file.h"
#include "ewa_base/basic/exception.h"

#define EW_FUNCTION_TRACER(lv) FunctionTracer __function_tracer(__FUNCTION__,lv);

EW_ENTER

class DLLIMPEXP_EWA_BASE System
{
public:

	static bool Execute(const String& s);
	static bool Execute(const String& s,StringBuffer<char>& result);

	static Stream ExecuteRedirect(const String& s,bool* status=NULL);

	static const String& GetModulePath();

	static int GetCacheLineSize();
	static int GetCpuCount();
	static int GetPageSize();

	static int GetPid();

	static double GetCpuTime();

	// Debug break.
	static void DebugBreak();

	// Exit process with return code v.
	static void Exit(int v);

	static void CheckErrno(const String& s);
	static void CheckError(const String& s);

	static void SetLastError(const String& msg);
	static String GetLastError();

	static void Update();

	static int64_t GetMemTotalPhys();
	static int64_t GetMemAvailPhys();
	static int64_t GetMemTotalVirtual();
	static int64_t GetMemAvailVirtual();

	static String GetUsername();
	static String GetEnv(const String& name,const String& value_if_not_found="");

	static bool IsPathSep(char ch);
	static char GetPathSep();
	static String AdjustPath(const String& path,bool sep);
	static String MakePath(const String& file,const String& path);
	static bool IsRelative(const String& file);

	static String GetCwd();
	static bool SetCwd(const String& s);

	static size_t Backtrace(void** stack,size_t frames);


#define STRING_FORMAT_LEVEL(X,Y) DoLog(Y,X)

#ifndef NDEBUG
	STRING_FORMAT_FUNCTIONS(static void LogDebug,STRING_FORMAT_LEVEL,LOGLEVEL_DEBUG)
#else
	static inline void LogDebug(...) {}
#endif

	STRING_FORMAT_FUNCTIONS(static void LogInfo,STRING_FORMAT_LEVEL,LOGLEVEL_INFO)
	STRING_FORMAT_FUNCTIONS(static void LogTrace,STRING_FORMAT_LEVEL,LOGLEVEL_TRACE)
	STRING_FORMAT_FUNCTIONS(static void Printf,STRING_FORMAT_LEVEL,LOGLEVEL_PRINT)
	STRING_FORMAT_FUNCTIONS(static void LogMessage,STRING_FORMAT_LEVEL,LOGLEVEL_MESSAGE)
	STRING_FORMAT_FUNCTIONS(static void LogWarning,STRING_FORMAT_LEVEL,LOGLEVEL_WARNING)
	STRING_FORMAT_FUNCTIONS(static void LogError,STRING_FORMAT_LEVEL,LOGLEVEL_ERROR)
	STRING_FORMAT_FUNCTIONS(static void LogFatal,STRING_FORMAT_LEVEL,LOGLEVEL_FATAL)
#undef STRING_FORMAT_LEVEL

	static bool SetLogFile(const String& fn,bool app=true);

	// default is enabled
	static void SetLogEnable(bool f);

	static void DoLog(int v,const char* msg,...);

};


class FunctionTracer
{
public:
	FunctionTracer(const char* s,int lv=LOGLEVEL_TRACE);
	~FunctionTracer();
private:
	const void* func;
	int level;
};


class KO_Policy_module
{
public:
	typedef void* type;
	typedef type const_reference;
	static type invalid_value(){return NULL;}
	static void destroy(type& o);
	static type open(const String& file);
};

class DLLIMPEXP_EWA_BASE DllModule : public Object
{
public:
	typedef KO_Handle<KO_Policy_module> impl_type;

	DllModule();

	bool Open(const String& dll,int f=0);
	void Close();
	void* GetSymbol(const String& s);

	bool IsOk(){return impl.ok();}

	template<typename T>
	T* GetSymbolEx(const String& name, T*& func){ func = (T*)GetSymbol(name); return func; }

	enum
	{
		FLAG_INITAL_OPEN				=1<<0,
		FLAG_ON_SYMBOL_NOT_FOUND_CLOSE	=1<<1,
		FLAG_SEARCH_PATHS				=1<<2,
	};

protected:
	impl_type impl;
	BitFlags flags;
};

EW_LEAVE
#endif
