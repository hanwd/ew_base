
#ifndef __H_EW_LOG_TARGET__
#define __H_EW_LOG_TARGET__


#include "ewa_base/basic/pointer.h"
#include "ewa_base/basic/object.h"
#include "ewa_base/basic/bitflags.h"
#include "ewa_base/basic/file.h"
#include "ewa_base/collection/arr_1t.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE LogRecord;

class DLLIMPEXP_EWA_BASE LogTarget : public ObjectData
{
public:

	enum
	{
		FLAG_NEWLINE	=1<<0,
		FLAG_SHOWALL	=1<<1,
		FLAG_SHOWDATE	=1<<2,
		FLAG_SHOWTIME	=1<<3,
		FLAG_SHOWRANK	=1<<4,
		FLAG_COLORED	=1<<8,
	};

	LogTarget();

	// Handle the record
	virtual void Handle(const LogRecord& o)=0;

	// cast logrecord to string
	virtual String Format(const LogRecord& o) const;

	// flush if cached.
	virtual void Flush();

	BitFlags flags;
};

// LogNull, ignore all messages.
class DLLIMPEXP_EWA_BASE LogNull : public LogTarget
{
public:
	virtual void Handle(const LogRecord&) {}
};

// LogConsole, display messages in console window.
class DLLIMPEXP_EWA_BASE LogConsole : public LogTarget
{
public:
	LogConsole();
	virtual void Handle(const LogRecord&);

protected:
	arr_1t<int> aColors;
};

// LogFile, write messages to file.
// if not append mode, file will be truncated
class DLLIMPEXP_EWA_BASE LogFile : public LogTarget
{
public:

	LogFile(const String& filename,bool app=true);
	~LogFile();

	virtual void Handle(const LogRecord&);
	bool good(){return fp.good();}

protected:
	File fp;
	AtomicSpin spin;
};


class DLLIMPEXP_EWA_BASE LogCache : public LogTarget
{
public:
	arr_1t<LogRecord> aRecords;

	void Serialize(Serializer& ar);
	virtual void Handle(const LogRecord& o);

	DECLARE_OBJECT_INFO(LogCache,ObjectInfo);
};

// LogFile, redirect messages to m_refData.
class DLLIMPEXP_EWA_BASE LogPtr : public ObjectT<LogTarget,LogTarget>
{
public:

	void Handle(const LogRecord& o);
};


EW_LEAVE
#endif
