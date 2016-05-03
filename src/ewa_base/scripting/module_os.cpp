#include "ewa_base/scripting.h"
#include "ewa_base/threading/thread.h"

EW_ENTER

class CallableMetatableTime : public CallableMetatable
{
public:

	CallableMetatableTime(const String& s) :CallableMetatable(s){}

	virtual bool __add(Variant& r, Variant& v1, Variant& v2)
	{
		if (CallableWrapT<TimePoint>* p1 = dynamic_cast<CallableWrapT<TimePoint>*>(v1.kptr()))
		{
			if (CallableWrapT<TimeSpan>* p2 = dynamic_cast<CallableWrapT<TimeSpan>*>(v2.kptr()))
			{
				r.kptr(new CallableWrapT<TimePoint>(p1->value + p2->value));
				return true;
			}
			return false;
		}
		else if (CallableWrapT<TimeSpan>* p1 = dynamic_cast<CallableWrapT<TimeSpan>*>(v1.kptr()))
		{
			if (CallableWrapT<TimePoint>* p2 = dynamic_cast<CallableWrapT<TimePoint>*>(v2.kptr()))
			{
				r.kptr(new CallableWrapT<TimePoint>(p1->value + p2->value));
				return true;
			}
			return false;
		}
		return false;
	}

	virtual bool __sub(Variant& r, Variant& v1, Variant& v2)
	{
		if (CallableWrapT<TimePoint>* p1 = dynamic_cast<CallableWrapT<TimePoint>*>(v1.kptr()))
		{
			if (CallableWrapT<TimePoint>* p2 = dynamic_cast<CallableWrapT<TimePoint>*>(v2.kptr()))
			{
				r.kptr(new CallableWrapT<TimeSpan>(p1->value - p2->value));
				return true;
			}
			else if (CallableWrapT<TimeSpan>* p2 = dynamic_cast<CallableWrapT<TimeSpan>*>(v2.kptr()))
			{
				r.kptr(new CallableWrapT<TimePoint>(p1->value - p2->value));
				return true;
			}
			return false;
		}
		if (CallableWrapT<TimeSpan>* p1 = dynamic_cast<CallableWrapT<TimeSpan>*>(v1.kptr()))
		{
			if (CallableWrapT<TimeSpan>* p2 = dynamic_cast<CallableWrapT<TimeSpan>*>(v2.kptr()))
			{
				r.kptr(new CallableWrapT<TimeSpan>(p1->value - p2->value));
				return true;
			}
			return false;
		}
		return false;
	}

	virtual bool __mul(Variant& r, Variant& v1, Variant& v2)
	{
		if (CallableWrapT<TimeSpan>* p1 = dynamic_cast<CallableWrapT<TimeSpan>*>(v1.kptr()))
		{
			try
			{
				r.kptr(new CallableWrapT<TimeSpan>(p1->value*variant_cast<double>(v2)));
				return true;
			}
			catch (...)
			{
				return false;
			}
		}
		if (CallableWrapT<TimeSpan>* p1 = dynamic_cast<CallableWrapT<TimeSpan>*>(v2.kptr()))
		{
			try
			{
				r.kptr(new CallableWrapT<TimeSpan>(p1->value*variant_cast<double>(v1)));
				return true;
			}
			catch (...)
			{
				return false;
			}
		}
		return false;
	}

	virtual bool __div(Variant& r, Variant& v1, Variant& v2)
	{
		if (CallableWrapT<TimeSpan>* p1 = dynamic_cast<CallableWrapT<TimeSpan>*>(v1.kptr()))
		{
			if (CallableWrapT<TimeSpan>* p2 = dynamic_cast<CallableWrapT<TimeSpan>*>(v2.kptr()))
			{
				r.reset(p1->value / p2->value);
				return true;
			}

			try
			{
				r.kptr(new CallableWrapT<TimeSpan>(p1->value/variant_cast<double>(v2)));
				return true;
			}
			catch (...)
			{
				return false;
			}
		}
		return false;
	}


