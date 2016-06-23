#include "ewa_base/scripting.h"
#include "ewa_base/threading/thread.h"

#include <cstdio>

EW_ENTER


class CallableFuncTimeAdd : public CallableFunction
{
public:

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this,pm,2);
		Variant& v1(ewsl.ci0.nbx[1]);
		Variant& v2(ewsl.ci0.nbx[2]);

		if (CallableWrapT<TimePoint>* p1 = dynamic_cast<CallableWrapT<TimePoint>*>(v1.kptr()))
		{
			if (CallableWrapT<TimeSpan>* p2 = dynamic_cast<CallableWrapT<TimeSpan>*>(v2.kptr()))
			{
				v1.kptr(new CallableWrapT<TimePoint>(p1->value + p2->value));
				return 1;
			}
		}
		else if (CallableWrapT<TimeSpan>* p1 = dynamic_cast<CallableWrapT<TimeSpan>*>(v1.kptr()))
		{
			if (CallableWrapT<TimePoint>* p2 = dynamic_cast<CallableWrapT<TimePoint>*>(v2.kptr()))
			{
				v1.kptr(new CallableWrapT<TimePoint>(p1->value + p2->value));
				return 1;
			}
		}
		return 0;
	}
};



class CallableFuncTimeSub : public CallableFunction
{
public:

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this,pm,2);
		Variant& v1(ewsl.ci0.nbx[1]);
		Variant& v2(ewsl.ci0.nbx[2]);
		if (CallableWrapT<TimePoint>* p1 = dynamic_cast<CallableWrapT<TimePoint>*>(v1.kptr()))
		{
			if (CallableWrapT<TimePoint>* p2 = dynamic_cast<CallableWrapT<TimePoint>*>(v2.kptr()))
			{
				v1.kptr(new CallableWrapT<TimeSpan>(p1->value - p2->value));
				return 1;
			}
			else if (CallableWrapT<TimeSpan>* p2 = dynamic_cast<CallableWrapT<TimeSpan>*>(v2.kptr()))
			{
				v1.kptr(new CallableWrapT<TimePoint>(p1->value - p2->value));
				return 1;
			}
		}
		else if (CallableWrapT<TimeSpan>* p1 = dynamic_cast<CallableWrapT<TimeSpan>*>(v1.kptr()))
		{
			if (CallableWrapT<TimeSpan>* p2 = dynamic_cast<CallableWrapT<TimeSpan>*>(v2.kptr()))
			{
				v1.kptr(new CallableWrapT<TimeSpan>(p1->value - p2->value));
				return 1;
			}
		}
		return 0;
	}
};


class CallableFuncTimeMul : public CallableFunction
{
public:

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this,pm,2);
		Variant& v1(ewsl.ci0.nbx[1]);
		Variant& v2(ewsl.ci0.nbx[2]);
		if (CallableWrapT<TimeSpan>* p1 = dynamic_cast<CallableWrapT<TimeSpan>*>(v1.kptr()))
		{
			try
			{
				v1.kptr(new CallableWrapT<TimeSpan>(p1->value*variant_cast<double>(v2)));
				return 1;
			}
			catch (...)
			{

			}
		}
		else if (CallableWrapT<TimeSpan>* p1 = dynamic_cast<CallableWrapT<TimeSpan>*>(v2.kptr()))
		{
			try
			{
				v1.kptr(new CallableWrapT<TimeSpan>(p1->value*variant_cast<double>(v1)));
				return 1;
			}
			catch (...)
			{

			}
		}
		return 0;
	}
};


class CallableFuncTimeDiv : public CallableFunction
{
public:

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this,pm,2);
		Variant& v1(ewsl.ci0.nbx[1]);
		Variant& v2(ewsl.ci0.nbx[2]);

		if (CallableWrapT<TimeSpan>* p1 = dynamic_cast<CallableWrapT<TimeSpan>*>(v1.kptr()))
		{
			if (CallableWrapT<TimeSpan>* p2 = dynamic_cast<CallableWrapT<TimeSpan>*>(v2.kptr()))
			{
				v1.reset(p1->value / p2->value);
				return 1;
			}

			try
			{
				v1.kptr(new CallableWrapT<TimeSpan>(p1->value/variant_cast<double>(v2)));
				return 1;
			}
			catch (...)
			{

			}
		}
		return 0;
	}
};


