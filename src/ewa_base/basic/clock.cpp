#include "ewa_base/basic/lockguard.h"
#include "ewa_base/basic/clock.h"
#include "ewa_base/basic/atomic.h"

#include <ctime>


#ifdef EW_WINDOWS
#include <windows.h>
#else
#include <sys/time.h>
#endif

EW_ENTER


#ifdef EW_WINDOWS

#define _W32_FT_OFFSET (116444736000000000)

TimePoint Clock::now()
{
	union
	{
		int64_t  ns100; /*time since 1 Jan 1601 in 100ns units */
		FILETIME ft;
	} _tnow;

	GetSystemTimeAsFileTime(&_tnow.ft);
	return TimePoint((int64_t)((_tnow.ns100 - _W32_FT_OFFSET)/10));
}


#else

TimePoint Clock::now()
{
	timeval tv;
	gettimeofday(&tv,NULL);
	return TimePoint(tv.tv_sec*1000000+tv.tv_usec);
}
#endif

const TimePoint Clock::tpStarted(now());

String TimePoint::Format(const String& f) const
{
	TimeDetail td(*this);
	return td.Format(f);
}

TimePoint::TimePoint(const String& s)
{
	if (Parse(s))
	{
		return;
	}

	Exception::XError("invalid timepoint expr");
	
}

bool TimePoint::Parse(const String& s)
{
	TimeDetail td;
	if(!td.Parse(s)) return false;
	(*this)=td.GetTimePoint();
	return true;
}

TimePoint& TimePoint::operator+=(const TimeSpan& span)
{
	val+=span.val;
	return *this;
}


AtomicSpin g_time_mutex;


TimeDetail::TimeDetail(const TimePoint& tp_,int t)
{
	Parse(tp_,t);
}

TimeDetail::TimeDetail(int t)
{
	Parse(Clock::now(),t);
}

int TimeDetail::GetYear() const
{
	return tk.tm_year+1900;
}

int TimeDetail::GetMonth() const
{
	return tk.tm_mon+1;
}

int TimeDetail::GetDay() const
{
	return tk.tm_mday;
}

int TimeDetail::GetHour() const
{
	return tk.tm_hour;
}

int TimeDetail::GetMinute() const
{
	return tk.tm_min;
}

int TimeDetail::GetSecond() const
{
	return tk.tm_sec;
}

int TimeDetail::GetMilliSecond() const
{
	return ms;
}

void TimeDetail::SetYear(int y)
{
	if(y>1900) y-=1900;
	tk.tm_year=y;
}
void TimeDetail::SetMonth(int m)
{
	tk.tm_mon=m-1;
}

void TimeDetail::SetDay(int d)
{
	tk.tm_mday=d;
}

void TimeDetail::SetHour(int h)
{
	tk.tm_hour=h;
}

void TimeDetail::SetMinute(int m)
{
	tk.tm_min=m;
}

void TimeDetail::SetSecond(int s)
{
	tk.tm_sec=s;
}

void TimeDetail::SetMilliSecond(int n)
{
	ms=n;
}

String TimeDetail::Format(const String& f) const
{
	char buffer[256];
	if(f=="")
	{
		strftime (buffer,256,"%Y-%m-%d %H:%M:%S",&tk);
	}
	else
	{
		strftime (buffer,256,f.c_str(),&tk);
	}
	return buffer;
}

static inline bool read_number(const char*& sp,int& val)
{
	const char* p1=sp;
	val=0;
	while(::isdigit(sp[0]))
	{
		val=val*10+sp[0]-'0';
		sp++;
	}
	if(p1==sp) return false;
	return true;
}


