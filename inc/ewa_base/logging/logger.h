
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
	};

	Logger(int src=0,int id=0);
	Logger(const String& src,int id=0);
	Logger(const Logger& o);
	Logger& operator=(const Logger& o);

	~Logger();

	void swap(Logger& logger)
	{
		std::swap(impl,logger.impl);
	}

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
	bool Test(int t=MSG_IF_ANY);

	//return true if no error logrecords.
	bool Ok();

	//clear error
	void Clear();

	void DoLog(int v,const String&);

#ifdef EW_C11
	void DoLog(int v,String&&);
#endif

#define STRING_FORMAT_LEVEL1(X,Y) DoLog(Y,X)
#define STRING_FORMAT_LEVEL2(X,Y) DoLog(Y,String::Format(X))

#ifndef NDEBUG
	STRING_FORMAT_FUNCTIONS_2(void LogDebug,STRING_FORMAT_LEVEL1,STRING_FORMAT_LEVEL2,LOGLEVEL_DEBUG)
#else
	inline void LogDebug(...) {}
#endif

	STRING_FORMAT_FUNCTIONS_2(void LogInfo,STRING_FORMAT_LEVEL1,STRING_FORMAT_LEVEL2,LOGLEVEL_INFO)
	STRING_FORMAT_FUNCTIONS_2(void LogTrace,STRING_FORMAT_LEVEL1,STRING_FORMAT_LEVEL2,LOGLEVEL_TRACE)
	STRING_FORMAT_FUNCTIONS_2(void Print,STRING_FORMAT_LEVEL1,STRING_FORMAT_LEVEL2,LOGLEVEL_PRINT)
	STRING_FORMAT_FUNCTIONS_2(void PrintLn,STRING_FORMAT_LEVEL1,STRING_FORMAT_LEVEL2,LOGLEVEL_PRINTLN)
	STRING_FORMAT_FUNCTIONS_2(void LogMessage,STRING_FORMAT_LEVEL1,STRING_FORMAT_LEVEL2,LOGLEVEL_MESSAGE)
	STRING_FORMAT_FUNCTIONS_2(void LogCommand,STRING_FORMAT_LEVEL1,STRING_FORMAT_LEVEL2,LOGLEVEL_COMMAND)
	STRING_FORMAT_FUNCTIONS_2(void LogWarning,STRING_FORMAT_LEVEL1,STRING_FORMAT_LEVEL2,LOGLEVEL_WARNING)
	STRING_FORMAT_FUNCTIONS_2(void LogError,STRING_FORMAT_LEVEL1,STRING_FORMAT_LEVEL2,LOGLEVEL_ERROR)
	STRING_FORMAT_FUNCTIONS_2(void LogFatal,STRING_FORMAT_LEVEL1,STRING_FORMAT_LEVEL2,LOGLEVEL_FATAL)

#undef STRING_FORMAT_LEVEL1
#undef STRING_FORMAT_LEVEL2

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
