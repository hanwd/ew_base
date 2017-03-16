#include "ewa_base/logging/logger.h"
#include "ewa_base/logging/logtarget.h"
#include "ewa_base/basic/pointer.h"
#include "ewa_base/serialization/serializer.h"
#include "ewa_base/basic/lockguard.h"
#include "ewa_base/basic/atomic.h"

#include <cstdarg>

#ifndef EW_WINDOWS
#include <errno.h>
#endif


EW_ENTER

LogRecord::LogRecord()
{
	m_tStamp=Clock::now();
	m_nId=0;
	m_nSrc=0;
	m_nLevel=LOGLEVEL_MESSAGE;

}

LogRecord::LogRecord(const String& s,int src,int id,int lv)
	:m_sMessage(s)
	,m_nSrc(src)
	,m_nId(id)
	,m_nLevel(lv)
{
	m_tStamp=Clock::now();
}

#ifdef EW_C11
LogRecord::LogRecord(String&& s,int src,int id,int lv)
	:m_sMessage(s)
	,m_nSrc(src)
	,m_nId(id)
	,m_nLevel(lv)
{
	m_tStamp=Clock::now();
}

#endif

LogRecord::LogRecord(int src,int id,int lv)
	:m_nSrc(src)
	,m_nId(id)
	,m_nLevel(lv)
{
	m_tStamp=Clock::now();
}

void LogRecord::Serialize(SerializerHelper sh)
{
	Serializer& ar(sh.ref(0));
	ar & m_tStamp & m_sMessage & m_nSrc & m_nId & m_nLevel;
}

class LoggerImpl : public ObjectT<LogTarget,ObjectData>
{
public:
	enum
	{
		FLAG_CACHED=1<<0,
	};


	static LogPtr* def()
	{		
		class DefaultLogPtr : public DataPtrT<LogPtr>
		{
		public:
			DefaultLogPtr()
			{
				reset(new LogPtr);
				m_ptr->SetData(new LogConsole);
			}
		};
		static DefaultLogPtr gInstance;
		return gInstance.get();
	}

	arr_1t<LogRecord> m_aMsg;
	int m_nId;
	int m_nSrc;

	int m_nErrCount;
	int m_nWrnCount;

	BitFlags flags;

	LoggerImpl(int src=0,int id=0)
		:m_nId(id)
		,m_nSrc(src)
		,m_nErrCount(0)
		,m_nWrnCount(0)
	{
		SetData(def());
	}

	~LoggerImpl()
	{

	}

	inline void Handle(LogRecord& o)
	{
		if(o.m_nLevel==LOGLEVEL_WARNING)
		{
			m_nWrnCount++;
		}
		if(o.m_nLevel>LOGLEVEL_WARNING)
		{
			m_nErrCount++;
		}

		if(flags.get(FLAG_CACHED))
		{
			m_aMsg.push_back(o);
		}
		else if(m_refData)
		{
			m_refData->Handle(o);
		}
	}

};

void Logger::DoLog(int lv, const String& msg)
{
	if(!impl) return;
	LogRecord rcd(msg,impl->m_nSrc,impl->m_nId,lv);
	Handle(rcd);
}

#ifdef EW_C11
void Logger::DoLog(int lv, String&& msg)
{
	LogRecord rcd(msg,impl->m_nSrc,impl->m_nId,lv);
	Handle(rcd);
}
#endif

void Logger::Handle(LogRecord& o)
{
	try
	{
		impl->Handle(o);
	}
	catch(...)
	{
		System::LogError("Unhandled exception caught");
	}
}

Logger::Logger(int src,int id)
{
	impl=new LoggerImpl(src,id);
	impl->IncRef();
}

Logger::Logger(const String& src,int id)
{
	impl=new LoggerImpl(LogSource::current().get(src),id);
	impl->IncRef();
}

Logger::Logger(const Logger& o):impl(NULL)
{
	ObjectData::locked_reset(impl,o.impl);
}

Logger& Logger::operator=(const Logger& o)
{
	ObjectData::locked_reset(impl,o.impl);
	return *this;
}

int Logger::Src()
{
	return impl->m_nSrc;
}

void Logger::Src(const String& src)
{
	Src(LogSource::current().get(src));
}

void Logger::Src(int src)
{
	impl->m_nSrc=src;
}

int Logger::Id()
{
	return impl->m_nId;
}

void Logger::Id(int id)
{
	impl->m_nId=id;
}

bool Logger::Cache()
{
	return impl->flags.get(LoggerImpl::FLAG_CACHED);
}

void Logger::Cache(bool c)
{
	impl->flags.set(LoggerImpl::FLAG_CACHED,c);
}

Logger::~Logger()
{
	impl->DecRef();
}

LogTarget* Logger::get()
{
	return impl->GetData();
}

void Logger::reset(LogTarget* p)
{
	if(!p) p=new LogNull();
	return impl->SetData(p);
}

bool Logger::Test(int t)
{

	bool needsend=true;

	switch(t)
	{
	case MSG_IF_ANY:
		needsend=true;
		break;
	case MSG_IF_WARNING:
		needsend=(impl->m_nErrCount|impl->m_nErrCount)!=0;
		break;
	case MSG_IF_ERROR:
		needsend=impl->m_nErrCount!=0;
		break;
	default:
		needsend=false;
	};


	if(needsend && impl->GetData())
	{
		for(size_t i=0; i<impl->m_aMsg.size(); i++)
		{
			impl->GetData()->Handle(impl->m_aMsg[i]);
		}
	}

	bool flag=Ok();
	Clear();
	return flag;
}

bool Logger::Ok()
{
	return impl->m_nErrCount==0;
}

void Logger::Clear()
{
	impl->m_nErrCount=0;
	impl->m_nWrnCount=0;
	impl->m_aMsg.clear();
}

void Logger::def(LogTarget* p)
{
	LoggerImpl::def()->SetData(p);
}

LogTarget* Logger::def()
{
	return LoggerImpl::def()->GetData();
}


LoggerSwap::LoggerSwap()
{
	reset(new LogNull);
	swap(this_logger());
}

LoggerSwap::LoggerSwap(LogTarget* p)
{
	reset(p);
	swap(this_logger());
}

LoggerSwap::LoggerSwap(Logger& logger):Logger(logger)
{
	swap(this_logger());
}

LoggerSwap::~LoggerSwap()
{
	swap(this_logger());
}


LogSource::LogSource()
{
	m_aSources.insert("Common");
}

size_t LogSource::size()
{
	LockGuard<AtomicSpin> lock(spin);
	return m_aSources.size();
}

int LogSource::get(const String& s)
{
	LockGuard<AtomicSpin> lock(spin);
	return m_aSources.find2(s);
}

String LogSource::operator[](size_t i)
{
	LockGuard<AtomicSpin> lock(spin);
	return m_aSources.get(i);
}

LogSource& LogSource::current()
{
	static LogSource gInstance;
	return gInstance;
}

EW_LEAVE
