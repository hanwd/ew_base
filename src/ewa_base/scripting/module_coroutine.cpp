#include "ewa_base/scripting.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE CallableDataRoutineYield : public CallableFunction
{
public:
	CallableDataRoutineYield():CallableFunction("coroutine.yield"){}
	int __fun_call(Executor&,int);
	DECLARE_OBJECT_CACHED_INFO(CallableDataRoutineYield, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableDataRoutineYield, ObjectInfo);

class DLLIMPEXP_EWA_BASE CallableDataRoutineResume : public CallableFunction
{
public:
	CallableDataRoutineResume():CallableFunction("coroutine.resume"){}
	int __fun_call(Executor&,int);
	DECLARE_OBJECT_CACHED_INFO(CallableDataRoutineResume, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableDataRoutineResume, ObjectInfo);


class DLLIMPEXP_EWA_BASE CallableDataRoutineSpawn : public CallableFunction
{
public:
	CallableDataRoutineSpawn(const String& s):CallableFunction(s){}
	int __fun_call(Executor&,int);
};


class DLLIMPEXP_EWA_BASE CallableDataRoutineCreate : public CallableDataRoutineSpawn
{
public:
	CallableDataRoutineCreate():CallableDataRoutineSpawn("coroutine.create"){}

	DECLARE_OBJECT_CACHED_INFO(CallableDataRoutineCreate, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableDataRoutineCreate, ObjectInfo);

class DLLIMPEXP_EWA_BASE CallableDataRoutineWrap : public CallableDataRoutineSpawn
{
public:
	CallableDataRoutineWrap():CallableDataRoutineSpawn("coroutine.wrap"){}
	DECLARE_OBJECT_CACHED_INFO(CallableDataRoutineWrap, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableDataRoutineWrap, ObjectInfo);

class DLLIMPEXP_EWA_BASE CallableDataRoutineStatus : public CallableFunction
{
public:
	CallableDataRoutineStatus():CallableFunction("coroutine.status"){}
	int __fun_call(Executor&,int);
	DECLARE_OBJECT_CACHED_INFO(CallableDataRoutineStatus, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableDataRoutineStatus, ObjectInfo);

CallableCoroutine::~CallableCoroutine()
{
	aStack.clear();
}

void CallableCoroutine::reset(size_t stack_size)
{
	if (stack_size == 0)
	{
		stack_size = 1024 * 256;
	}
	else if (stack_size<1024*8)
	{
		stack_size = 1024*8;
	}

	aStack.resize(stack_size);

	bResumeOrYield=false;

	ci1.nbp=aStack.data();

	ci1.nsp=ci1.nbp+StackState1::SBASE_KEEP;
	ci1.end=ci1.nbp+aStack.size()-128;

	ci0.kep=-1;
	ci0.sp1=ci1.nsp;
	ci0.sp2=ci1.nsp;
	ci0.nbx=ci1.nsp;

	nState=STATE_SUSPEND;
}

CallableCoroutine::CallableCoroutine(size_t stack_size)
{
	reset(stack_size);
	ci1.nbp[StackState1::SBASE_CWD].reset(System::MakePath(System::GetCwd(),""));
}

void CallableCoroutine::UnInit()
{
	nState=STATE_DEAD;
}

int Executor_swap_context(Executor& ewsl,int pm)
{
	if(ewsl.co_this->bResumeOrYield)
	{
		++ewsl.ci0.nbx;
		--pm;
	}

	Variant* sp1=ewsl.ci0.nbx;
	ewsl.ci1.nsp=sp1;


	ewsl.co_this->ci1=ewsl.ci1;
	ewsl.co_this->ci0=ewsl.ci0;
	ewsl.co_this->nState=CallableCoroutine::STATE_SUSPEND;
	ewsl.co_this.swap(ewsl.co_last);
	ewsl.co_this->nState=CallableCoroutine::STATE_RUNNING;
	ewsl.ci0=ewsl.co_this->ci0;
	ewsl.ci1=ewsl.co_this->ci1;

	if(ewsl.co_this->bResumeOrYield)
	{
		--ewsl.ci0.nbx;
		ewsl.ci1.nsp[0].reset(true);
		for(int i=1;i<=pm;i++)
		{
			ewsl.push(sp1[i]);
		}
		return pm+1;
	}
	else
	{
		for(int i=1;i<=pm;i++)
		{
			ewsl.push(sp1[i]);
		}
		return pm;
	}
}

int Executor_swap_error(Executor& ewsl)
{
	if(ewsl.co_this->bResumeOrYield)
	{
		ewsl.ci0.nbx[1].reset(false);
		ewsl.ci0.nbx[2].reset("cannot yield to dead coroutine");
		return 2;
	}
	else
	{
		Exception::XError("cannot yield to dead coroutine");
		return 0;
	}
}



int CallableDataRoutineYield::__fun_call(Executor& ewsl,int pm)
{
	ewsl.co_this->bResumeOrYield=false;

	if(!ewsl.co_last)
	{
		return Executor_swap_error(ewsl);
	}

	return Executor_swap_context(ewsl,pm);

}


int CallableCoroutine::__fun_call(Executor& ewsl,int pm)
{
	ewsl.co_this->bResumeOrYield=false;

	if(nState!=STATE_SUSPEND)
	{
		return Executor_swap_error(ewsl);
	}

	ewsl.co_last.reset(this);
	return Executor_swap_context(ewsl,pm);

}

int CallableDataRoutineResume::__fun_call(Executor& ewsl,int pm)
{
	ewsl.co_this->bResumeOrYield=true;

	if(pm==0)
	{
		ewsl.kerror("error");
	}

	DataPtrT<CallableCoroutine> _pCoroutine=dynamic_cast<CallableCoroutine*>(ewsl.ci0.nbx[1].kptr());
	if(_pCoroutine)
	{
		if(_pCoroutine->nState!=CallableCoroutine::STATE_SUSPEND)
		{
			return Executor_swap_error(ewsl);
		}
		ewsl.co_last=_pCoroutine;
	}
	else
	{
		return Executor_swap_error(ewsl);
	}

	return Executor_swap_context(ewsl,pm);

}


int CallableDataRoutineStatus::__fun_call(Executor& ewsl,int pm)
{

	ewsl.check_pmc(this,pm,1);

	CallableCoroutine* p1=dynamic_cast<CallableCoroutine*>(ewsl.ci1.nsp[0].kptr());

	int n=p1?p1->nState:-1;
	ewsl.ci1.nsp[0].reset(n);

	return 1;
}

class CallableDataRoutineLeave : public CallableObject
{
public:
	CallableDataRoutineLeave(){}

	int __fun_call(Executor& ewsl,int pm)
	{

		ewsl.co_this->bResumeOrYield=false;

		if(!ewsl.co_last)
		{
			if(ewsl.co_main.get()==ewsl.co_this.get())
			{
				Exception::XError("Invalid coroutine status");
			}
			ewsl.co_last=ewsl.co_main;
		}

		//skip param adjust
		ewsl.popq();	

		ewsl.ci0.nbx=ewsl.ci0.sp1;

		pm=Executor_swap_context(ewsl,ewsl.ci1.nsp-ewsl.ci0.nbx);
	
		ewsl.co_last->UnInit();
		ewsl.co_last.reset(NULL);

		return pm;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableDataRoutineLeave, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableDataRoutineLeave, ObjectInfo);

class CallableDataRoutineEnter : public CallableObject
{
public:

	CallableDataRoutineEnter(){}

	int __fun_call(Executor& ewsl,int)
	{
		ewsl.ci0.nbx=ewsl.ci0.sp1;;
		ewsl.ci0.kep=ewsl.ci1.nsp-ewsl.ci0.sp1;
		return ewsl.ci0.kep;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableDataRoutineEnter, ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableDataRoutineEnter, ObjectInfo);

void CallableCoroutine::Init(Variant& fn)
{
	aLLVar[1]=fn;
	aLLVar[2].kptr(CallableDataRoutineEnter::sm_info.GetCachedInstance());
	aLLVar[3].kptr(CallableDataRoutineLeave::sm_info.GetCachedInstance());
	
	// this op will be skipped
	aInsts.push_back(XopInst(XOP_NOOP));

	// call CallableDataRoutineEnter to adjust param
	aInsts.push_back(XopInst(XOP_GET_LLVAR,2));
	aInsts.push_back(XopInst(XOP_CALL,0,0));

	// call coroutine function
	aInsts.push_back(XopInst(XOP_GET_LLVAR,1));
	aInsts.push_back(XopInst(XOP_CALL,-1,-1));

	// call CallableDataRoutineLeave to adjust return value
	aInsts.push_back(XopInst(XOP_GET_LLVAR,3));
	aInsts.push_back(XopInst(XOP_CALL,0,0));

	aInsts.push_back(XopInst(XOP_FAIL,0));	// never reach here

	ci0.nbx=ci1.nsp;
	ci0.nip=aInsts.data();
	ci0.dat=&aLLVar[0];

}

int CallableDataRoutineSpawn::__fun_call(Executor& ewsl,int pm)
{
	ewsl.check_pmc(this,pm,1);

	DataPtrT<CallableCoroutine> co=new CallableCoroutine;

	co->Init(ewsl.ci1.nsp[0]);
	ewsl.ci1.nsp[0].kptr(co.get());
	
	return 1;

}


void CallableCoroutine::Serialize(Serializer& ar)
{
	int v = ar.local_version(0);
	if (v == 0)
	{
		ar & aLLVar[1];
		if(ar.is_reader())
		{
			Init(aLLVar[1]);
		}
		//this_logger().LogWarning("Coroutine cannot be Serialized!");
	}
	else
	{
		ar.errver();
	}
}

class CallableMetatableCoroutine : public CallableMetatable
{
public:
	CallableMetatableCoroutine() :CallableMetatable("coroutine")
	{

	}

	DECLARE_OBJECT_CACHED_INFO(CallableMetatableCoroutine, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableMetatableCoroutine, ObjectInfo);

CallableMetatable* CallableCoroutine::GetMetaTable()
{
	return NULL;// CallableMetatableCoroutine::sm_info.GetCachedInstance();
}

IMPLEMENT_OBJECT_INFO(CallableCoroutine, ObjectInfo);

void init_module_coroutine()
{
	CG_GGVar& gi(CG_GGVar::current());

	gi.add_inner<CallableMetatableCoroutine>();

	gi.add_inner<CallableDataRoutineCreate>();
	gi.add_inner<CallableDataRoutineWrap>();
	gi.add_inner<CallableDataRoutineYield>();
	gi.add_inner<CallableDataRoutineResume>();
	gi.add_inner<CallableDataRoutineStatus>();

}

EW_LEAVE

