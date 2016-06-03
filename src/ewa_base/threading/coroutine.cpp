#include "ewa_base/threading/coroutine.h"
#include "ewa_base/basic/platform.h"

#include "thread_impl.h"


EW_ENTER

class DLLIMPEXP_EWA_BASE CoroutineContext;

void (*raw_coroutine_swap)(CoroutineContext*,CoroutineContext*)=NULL;

#ifdef EW_X86

const unsigned char asm_swap_context_code[]=
{
// mov parameters from stack to ncx,ndx
0x8B,0x4C,0x24,0x04,
0x8B,0x54,0x24,0x08,

// push nbp,nbx,nsi,ndi
0x55,
0x53,
0x56,
0x57,

#ifdef EW_WINDOWS

// push fs:[0],fs:[4],fs:[8];
0x64,0xA1,0x00,0x00,0x00,0x00,
0x50,
0x64,0xA1,0x04,0x00,0x00,0x00,
0x50,
0x64,0xA1,0x08,0x00,0x00,0x00,
0x50,
#endif

// swap nsp
0x89,0x22,
0x8B,0x21,
#ifdef EW_WINDOWS

// pop fs:[8],fs:[4],fs:[0];
0x58,
0x64,0xA3,0x08,0x00,0x00,0x00,
0x58,
0x64,0xA3,0x04,0x00,0x00,0x00,
0x58,
0x64,0xA3,0x00,0x00,0x00,0x00,
#endif

// pop ndi,nsi,nbx,nbp
0x5F,
0x5E,
0x5B,
0x5D,

// ret
0xC3
};

#else

const unsigned char asm_swap_context_code[]=
{
// push nbp,nbx,nsi,ndi
0x55,
0x53,
0x56,
0x57,

// push r12,r13,r14,r15
0x41,0x54,
0x41,0x55,
0x41,0x56,
0x41,0x57,

#ifdef EW_WINDOWS
// push gs:[0],gs[8],gs:[16]
0x65, 0x48, 0x8B, 0x04, 0x25, 0x00, 0x00, 0x00, 0x00,
0x50,
0x65, 0x48, 0x8B, 0x04, 0x25, 0x08, 0x00, 0x00, 0x00,
0x50,
0x65, 0x48, 0x8B, 0x04, 0x25, 0x10, 0x00, 0x00, 0x00,
0x50,
#endif


#ifdef EW_WINDOWS
// swap nsp
0x48, 0x89, 0x22,
0x48, 0x8B, 0x21,

#else
// swap nsp
0x48,0x89,0x26,
0x48,0x8B,0x27,

#endif

#ifdef EW_WINDOWS
// pop gs:[16],gs:[8],gs:[0]
0x58,
0x65, 0x48, 0x89, 0x04, 0x25, 0x10, 0x00, 0x00, 0x00,
0x58,
0x65, 0x48, 0x89, 0x04, 0x25, 0x08, 0x00, 0x00, 0x00,
0x58,
0x65, 0x48, 0x89, 0x04, 0x25, 0x00, 0x00, 0x00, 0x00,
#endif

// pop r15,r14,r13,r12
0x41,0x5F,
0x41,0x5E,
0x41,0x5D,
0x41,0x5C,

// pop ndi,nsi,nbx,nbp
0x5F,
0x5E,
0x5B,
0x5D,

// ret
0xC3

};
#endif

class DLLIMPEXP_EWA_BASE CoroutineContext : public mp_obj
{
public:

	static void raw_coroutine_spawn(CoroutineContext* cx0);

	CoroutineContext(size_t n=1024*1024)
	{
		m_pRoutine=NULL;
		base=NULL;nsp=NULL;size=0;
		size_t pagesize=System::GetPageSize();
		size=sz_helper::adj(n,pagesize)+pagesize;
		m_nState.store(Coroutine::STATE_STOPPED);
	}

	~CoroutineContext()
	{
		if(base)
		{
			page_free(base,size);
		}
	}


	void** nsp;
	void** nbp;
	void *base;
	size_t size;

	Coroutine* m_pRoutine;
	DataPtrT<ObjectData> m_pExtraParam;

	AtomicInt32 m_nState;

	// initialize coroutine stack
	bool init()
	{
		if(nsp)
		{
			return true;
		}

		if(!raw_coroutine_swap)
		{
			void*p=(void*)asm_swap_context_code;
			raw_coroutine_swap=(void (*)(CoroutineContext*,CoroutineContext*))p;
			page_protect(p,4096,FLAG_FILE_RD|FLAG_FILE_EXEC);
		}

		if(size==0)
		{
			return false;
		}

		base=page_alloc(size);
		if(!base)
		{
			return false;
		}

		// protect first page to detect stack overflow
		page_protect(base,4096,0);

		nbp=(void**)((char*)base+size);
		nsp=nbp-16;

		push(NULL);
		push(this);		// arg0 from X86
		push(NULL);		// frame pointer
		push((void*)raw_coroutine_spawn);	// ret address
		push(NULL);		// nbp
		push(NULL);		// nbx
		push(this);		// nsi, arg for X64 linux
		push(this);		// ndi, arg for X64 linux

#ifdef EW_X64
		push(NULL);		// r12
		push(NULL);		// r13
		push(NULL);		// r14
		push(NULL);		// r15
#endif

#ifdef EW_WINDOWS
		push((void*)(sizeof(void*)==4?-1:0));	// fs:[0] exception handler
		push(nbp);								// fs:[4] stack hi
		push(base);								// fs:[8] stack lo
#endif

		return true;
	}