template<typename P>
class CallableFuncCompare : public CallableFunction
{
public:

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this,pm,2);
		Variant& v1(ewsl.ci0.nbx[1]);
		Variant& v2(ewsl.ci0.nbx[2]);

		if (CallableWrapT<TimeSpan>* p1 = dynamic_cast<CallableWrapT<TimeSpan>*>(v1.kptr()))
		{
			if (CallableWrapT<TimeSpan>* p2 = dynamic_cast<CallableWrapT<TimeSpan>*>(v2.kptr()))
			{
				v1.reset(P::g(p1->value.val,p2->value.val));
				return 1;
			}

			try
			{
				v1.reset(P::g(p1->value / TimeSpan::MilliSeconds(1),variant_cast<double>(v2)));
				return 1;
			}
			catch (...)
			{
				return 0;
			}
		}
		if (CallableWrapT<TimeSpan>* p1 = dynamic_cast<CallableWrapT<TimeSpan>*>(v2.kptr()))
		{
			try
			{
				v1.reset(P::g(variant_cast<double>(v1), p1->value / TimeSpan::MilliSeconds(1)));
				return 1;
			}
			catch (...)
			{
				return 0;
			}
		}
		if (CallableWrapT<TimePoint>* p1 = dynamic_cast<CallableWrapT<TimePoint>*>(v1.kptr()))
		{
			if (CallableWrapT<TimePoint>* p2 = dynamic_cast<CallableWrapT<TimePoint>*>(v2.kptr()))
			{
				v1.reset(P::g(p1->value.val, p2->value.val));
				return 1;
			}
		}
		return 0;
	}

};

class CallableMetatableTime : public CallableMetatable
{
public:

	CallableMetatableTime(const String& s) :CallableMetatable(s)
	{
		table_meta["__add"].kptr(new CallableFuncTimeAdd);
		table_meta["__sub"].kptr(new CallableFuncTimeSub);
		table_meta["__mul"].kptr(new CallableFuncTimeMul);
		table_meta["__div"].kptr(new CallableFuncTimeDiv);
		table_meta["__gt"].kptr(new CallableFuncCompare<pl_gt>);
		table_meta["__lt"].kptr(new CallableFuncCompare<pl_lt>);
		table_meta["__ge"].kptr(new CallableFuncCompare<pl_ge>);
		table_meta["__le"].kptr(new CallableFuncCompare<pl_le>);
		table_meta["__ne"].kptr(new CallableFuncCompare<pl_ne>);
		table_meta["__eq"].kptr(new CallableFuncCompare<pl_eq>);

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


class CallableFunctionLastError : public CallableFunction
{
public:
	CallableFunctionLastError() :CallableFunction("os.lasterror",0){}
	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this, pm, 0,1);
		String lasterror=System::GetLastError();
		if(pm==1)
		{
			String p = variant_cast<String>(ewsl.ci0.nbx[1]);
			System::SetLastError(p);
		}
		ewsl.ci0.nbx[1].reset(lasterror);
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionLastError, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionLastError, ObjectInfo);




class CallableFunctionGetCwd : public CallableFunction
{
public:

	CallableFunctionGetCwd() :CallableFunction("os.getcwd",1){}

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this, pm, 0);
		ewsl.ci0.nbx[1].reset(ewsl.get_cwd());
		return 1;
	}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionGetCwd, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionGetCwd, ObjectInfo);

class CallableFunctionSetCwd : public CallableFunction
{
public:

	CallableFunctionSetCwd() :CallableFunction("os.setcwd",1){}

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this, pm, 1);
		String& newcwd=ewsl.ci0.nbx[1].ref<String>();
		ewsl.set_cwd(System::MakePath(newcwd,ewsl.get_cwd()));
		return 1;
	}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionSetCwd, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionSetCwd, ObjectInfo);


class CallableFunctionDir : public CallableFunction
{
public:

	CallableFunctionDir() :CallableFunction("os.dir",1){}

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.check_pmc(this, pm, 0,1);
		String dir;
		if(pm==1) dir=ewsl.ci0.nbx[1].ref<String>();

		dir=System::MakePath(dir,ewsl.get_cwd());

		arr_1t<FileItem> files=FSLocal::current().FindFilesEx(dir);

		arr_xt<Variant> result;
		result.resize(files.size());

		for(size_t i=0;i<files.size();i++)
		{
			result[i].reset(files[i].filename);
		}
		ewsl.ci0.nbx[1].reset(result);
		return 1;
	}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionDir, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionDir, ObjectInfo);


class CallableFunctionExit : public CallableFunction
{
public:

	CallableFunctionExit() :CallableFunction("os.exit",1)
	{
		xop[0].op=XOP_EXIT;
		xop[1].op=XOP_FAIL;

		__set_helpdata("exit ewsl excutor");
	}
	XopInst xop[2];
	virtual int __fun_call(Executor& ewsl, int pm)
	{
		ewsl.ci0.nip=xop;
		return CallableData::STACK_BALANCED;
	}

