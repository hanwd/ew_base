
#ifndef __H_EW_LOG_LOGGING__
#define __H_EW_LOG_LOGGING__

#include "ewa_base/logging/logrecord.h"
#include "ewa_base/collection/indexer_set.h"
#include "ewa_base/basic/object.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE LogTarget;
class DLLIMPEXP_EWA_BASE LoggerImpl;

class DLLIMPEXP_EWA_BASE Logger : public Object
{
public:

	enum
	{
		MSG_IF_ANY,
		MSG_IF_WARNING,
		MSG_IF_ERROR,
		MSG_NEVER,
		MSG_MUTED=1<<4,
	};

	Logger(int src=0,int id=0);
	Logger(const String& src,int id=0);
	Logger(const Logger& o);
	Logger& operator=(const Logger& o);

	~Logger();

	void swap(Logger& logger);

	// set/get the default logtarget.
	static void def(LogTarget* p);
	static LogTarget* def();

	// set/get logtarget.
	void reset(LogTarget* p);
	LogTarget* get();

	// set/get source
	void Src(const String& src);
	void Src(int src);
	int Src();

	// set/get id
	int Id();
	void Id(int id);

	// cache mode, LogRecords will not be send to logtarget immediately.
	bool Cache();
	void Cache(bool cache);

	// handle the logrecord, just send to the logtarget.
	void Handle(LogRecord& o);

	// flush cached logrecords to logtarget, return Ok() and Clear();
	bool Test(int t=MSG_IF_ANY,const String& title="");

	//return true if no error logrecords.
	bool Ok();

	//clear error
	void Clear();

	void DoLog(int v,const String&);

#ifdef EW_C11
	void DoLog(int v,String&&);
#endif


#ifndef NDEBUG
	STRING_FORMATER_FORMAT_FUNCS_SB(void LogDebug,DoLog(LOGLEVEL_DEBUG,fb);)
#else
	inline void LogDebug(...) {}
#endif

	STRING_FORMATER_FORMAT_FUNCS_SB(void LogInfo,DoLog(LOGLEVEL_INFO,fb);)
	STRING_FORMATER_FORMAT_FUNCS_SB(void LogTrace,DoLog(LOGLEVEL_TRACE,fb);)
	STRING_FORMATER_FORMAT_FUNCS_SB(void Print,DoLog(LOGLEVEL_PRINT,fb);)
	STRING_FORMATER_FORMAT_FUNCS_SB(void PrintLn,DoLog(LOGLEVEL_PRINTLN,fb);)
	STRING_FORMATER_FORMAT_FUNCS_SB(void LogMessage,DoLog(LOGLEVEL_MESSAGE,fb);)
	STRING_FORMATER_FORMAT_FUNCS_SB(void LogCommand,DoLog(LOGLEVEL_COMMAND,fb);)
	STRING_FORMATER_FORMAT_FUNCS_SB(void LogWarning,DoLog(LOGLEVEL_WARNING,fb);)
	STRING_FORMATER_FORMAT_FUNCS_SB(void LogError,DoLog(LOGLEVEL_ERROR,fb);)
	STRING_FORMATER_FORMAT_FUNCS_SB(void LogFatal,DoLog(LOGLEVEL_FATAL,fb);)


protected:

	LoggerImpl* impl;
};

// get thread private logger;
DLLIMPEXP_EWA_BASE Logger& this_logger();



// LoggerSwap, swap logger with this_logger().
class DLLIMPEXP_EWA_BASE LoggerSwap : public Logger, private NonCopyable
{
public:
	LoggerSwap();
	LoggerSwap(LogTarget* p);
	LoggerSwap(Logger& logger);
	~LoggerSwap();
};

// LoggerSwap, swap logger with this_logger().
class DLLIMPEXP_EWA_BASE LoggerAuto : public Logger, private NonCopyable
{
public:
	LoggerAuto();
	~LoggerAuto();
	Logger logger_old;
};


class DLLIMPEXP_EWA_BASE LogSource
{
public:

	LogSource();

	size_t size();
	int get(const String& s);

	String operator[](size_t i);

	static LogSource& current();

protected:
	indexer_set<String> m_aSources;
	AtomicSpin spin;
};


EW_LEAVE
#endif