	void push(void* d){*--nsp=d;}

};



CoroutineMain::CoroutineMain():Coroutine(0)
{
	m_pThisRoutine=this;
	m_pLastRoutine=NULL;
	m_pVoidRoutine=NULL;
}

void CoroutineMain::init()
{
	if(m_pContext) return;

	m_pContext=new CoroutineContext(0);

	m_pContext->m_nState.store(Coroutine::STATE_RUNNING);
	m_pContext->m_pRoutine=this;
}

Coroutine& Coroutine::this_coroutine()
{
	return *ThreadImpl::this_data().cort_main().m_pThisRoutine;
}

CoroutineMain& Coroutine::main_coroutine()
{
	return ThreadImpl::this_data().cort_main();
}

Coroutine::~Coroutine()
{
	delete m_pContext;
}

int Coroutine::state() const
{
	return m_pContext->m_nState.get();
}

size_t Coroutine::stack_size() const
{
	return m_pContext->size;
}

bool Coroutine::spawn(Coroutine* pcortctx_)
{
	if(!pcortctx_) return false;

	CoroutineContext& ctx(*pcortctx_->m_pContext);

	if(!ctx.init())
	{
		return false;
	}

	int32_t val=STATE_STOPPED;
	if(!ctx.m_nState.compare_exchange(val,STATE_PAUSED))
	{
		return false;
	}

	return true;
}


Coroutine::Coroutine(size_t stksize_)
{
	if(stksize_>0)
	{
		m_pContext=new CoroutineContext(stksize_);
		m_pContext->m_pRoutine=this;
	}
	else
	{
		m_pContext=NULL;
	}
}

bool Coroutine::yield_main(ObjectData* extra_)
{
	return yield(&main_coroutine(),extra_);
}

bool Coroutine::yield_last(ObjectData* extra_)
{
	return yield(main_coroutine().m_pLastRoutine,extra_);
}


ObjectData* Coroutine::extra()
{
	return m_pContext->m_pExtraParam.get();
}

void Coroutine::_yield_phase2(CoroutineContext& caller)
{
	CoroutineMain& comain(main_coroutine());
	comain.m_pThisRoutine=caller.m_pRoutine;

	if(comain.m_pLastRoutine)
	{
		EW_ASSERT(!comain.m_pVoidRoutine);
		comain.m_pLastRoutine->m_pContext->m_nState.store(Coroutine::STATE_PAUSED);
	}
	else if(comain.m_pVoidRoutine)
	{
		comain.m_pVoidRoutine->m_pContext->m_nState.store(Coroutine::STATE_STOPPED);
		comain.m_pVoidRoutine=NULL;
	}
	else
	{
		System::LogError("which coroutine yiled to me???");
	}

}

bool Coroutine::_yield_phase1(CoroutineContext& callee,CoroutineContext& caller)
{

	EW_ASSERT(caller.m_nState.get()==Coroutine::STATE_RUNNING);

	CoroutineMain& comain(main_coroutine());

	if(&caller==&callee) return false;

	int32_t val=Coroutine::STATE_PAUSED;
	if(!callee.m_nState.compare_exchange(val,Coroutine::STATE_RUNNING))
	{
		if(val==Coroutine::STATE_RUNNING)
		{
			System::LogTrace("try yielding to running coroutine");
		}
		else
		{
			System::LogTrace("try yielding to stopped coroutine");
		}
		return false;
	}

	comain.m_pLastRoutine=caller.m_pRoutine;

	return true;

}

bool Coroutine::yield(Coroutine* pcortctx_,ObjectData* extra_)
{
	if(!pcortctx_)
	{
		System::LogTrace("try yielding to NULL coroutine");
		return false;
	}

	CoroutineContext& caller(*this_coroutine().m_pContext);
	CoroutineContext& callee(*pcortctx_->m_pContext);

	if(!_yield_phase1(callee,caller))
	{
		return false;
	}


	callee.m_pExtraParam.reset(extra_);

	raw_coroutine_swap(&callee,&caller);

	_yield_phase2(caller);

	return true;
}

void CoroutineContext::raw_coroutine_spawn(CoroutineContext* cx0)
{
	CoroutineContext& caller(*cx0);

	for(;;)
	{

		Coroutine::_yield_phase2(caller);

		try
		{
			caller.m_pRoutine->svc();
		}
		catch(std::exception& e)
		{
			System::LogError("Unhandled exception in Coroutine::svc : %s",e.what());
		}
		
		CoroutineMain& comain(Coroutine::main_coroutine());
		CoroutineContext& callee(*comain.m_pContext);

		if(!Coroutine::_yield_phase1(callee,caller))
		{
			System::LogError("yield main_coroutine failed, why???");
			while(!Coroutine::_yield_phase1(callee,caller))
			{
				Thread::yield();
			}
		}

		comain.m_pLastRoutine=NULL;
		comain.m_pVoidRoutine=caller.m_pRoutine;
		comain.m_pContext->m_pExtraParam.reset(NULL);

		raw_coroutine_swap(&callee,&caller);
		
	}
}


EW_LEAVE