	template<typename P>
	bool __compare(Variant& r, Variant& v1, Variant& v2)
	{
		if (CallableWrapT<TimeSpan>* p1 = dynamic_cast<CallableWrapT<TimeSpan>*>(v1.kptr()))
		{
			if (CallableWrapT<TimeSpan>* p2 = dynamic_cast<CallableWrapT<TimeSpan>*>(v2.kptr()))
			{
				r.reset(P::g(p1->value.val,p2->value.val));
				return true;
			}

			try
			{
				r.reset(P::g(p1->value / TimeSpan::MilliSeconds(1),variant_cast<double>(v2)));
				return true;
			}
			catch (...)
			{
				return false;
			}
		}
		if (CallableWrapT<TimeSpan>* p1 = dynamic_cast<CallableWrapT<TimeSpan>*>(v2.kptr()))
		{
			try
			{
				r.reset(P::g(variant_cast<double>(v1), p1->value / TimeSpan::MilliSeconds(1)));
				return true;
			}
			catch (...)
			{
				return false;
			}
		}
		if (CallableWrapT<TimePoint>* p1 = dynamic_cast<CallableWrapT<TimePoint>*>(v1.kptr()))
		{
			if (CallableWrapT<TimePoint>* p2 = dynamic_cast<CallableWrapT<TimePoint>*>(v2.kptr()))
			{
				r.reset(P::g(p1->value.val, p2->value.val));
				return true;
			}
		}

		return false;
	}

	virtual bool __gt(Variant& r, Variant& v1, Variant& v2)
	{
		return __compare<pl_gt>(r, v1, v2);
	}
	virtual bool __lt(Variant& r, Variant& v1, Variant& v2)
	{
		return __compare<pl_lt>(r, v1, v2);
	}
	virtual bool __ge(Variant& r, Variant& v1, Variant& v2)
	{
		return __compare<pl_ge>(r, v1, v2);
	}
	virtual bool __le(Variant& r, Variant& v1, Variant& v2)
	{
		return __compare<pl_le>(r, v1, v2);
	}
	virtual bool __ne(Variant& r, Variant& v1, Variant& v2)
	{
		return __compare<pl_ne>(r, v1, v2);
	}
	virtual bool __eq(Variant& r, Variant& v1, Variant& v2)
	{
		return __compare<pl_eq>(r, v1, v2);
	}


};

class CallableMetatableTimePoint : public CallableMetatableTime
{
public:
	CallableMetatableTimePoint() :CallableMetatableTime("timepoint"){}

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.ci0.nbx[1].kptr(new CallableWrapT<TimePoint>(Clock::now()));
		return 1;
	}

	DECLARE_OBJECT_CACHED_INFO(CallableMetatableTimePoint, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableMetatableTimePoint, ObjectInfo);

class CallableMetatableTimeSpan : public CallableMetatableTime
{
public:
	CallableMetatableTimeSpan() :CallableMetatableTime("timespan"){}
	virtual int __fun_call(Executor& ewsl, int pm)
	{
		if (pm == 0)
		{
			ewsl.ci0.nbx[1].kptr(new CallableWrapT<TimeSpan>());
			return 1;
		}
		else if (pm == 1)
		{
			ewsl.ci0.nbx[1].kptr(new CallableWrapT<TimeSpan>(TimeSpan(1000.0*variant_cast<double>(ewsl.ci0.nbx[1]))));
			return 1;
		}

		return CallableData::__fun_call(ewsl, pm);
	}

