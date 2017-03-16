
#ifndef __H_EW_LOG_RECORD__
#define __H_EW_LOG_RECORD__

#include "ewa_base/basic/clock.h"
#include "ewa_base/basic/string.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE SerializerHelper;

class DLLIMPEXP_EWA_BASE LogRecord
{
public:
	LogRecord();
	LogRecord(int src,int id,int lv);

	LogRecord(const String& s,int src,int id,int lv);

#ifdef EW_C11
	LogRecord(String&& s,int src,int id,int lv);
#endif

	// timepoint when record generated.
	TimePoint m_tStamp;

	// message description
	String m_sMessage;

	// source of this logrecord.
	int32_t m_nSrc;

	// thread id
	int32_t m_nId;

	// level of the logrecord
	int32_t m_nLevel;

	void Serialize(SerializerHelper sh);
};

EW_LEAVE
#endif