	DECLARE_OBJECT_CACHED_INFO(CallableFunctionExit, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableFunctionExit, ObjectInfo);



static int ERRNO_HandleResult(Executor& ewsl,bool flag)
{
	if(flag)
	{
		ewsl.ci0.nbx[1].reset(true);
		return 1;
	}
	else
	{
		ewsl.ci0.nbx[1].reset(false);
		ewsl.ci0.nbx[2].reset(System::GetLastError());
		return 2;
	}
}


class CallableFunctionRemove : public CallableFunction
{
public:

	CallableFunctionRemove():CallableFunction("os.remove",0){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		String fp=variant_cast<String>(ewsl.ci0.nbx[1]);
		return ERRNO_HandleResult(ewsl,FSLocal::current().Remove(System::MakePath(fp,ewsl.get_cwd())));

	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionRemove, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionRemove, ObjectInfo);


class CallableFunctionRename : public CallableFunction
{
public:

	CallableFunctionRename():CallableFunction("os.rename",0){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,2);
		String fp1=System::MakePath(variant_cast<String>(ewsl.ci0.nbx[1]),ewsl.get_cwd());
		String fp2=System::MakePath(variant_cast<String>(ewsl.ci0.nbx[2]),ewsl.get_cwd());
		return ERRNO_HandleResult(ewsl,FSLocal::current().Rename(fp1,fp2,0));
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionRename, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionRename, ObjectInfo);




class CallableFunctionTmpname : public CallableFunction
{
public:

	CallableFunctionTmpname():CallableFunction("os.tmpname",0){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,0);
		char buf[1024*4];
		String fp=::tmpnam(buf);
		ewsl.ci0.nbx[1].reset(fp);
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionTmpname, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionTmpname, ObjectInfo);



class CallableFunctionFiles : public CallableFunction
{
public:

	CallableFunctionFiles():CallableFunction("os.files",0){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);

		String fp=variant_cast<String>(ewsl.ci0.nbx[1]);
		if(FSLocal::current().FileExists(fp)!=2)
		{
			Exception::XError("invalid folder");
		}

		arr_1t<FileItem> items=FSLocal::current().FindFilesEx(fp);

		arr_xt<Variant> & tb(ewsl.ci0.nbx[1].ref<arr_xt<Variant> >());
		tb.resize(items.size());

		for(size_t i=0;i<items.size();i++)
		{
			VariantTable& item(tb[i].ref<VariantTable>());
			item["name"].reset(items[i].filename);
			item["size"].reset(items[i].filesize);
			item["flag"].reset(items[i].flags.val());
		}

		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionFiles, ObjectInfo);

};

IMPLEMENT_OBJECT_INFO(CallableFunctionFiles, ObjectInfo);



class CallableFunctionMkdir : public CallableFunction
{
public:

	CallableFunctionMkdir():CallableFunction("os.mkdir",0){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		String fp=variant_cast<String>(ewsl.ci0.nbx[1]);
		ewsl.ci0.nbx[1].reset(FSLocal::current().Mkdir(System::MakePath(fp,ewsl.get_cwd())));
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionMkdir, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionMkdir, ObjectInfo);

class CallableFunctionRmdir : public CallableFunction
{
public:

	CallableFunctionRmdir():CallableFunction("os.rmdir",0){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,1);
		String fp=variant_cast<String>(ewsl.ci0.nbx[1]);
		ewsl.ci0.nbx[1].reset(FSLocal::current().Rmdir(System::MakePath(fp,ewsl.get_cwd()),0));
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionRmdir, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionRmdir, ObjectInfo);

void init_module_os()
{

	CG_GGVar& gi(CG_GGVar::current());
	gi.add_inner<CallableFunctionTime>();
	gi.add_inner<CallableFunctionSleep>();
	gi.add_inner<CallableFunctionShell>();
	gi.add_inner<CallableFunctionGetEnv>();
	gi.add_inner<CallableFunctionLastError>();

	gi.add_inner<CallableFunctionGetCwd>();
	gi.add_inner<CallableFunctionSetCwd>();
	gi.add_inner<CallableFunctionDir>();

	gi.add_inner<CallableMetatableTimeSpan>();
	gi.add_inner<CallableMetatableTimePoint>();

	gi.add_inner<CallableFunctionRemove>();
	gi.add_inner<CallableFunctionRename>();
	gi.add_inner<CallableFunctionTmpname>();

	gi.add_inner<CallableFunctionFiles>();
	gi.add_inner<CallableFunctionMkdir>();
	gi.add_inner<CallableFunctionRmdir>();

	gi.add_inner<CallableFunctionExit>();
}

IMPLEMENT_OBJECT_INFO(CallableWrapT<TimePoint>,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableWrapT<TimeSpan>,ObjectInfo);

EW_LEAVE
