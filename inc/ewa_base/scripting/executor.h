
#ifndef __H_EW_SCRIPTING_EXECUTOR__
#define __H_EW_SCRIPTING_EXECUTOR__

#include "ewa_base/scripting/variant_ggvar.h"
#include "ewa_base/scripting/callable_coroutine.h"
#include "ewa_base/scripting/codegen.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE Executor
{
public:

	friend class CallableCode;

	explicit Executor(size_t stack_size=0);

	Executor(const Executor& o);
	~Executor();

	Executor& operator=(const Executor& o);


	EW_FORCEINLINE void push(CallableData* v){(*++ci1.nsp).kptr(v);}
	EW_FORCEINLINE void push(){(*++ci1.nsp).clear();}

#ifdef EW_C11
	template<typename T>
	EW_FORCEINLINE void push(T&& v=T()){ (*++ci1.nsp).reset(std::forward<T>(v)); }
#else
	template<typename T>
	EW_FORCEINLINE void push(const T& v=T()){ (*++ci1.nsp).reset(v); }
#endif

	void popq_ref(Variant& d);
	void popq(Variant& d);

	EW_FORCEINLINE void popq(){ --ci1.nsp; }

	template<typename T>
	void popq(T& v)
	{
		v=variant_cast<T>(*ci1.nsp);
		--ci1.nsp;
	}

	EW_FORCEINLINE Variant& top(){ return *ci1.nsp; }

	template<typename P>
	void call1()
	{
		pl1_call<P>::g(*this);
	}

	template<typename P>
	void call2()
	{
		pl2_call<P>::g(*this);
	}

	bool callx(int n,int k=1);
	int callx_raw(int n,int k);

	void kerror(const String& s);
	void kthrow(int n);

	inline void check_pmc(CallableData*,int pm1,int pm2)
	{
		if(pm1==pm2) return;
		kerror(String::Format("invalid param count, %d param(s) expected",pm2));
	}

	inline void check_pmc(CallableData*,int pm1,int pm_lo,int pm_hi)
	{
		if( (pm_lo<0||pm1>=pm_lo) && (pm_hi<0||pm1<=pm_hi)) return;
		kerror(String::Format("invalid param count, %d-%d params expected",pm_lo,pm_hi));
	}

	inline void check_stk(size_t n)
	{
		if (ci0.nbx + n < ci1.end) return;
		kerror("stack overflow");
	}


	void get_array(int n);
	void set_array(int n);
	void get_global(const String& s){push(tb1[s]);}
	void set_global(const String& s){popq(tb1[s]);}
	void get_system(const String& s){push(tb0[s]);}

	void get_index(const String& s);
	void set_index(const String& s);

	void reset();

	int move_return_values(int n);

	DataPtrT<CallableData> parse(const String& s);

	enum
	{
		INDIRECT_FILE,
		DIRECT_STRING,
	};

	// compile s(string or filename) and push to stack
	bool prepare(const String& s,int t=DIRECT_STRING);

	// complie s(string) and execute
	bool execute(const String& s,int pmc=0,int kep=0);

	// complie s(filename) and execute
	bool execute_file(const String& s,int pmc=0,int kep=0);

	template<typename T>
	T eval(const String &s);

	template<typename T>
	bool eval(const String& s,T& v);

	StackState0 ci0;
	StackState1 ci1;
	StackState2 ci2;

	const CG_GGVar& tb0;
	VariantTable& tb1;

	bool set_cwd(const String& s);
	String get_cwd();

	String make_path(const String& s);

	DataPtrT<CallableCoroutine> co_this;
	DataPtrT<CallableCoroutine> co_main;
	DataPtrT<CallableCoroutine> co_last;

protected:

	VariantTable tb1_internal;

	void _vm_run1(int kep);
	void _vm_run2(int kep);
	void _vm_check_ret(int ret);

	void _vm_handle_exception(std::exception &e);

};

EW_FORCEINLINE void Executor::popq_ref(Variant& d)
{
	d = *ci1.nsp--;
}

EW_FORCEINLINE void Executor::popq(Variant& d)
{
	Variant& s(*ci1.nsp--);
	if(s.flag==-1&&s.data.pval->GetRef()!=1)
	{
		variant_handler<CallableData*>::clone(d,s.data.pval);
	}
	else
	{
		d=s;
	}
}

template<typename T>
T Executor::eval(const String &s)
{
	if(!execute("return ("+s+");",0,1))
	{
		Exception::XError("eval failed");
	}
	return pl_cast<T>::g(*ci1.nsp--);
}

template<typename T>
bool Executor::eval(const String& s,T& v)
{
	if(!execute("return ("+s+");",0,1))
	{
		return false;
	}

	try
	{
		v=pl_cast<T>::g(*ci1.nsp--);
		return true;
	}
	catch(...)
	{
		return false;
	}
}

EW_LEAVE

#endif
