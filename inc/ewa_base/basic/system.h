#ifndef __H_EW_BASIC_SYSTEM__
#define __H_EW_BASIC_SYSTEM__

#include "ewa_base/config.h"
#include "ewa_base/basic/string_detail.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/platform.h"

#define EW_FUNCTION_TRACER(lv) FunctionTracer __function_tracer(__FUNCTION__,lv);

EW_ENTER


class DLLIMPEXP_EWA_BASE System
{
public:

	static bool Execute(const String& s);
	static bool Execute(const String& s,StringBuffer<char>& result);

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

	static void Update();

	static int64_t GetMemTotalPhys();
	static int64_t GetMemAvailPhys();
	static int64_t GetMemTotalVirtual();
	static int64_t GetMemAvailVirtual();

	static String GetEnv(const String& name);

	static arr_1t<String> FindAllFiles(const String& folder, const String& pattern = "*.*");


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
};

class DLLIMPEXP_EWA_BASE DllModule
{
public:
	typedef KO_Handle<KO_Policy_module> impl_type;

	bool Open(const String& dll);
	void Close();

	void* GetSymbol(const String& s);

	bool IsOpen(){return impl.ok();}

protected:
	impl_type impl;
};

EW_LEAVE
#endif
