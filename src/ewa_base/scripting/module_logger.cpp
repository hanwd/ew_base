#include "ewa_base/scripting.h"
#include "ewa_base/logging.h"

EW_ENTER


String string_format_variants(Variant* p,int n)
{
	if(n--<=0) return "";
	String res;

	switch(n)
	{
	case 0: 
		res=variant_cast<String>(p[0]);
		break;
	case 1:
		res=String::Format(p[0],p[1]);
		break;
	case 2:
		res=String::Format(p[0],p[1],p[2]);
		break;
	case 3:
		res=String::Format(p[0],p[1],p[2],p[3]);
		break;
	case 4:
		res=String::Format(p[0],p[1],p[2],p[3],p[4]);
		break;
	case 5:
		res=String::Format(p[0],p[1],p[2],p[3],p[4],p[5]);
		break;
	case 6:
		res=String::Format(p[0],p[1],p[2],p[3],p[4],p[5],p[6]);
		break;
	case 7:
		res=String::Format(p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
		break;
	case 8:
		res=String::Format(p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8]);
		break;
	default:
		res=String::Format(p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9]);
		break;
	}
	return res;
}

class CallableFunctionLogMessage : public CallableFunction
{
public:
	CallableFunctionLogMessage():CallableFunction("logger.log_message"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		String v=string_format_variants(ewsl.ci0.nbx+1,pm);
		this_logger().LogMessage(v);
		return 0;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionLogMessage, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionLogMessage, ObjectInfo);

class CallableFunctionLogError : public CallableFunction
{
public:
	CallableFunctionLogError():CallableFunction("logger.log_error"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		String v=string_format_variants(ewsl.ci0.nbx+1,pm);
		this_logger().LogError(v);
		return 0;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionLogError, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionLogError, ObjectInfo);

class CallableFunctionLogWarning : public CallableFunction
{
public:
	CallableFunctionLogWarning():CallableFunction("logger.log_warning"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		String v=string_format_variants(ewsl.ci0.nbx+1,pm);
		this_logger().LogWarning(v);
		return 0;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionLogWarning, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionLogWarning, ObjectInfo);

class CallableFunctionLogTarget : public CallableFunction
{
public:
	CallableFunctionLogTarget():CallableFunction("logger.set_target"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if(pm==0)
		{
			ewsl.push(false);
			return 1;
		}

		String v=variant_cast<String>(ewsl.ci0.nbx[1]);
		bool flag=true;

		if(v=="null")
		{
			this_logger().reset(new LogNull);
		}
		else if(v=="console")
		{
			this_logger().reset(new LogConsole);
		}
		else if(v=="file" && pm>1)
		{
			bool app=true;
			if(pm>2)
			{
				app=variant_cast<bool>(ewsl.ci0.nbx[3]);
			}

			String file=variant_cast<String>(ewsl.ci0.nbx[2]);
			LogFile* p=new LogFile(file,app);
			this_logger().reset(p);

			flag=p->good();

		}
		else
		{
			flag=false;
		}

		ewsl.ci0.nbx[1].reset(flag);
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionLogTarget, ObjectInfo);

};
IMPLEMENT_OBJECT_INFO(CallableFunctionLogTarget, ObjectInfo);

void init_module_logger()
{	
	CG_GGVar& gi(CG_GGVar::current());
	gi.add_inner<CallableFunctionLogTarget>();
	gi.add_inner<CallableFunctionLogMessage>();
	gi.add_inner<CallableFunctionLogError>();
	gi.add_inner<CallableFunctionLogWarning>();
}

EW_LEAVE
