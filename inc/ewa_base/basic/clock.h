#ifndef __H_EW_BASIC_CLOCK__
#define __H_EW_BASIC_CLOCK__

#include "ewa_base/config.h"
#include "ewa_base/basic/string.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE String;
class DLLIMPEXP_EWA_BASE TimeSpan;
class DLLIMPEXP_EWA_BASE ElapsedTimer;

// TimePoint
class DLLIMPEXP_EWA_BASE TimePoint
{
public:

	TimePoint():val(0) {}

	explicit TimePoint(const String& s);
	explicit TimePoint(int64_t v):val(v){}

	// cast to time_t
	time_t GetTime() const
	{
		return val/1000000;
	}

	// set from time_t and microseconds
	void SetTime(time_t v,int microseconds=0)
	{
		val=(int64_t)v*1000000+(int64_t)microseconds;
	}

	// cast to string
	String Format(const String& f="") const;

	bool Parse(const String& s);

	// add a timespan
	TimePoint& operator+=(const TimeSpan& span);

	int64_t val;
};

// TimeDetail
class DLLIMPEXP_EWA_BASE TimeDetail
{
public:

	enum
	{
		LOCAL,
		UTC,
	};

	TimeDetail(int t=LOCAL);
	TimeDetail(const TimePoint& tp_,int t=LOCAL);

	int GetYear() const;
	int GetMonth() const;
	int GetDay() const;
	int GetHour() const;
	int GetMinute() const;
	int GetSecond() const;
	int GetMilliSecond() const;

	void SetYear(int y);
	void SetMonth(int m);
	void SetDay(int d);
	void SetHour(int h);
	void SetMinute(int m);
	void SetSecond(int s);
	void SetMilliSecond(int n);

	String Format(const String& f="") const;

	// Parse from string yy-mm-dd[ hh:mm:dd]
	bool Parse(const String& ts_,int t=LOCAL);

	bool Parse(const TimePoint& tp_,int t=LOCAL);

	TimePoint GetTimePoint() const;

	struct tm& GetTm(){return tk;}

private:
	mutable struct tm tk;
	int ms;
};

// TimeSpan
class DLLIMPEXP_EWA_BASE TimeSpan
{
public:
	TimeSpan():val(0) {}
	explicit TimeSpan(int64_t v):val(v) {}

	int64_t GetSeconds() const
	{
		return val/1000000;
	}
	int64_t GetMilliSeconds() const
	{
		return val/1000;
	}
	int64_t GetMicroSeconds() const
	{
		return val;
	}

	static TimeSpan Day(int64_t v)
	{
		return Seconds(60*60*24*v);
	}
	static TimeSpan Hours(int64_t v)
	{
		return Seconds(60*60*v);
	}
	static TimeSpan Minutes(int64_t v)
	{
		return Seconds(60*v);
	}
	static TimeSpan Seconds(int64_t v)
	{
		return TimeSpan(1000*1000*v);
	}
	static TimeSpan MilliSeconds(int64_t v)
	{
		return TimeSpan(1000*v);
	}
	static TimeSpan MicroSeconds(int64_t v)
	{
		return TimeSpan(v);
	}

	int64_t val;
};


#define BOOL_OPERATOR(TYPE,VALUE)\
	inline bool operator <(const TYPE& p1,const TYPE& p2){return p1.VALUE  < p2.VALUE;}\
	inline bool operator<=(const TYPE& p1,const TYPE& p2){return p1.VALUE <= p2.VALUE;}\
	inline bool operator>=(const TYPE& p1,const TYPE& p2){return p1.VALUE >= p2.VALUE;}\
	inline bool operator >(const TYPE& p1,const TYPE& p2){return p1.VALUE  > p2.VALUE;}\
	inline bool operator==(const TYPE& p1,const TYPE& p2){return p1.VALUE == p2.VALUE;}\
	inline bool operator!=(const TYPE& p1,const TYPE& p2){return p1.VALUE != p2.VALUE;}

BOOL_OPERATOR(TimePoint,val)
BOOL_OPERATOR(TimeSpan,val)

// TimeSpan and TimePoint operators

inline double operator/(const TimeSpan& s1,const TimeSpan& s2)
{
	return double(s1.val)/double(s2.val);
}

inline const TimeSpan operator/(const TimeSpan& s1,double v)
{
	return TimeSpan((int64_t)(double(s1.val)/v));
}

inline const TimeSpan operator*(const TimeSpan& s1,double v)
{
	return TimeSpan((int64_t)(double(s1.val)*v));
}

inline const TimeSpan operator*(double v,const TimeSpan& s1)
{
	return TimeSpan((int64_t)(double(s1.val)*v));
}

inline TimeSpan operator-(const TimePoint& p1,const TimePoint& p2)
{
	return TimeSpan(p1.val-p2.val);
}

inline TimePoint operator+(const TimePoint& p1,const TimeSpan& p2)
{
	return TimePoint(p1.val+p2.val);
}

inline TimePoint operator-(const TimePoint& p1,const TimeSpan& p2)
{
	return TimePoint(p1.val-p2.val);
}

inline TimePoint operator+(const TimeSpan& p1,const TimePoint& p2)
{
	return TimePoint(p1.val+p2.val);
}

inline TimeSpan operator+(const TimeSpan& p1,const TimeSpan& p2)
{
	return TimeSpan(p1.val+p2.val);
}

inline TimeSpan operator-(const TimeSpan& p1,const TimeSpan& p2)
{
	return TimeSpan(p1.val-p2.val);
}

class DLLIMPEXP_EWA_BASE Clock
{
public:

	// now() returns current TimePoint
	static TimePoint now();

	// process started time
	static const TimePoint tpStarted;
};


class DLLIMPEXP_EWA_BASE ElapsedTimer
{
public:

	ElapsedTimer()
	{
		tick();
	}

	void tick()
	{
		p1=p2=Clock::now();
	}

	double tack()
	{
		p2=Clock::now();
		return get();
	}

	double get()
	{
		return (p2-p1)/TimeSpan::Seconds(1);
	}

private:
	TimePoint p1;
	TimePoint p2;
};

template<> class hash_t<TimePoint> : public hash_pod<TimePoint> {};
template<> class hash_t<TimeSpan> : public hash_pod<TimeSpan> {};

EW_LEAVE

namespace tl
{
	template<> struct is_pod<ew::TimePoint> : public value_type<true>{};
	template<> struct is_pod<ew::TimeSpan> : public value_type<true>{};
};

#endif
