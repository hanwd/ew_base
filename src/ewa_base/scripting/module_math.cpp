#include "ewa_base/scripting.h"
#include "ewa_base/math/math_op.h"

EW_ENTER


template<typename P>
class PL1_func : public CallableFunction
{
public:

	PL1_func():CallableFunction(P::info().name,P::info().flag){}

	virtual int __fun_call(Executor& ewsl,int n)
	{
		ewsl.check_pmc(this,n,1);

		Variant &v(*ewsl.ci1.nsp);
		pl1_call<P>::g(v,v);
		return 1;
	}
};

template<typename P>
class PL2_func : public CallableFunction
{
public:

	PL2_func():CallableFunction(P::info().name,P::info().flag){}

	virtual int __fun_call(Executor& ewsl,int n)
	{
		ewsl.check_pmc(this,n,2);

		Variant &v2(ewsl.ci1.nsp[0]);
		Variant &v1(ewsl.ci1.nsp[-1]);
		pl2_call<P>::g(v1,v1,v2);
		return 1;
	}
};

template<typename P>
class PLX_func : public tl::meta_if<tl::is_convertible<P,pl_base1>::value,PL1_func<P>,PL2_func<P> >::type
{
public:

	DECLARE_OBJECT_CACHED_INFO(PLX_func, ObjectInfo);
};

template<typename P> typename PLX_func<P>::infotype  PLX_func<P>::sm_info("EWSL_PLX_func#" + P::info().name);


void int_div_hook_int_int(Variant &r, Variant& v1, Variant& v2)
{
	int64_t n1=variant_handler<int64_t>::raw(v1);
	int64_t n2=variant_handler<int64_t>::raw(v2);

	if(n2==0)
	{
		Exception::XError("math.div zero detected");
	}
	if(n1%n2==0)
	{
		r.reset<int64_t>(n1/n2);
	}
	else
	{
		r.reset<double>(double(n1)/double(n2));
	}
}

void init_module_math()
{
	CG_GGVar& gi(CG_GGVar::current());

	gi.add_inner<PLX_func<pl_add> >();
	gi.add_inner<PLX_func<pl_sub> >();
	gi.add_inner<PLX_func<pl_mul> >();
	gi.add_inner<PLX_func<pl_div> >();
	gi.add_inner<PLX_func<pl_mod> >();
	gi.add_inner<PLX_func<pl_pow> >();

	gi.add_inner<PLX_func<pl_sqrt> >();
	gi.add_inner<PLX_func<pl_log> >();
	gi.add_inner<PLX_func<pl_log10> >();
	gi.add_inner<PLX_func<pl_exp> >();
	gi.add_inner<PLX_func<pl_neg> >();
	gi.add_inner<PLX_func<pl_abs> >();

	gi.add_inner<PLX_func<pl_sin> >();
	gi.add_inner<PLX_func<pl_cos> >();
	gi.add_inner<PLX_func<pl_tan> >();
	gi.add_inner<PLX_func<pl_asin> >();
	gi.add_inner<PLX_func<pl_acos> >();
	gi.add_inner<PLX_func<pl_atan> >();

#if EW_HAS_TRI_H
	gi.add_inner<PLX_func<pl_asinh> >();
	gi.add_inner<PLX_func<pl_acosh> >();
	gi.add_inner<PLX_func<pl_atanh> >();
#endif

	//gi.add_inner<PLX_func<pl_cpx> >();
	gi.add_inner<PLX_func<pl_real> >();
	gi.add_inner<PLX_func<pl_imag> >();
	gi.add_inner<PLX_func<pl_conj> >();

	gi.add_inner<PLX_func<pl_gt> >();
	gi.add_inner<PLX_func<pl_le> >();
	gi.add_inner<PLX_func<pl_lt> >();
	gi.add_inner<PLX_func<pl_ge> >();
	gi.add_inner<PLX_func<pl_ne> >();
	gi.add_inner<PLX_func<pl_eq> >();

	gi.add_inner<PLX_func<pl_bw_and> >();
	gi.add_inner<PLX_func<pl_bw_or> >();
	gi.add_inner<PLX_func<pl_bw_xor> >();
	gi.add_inner<PLX_func<pl_bw_not> >();

	// replace div(int,int) implement
	pl2_call<pl_div>::lk::cmap[type_flag<int64_t>::value<<4|type_flag<int64_t>::value]=int_div_hook_int_int;

}


EW_LEAVE
