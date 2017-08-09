#include "ewa_base/logging/logtarget.h"
#include "ewa_base/logging/logrecord.h"
#include "ewa_base/basic/lockguard.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/serialization/serializer.h"

#include <cstdlib>
#include <cstdio>

EW_ENTER

LogTarget::LogTarget()
{
	flags.clr(FLAG_SHOWTIME);
}

void LogTarget::Flush()
{

}

String LogTarget::Format(const LogRecord& o) const
{
	if(o.m_nLevel==LOGLEVEL_PRINT||o.m_nLevel==LOGLEVEL_PRINTLN)
	{
		return o.m_sMessage;
	}

	String msg;

	if (flags.get(FLAG_SHOWRANK))
	{
		msg.Printf("[%2d]", o.m_nId);
	}

	if(flags.get(FLAG_SHOWDATE))
	{
		if(flags.get(FLAG_SHOWTIME))
		{
			msg << o.m_tStamp.Format("%Y-%m-%d %H:%M:%S");
		}
		else
		{
			msg << o.m_tStamp.Format("%Y-%m-%d");
		}
	}
	else if(flags.get(FLAG_SHOWTIME))
	{
		msg << o.m_tStamp.Format("%H:%M:%S");
	}


	char _nLevel=' ';
	switch(o.m_nLevel)
	{
	case LOGLEVEL_MESSAGE:
		_nLevel='M';
		break;
	case LOGLEVEL_WARNING:
		_nLevel='W';
		break;
	case LOGLEVEL_LITE_ERROR:
		_nLevel='E';
		break;
	case LOGLEVEL_ERROR:
		_nLevel='E';
		break;
	default:
		_nLevel='O';
		break;
	};

	msg<<String::Format("[%c]",_nLevel);
	msg<<":";
	msg<<o.m_sMessage;

	return msg;

}


LogFile::LogFile(const String& filename,bool app)
{
	int flag=FLAG_FILE_WC;
	if(app) flag|=FLAG_FILE_APPEND;
	fp.open(filename,flag);
	if(!app)
	{
		fp.truncate(0);
	}
}

LogFile::~LogFile()
{

}

void LogFile::Handle(const LogRecord& o)
{

	LockGuard<AtomicSpin> lock1(spin);

	String s;
	try
	{
		s=Format(o);
	}
	catch(...)
	{
		System::LogError("Unknown exception in LogFile::Handle");
		return;
	}

	fp.write(s.c_str(),s.size());
	fp.write("\r\n",2);
	fp.flush();


}

void LogPtr::Handle(const LogRecord& o)
{
	if(m_refData) m_refData->Handle(o);
}

void LogCache::Serialize(SerializerHelper sh)
{

}

void LogCache::Handle(const LogRecord& o)
{
	aRecords.push_back(o);
}

IMPLEMENT_OBJECT_INFO(LogCache,ObjectInfo);

EW_LEAVE