bool TimeDetail::Parse(const String& ts,int t)
{

	struct tm tmp;
	int yy = GetYear(), mm = GetMonth(), dd = GetDay() , hh = 0, mu = 0, sc = 0;
	
	size_t nl = ts.size();

	const char* s1 = ::strstr(ts.c_str(), "/");
	const char* s2 = ::strstr(ts.c_str(), ":");

	if (s1 == NULL)
	{
		s1=::strstr(ts.c_str(), "-");
	}


	if (s1>s2)
	{
		char k1,k2;
		const char* sp=ts.c_str();
		if(!read_number(sp,yy)) return false;
		k1=*sp++;
		if(!read_number(sp,mm)) return false;
		if(k1!=*sp++) return false;
		if(!read_number(sp,dd)) return false;
		sp++;
		if(s2 && read_number(sp,hh))
		{
			k2=*sp++;
			if(!read_number(sp,mu)) return false;
			if(k2!=*sp++) return false;
			if(!read_number(sp,sc)) return false;
		}
	}
	else if (s2>s1)
	{
		char k1, k2;
		const char* sp = ts.c_str();
		if (!read_number(sp, hh)) return false;
		k2 = *sp++;
		if (!read_number(sp, mu)) return false;
		if (k2 != *sp++) return false;
		if (!read_number(sp, sc)) return false;

		if (s1 && read_number(sp, yy))
		{
			k1 = *sp++;
			if (!read_number(sp, mm)) return false;
			if (k1 != *sp++) return false;
			if (!read_number(sp, dd)) return false;
		}
	
	}
	else if (nl == 8)
	{
		if (!ts.substr(0, 4).ToNumber(&yy) || !ts.substr(4, 2).ToNumber(&mm) || !ts.substr(6, 2).ToNumber(&dd))
		{
			return false;
		}
	}
	else if (nl == 12)
	{
		if (!ts.substr(0, 2).ToNumber(&yy) || !ts.substr(2, 2).ToNumber(&mm) || !ts.substr(4, 2).ToNumber(&dd))
		{
			return false;
		}
		if (!ts.substr(6, 2).ToNumber(&hh) || !ts.substr(8, 2).ToNumber(&mu) || !ts.substr(10, 2).ToNumber(&sc))
		{
			return false;
		}
	}
	else if (nl == 14)
	{
		if (!ts.substr(0, 4).ToNumber(&yy) || !ts.substr(4, 2).ToNumber(&mm) || !ts.substr(6, 2).ToNumber(&dd))
		{
			return false;
		}
		if (!ts.substr(8, 2).ToNumber(&hh) || !ts.substr(10, 2).ToNumber(&mu) || !ts.substr(12, 2).ToNumber(&sc))
		{
			return false;
		}
	}
	else
	{
		return false;
	}


	if(yy>1900)
	{
		tmp.tm_year=yy-1900;
	}
	else if(yy<99&&yy>49)
	{
		tmp.tm_year=yy;
	}
	else if(yy<30)
	{
		tmp.tm_year=yy+100;
	}
	else
	{
		return false;
	}

	if(mm<1||mm>12)
	{
		return false;
	}
	tmp.tm_mon=mm-1;

	if(dd<1||dd>31)
	{
		return false;
	}

	tmp.tm_mday=dd;
	tmp.tm_hour=hh;
	tmp.tm_min=mu;
	tmp.tm_sec=sc;

	time_t tt=mktime(&tmp);
	TimePoint tp(tt*1000000);

	return Parse(tp,t);
}

bool TimeDetail::Parse(const TimePoint& tp,int t)
{

	time_t tt=tp.val/1000000;
	ms=(tp.val/1000)%1000;

#ifdef EW_MSVC
	if(t==UTC)
	{
		_gmtime64_s(&tk,&tt);
	}
	else
	{
		_localtime64_s(&tk,&tt);
	}
#else
	g_time_mutex.lock();

	struct tm* pk;
	if(t==UTC)
	{
		pk=gmtime(&tt);
	}
	else
	{
		pk=localtime(&tt);
	}
	memcpy(&tk,pk,sizeof(struct tm));

	g_time_mutex.unlock();
#endif

	return true;
}

TimePoint TimeDetail::GetTimePoint() const
{
	time_t tt=mktime(&tk);
	TimePoint tp(tt*1000000);
	return tp;
}

EW_LEAVE