	DECLARE_OBJECT_CACHED_INFO(CallableMetatableTimeSpan, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableMetatableTimeSpan, ObjectInfo);

CallableMetatable* CallableWrapT<TimePoint>::GetMetaTable()
{
	return CallableMetatableTimePoint::sm_info.CreateObject();
}

CallableMetatable* CallableWrapT<TimeSpan>::GetMetaTable()
{
	return CallableMetatableTimeSpan::sm_info.CreateObject();
}


CallableWrapT<TimePoint>::CallableWrapT()
{
	value=Clock::now();
}

bool CallableWrapT<TimePoint>::ToValue(String& v,int) const 
{
	TimeDetail td(value);
	v=td.Format();
	return true;
}

bool CallableWrapT<TimePoint>::ToValue(int64_t& v) const 
{
	v=value.val;
	return true;
}

bool CallableWrapT<TimePoint>::ToValue(double& v) const 
{
	v=double(value.val)/(TimeSpan::Seconds(1).val);
	return true;
}


bool CallableWrapT<TimeSpan>::ToValue(int64_t& v) const 
{
	v=value/TimeSpan::Seconds(1);
	return true;
}

bool CallableWrapT<TimeSpan>::ToValue(double& v) const 
{
	v=value/TimeSpan::Seconds(1);
	return true;
}

bool CallableWrapT<TimeSpan>::ToValue(String& v,int) const 
{
	double s=value/TimeSpan::Seconds(1);
	double k=s<0?-s:+s;

	if(k<1)
	{
		v.Printf("%4.3lf ms",k*1000.0);
	}
	else if(k<100)
	{
		v.Printf("%4.3lf seconds",k);
	}
	else if(k<3600)
	{
		v.Printf("%4.3lf minutes",k/60.0);
	}
	else if(k<3600*24)
	{
		v.Printf("%4.3lf hours",k/3600.0);
	}
	else
	{
		v.Printf("%4.3lf days",k/3600.0/24.0);
	}

	if(s<0)
	{
		v="-"+v;
	}

	return true;
}


class DLLIMPEXP_EWA_BASE CallableFunctionTime : public CallableFunction
{
public:
	CallableFunctionTime():CallableFunction("os.time",1){}

	int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,0);
		ewsl.ci0.nbx[1].reset(new CallableWrapT<TimePoint>());
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionTime, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionTime, ObjectInfo);

class DLLIMPEXP_EWA_BASE CallableFunctionSleep : public CallableFunction
{
public:
	CallableFunctionSleep():CallableFunction("os.sleep",1){}

	int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		Variant& v(*ewsl.ci1.nsp);

		TimeSpan* ts=v.ptr<TimeSpan>();
		if(ts)
		{
			Thread::sleep_for(*ts);
			return 0;
		}

		TimePoint* tp=ewsl.ci0.nbx[1].ptr<TimePoint>();
		if(tp)
		{
			Thread::sleep_until(*tp);
			return 0;
		}

		Thread::sleep_for(variant_cast<int>(v));
		return 0;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionSleep, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionSleep, ObjectInfo);

class CallableFunctionShell : public CallableFunction
{
public:

	CallableFunctionShell() :CallableFunction("os.shell",1){}

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this, pm, 1);
		String *p = ewsl.ci0.nbx[1].ptr<String>();
		if (!p)
		{
			ewsl.ci0.nbx[1].reset<String>("error:invalid param");
		}
		else
		{
			StringBuffer<char> sb;
			if (System::Execute("cmd /c " + *p, sb))
			{
				ewsl.ci0.nbx[1].reset<String>(sb);
			}
			else
			{
				ewsl.ci0.nbx[1].reset<String>("error:execute failed");
			}
		}

		return 1;

	}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionShell, ObjectInfo);

};

IMPLEMENT_OBJECT_INFO(CallableFunctionShell, ObjectInfo);


class CallableFunctionGetEnv : public CallableFunction
{
public:

	CallableFunctionGetEnv() :CallableFunction("os.getenv",1){}

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this, pm, 1);
		String p = variant_cast<String>(ewsl.ci0.nbx[1]);
		ewsl.ci0.nbx[1].reset(System::GetEnv(p));
		return 1;
	}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionGetEnv, ObjectInfo);

};

IMPLEMENT_OBJECT_INFO(CallableFunctionGetEnv, ObjectInfo);

void init_module_os()
{

	CG_GGVar& gi(CG_GGVar::current());
	gi.add_inner<CallableFunctionTime>();
	gi.add_inner<CallableFunctionSleep>();
	gi.add_inner<CallableFunctionShell>();
	gi.add_inner<CallableFunctionGetEnv>();

	gi.add_inner<CallableMetatableTimeSpan>();
	gi.add_inner<CallableMetatableTimePoint>();

}

IMPLEMENT_OBJECT_INFO(CallableWrapT<TimePoint>,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableWrapT<TimeSpan>,ObjectInfo);

EW_LEAVE
