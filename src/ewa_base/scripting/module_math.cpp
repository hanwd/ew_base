#include "ewa_base/scripting.h"
#include "ewa_base/math/math_op.h"

EW_ENTER


int down_cast_variant_type(Variant& v)
{
	typedef arr_xt<Variant> arr_type;
	int n=v.type();
	if(n!=type_flag<arr_type>::value)
	{
		return n;
	}

	arr_type& val(variant_handler<arr_type>::raw(v));
	int type=0;

	for(size_t i=0;i<val.size();i++)
	{
		int t=val[i].type();
		if(type<t) type=t;
	}

	switch(type)
	{
	case type_flag<int64_t>::value :
	{
		arr_xt<int64_t> arr=variant_cast<arr_xt<int64_t> >(val);
		v.ref<arr_xt<int64_t> >().swap(arr);
		break;
	}
	case type_flag<double>::value :
	{
		arr_xt<double> arr=variant_cast<arr_xt<double> >(val);
		v.ref<arr_xt<double> >().swap(arr);
		break;
	}
	case type_flag<dcomplex>::value :
	{
		arr_xt<dcomplex> arr=variant_cast<arr_xt<dcomplex> >(val);
		v.ref<arr_xt<dcomplex> >().swap(arr);
		break;
	}
	};


	return v.type();
}

template<typename P>
class PL1_func : public CallableFunction
{
public:
	typedef typename pl1_call<P>::lk lk;

	static int variant_call(Executor &ewsl,Variant& v1)
	{
		int n=down_cast_variant_type(v1);
		if(lk::cmap[n]==&PL1_func<P>::variant_call)
		{
			return pl1_dispatch_base<P>::value(ewsl,v1);
		}
		return lk::test(n)(ewsl,v1);	
	}

	PL1_func():CallableFunction(P::info().name,P::info().flag)
	{
		int v1=type_flag<arr_xt<Variant> >::value;
		lk::cmap[v1]=variant_call;
	}

	virtual int __fun_call(Executor& ewsl,int n)
	{
		ewsl.check_pmc(this,n,1);
		Variant &v(*ewsl.ci1.nsp);
		return pl1_call<P>::g(ewsl);
	}
};




template<typename P>
class PL2_func : public CallableFunction
{
public:

	typedef typename pl2_call<P>::lk lk;

	static int variant_call(Executor& ewsl,Variant& v1,Variant& v2)
	{
		int n=(down_cast_variant_type(v1)<<4)|down_cast_variant_type(v2);
		if(lk::cmap[n]==&PL2_func<P>::variant_call)
		{
			return pl2_dispatch_base<P>::value(ewsl,v1,v2);
		}
		return lk::test(n)(ewsl,v1,v2);	
	}

	PL2_func():CallableFunction(P::info().name,P::info().flag)
	{
		int value=type_flag<arr_xt<Variant> >::value;
		for(int i=0;i<16;i++)
		{
			int v1=(value<<4)|i;
			int v2=(i<<4)|value;
			lk::cmap[v1]=variant_call;
			lk::cmap[v2]=variant_call;
		}
	}

	virtual int __fun_call(Executor& ewsl,int n)
	{
		ewsl.check_pmc(this,n,2);
		return pl2_call<P>::g(ewsl);
	}
};

template<typename P>
class PLX_func : public tl::meta_if<tl::is_convertible<P,pl_base1>::value,PL1_func<P>,PL2_func<P> >::type
{
public:

	DECLARE_OBJECT_CACHED_INFO(PLX_func, ObjectInfo);
};

template<typename P> typename PLX_func<P>::infotype  PLX_func<P>::sm_info("EWSL_PLX_func#" + P::info().name);


int int_div_hook_int_int(Executor &ewsl, Variant& v1, Variant& v2)
{
	int64_t n1=variant_handler<int64_t>::raw(v1);
	int64_t n2=variant_handler<int64_t>::raw(v2);

	if(n2==0)
	{
		Exception::XError("math.div zero detected");
	}
	if(n1%n2==0)
	{
		(*--ewsl.ci1.nsp).reset(n1/n2);
	}
	else
	{
		(*--ewsl.ci1.nsp).reset(double(n1)/double(n2));
	}

	return 1;
}

template<bool F>
static int value_direct(Executor& ewsl, Variant& v1,Variant&)
{
	(*--ewsl.ci1.nsp).reset(F);
	return 1;
}

void init_module_math()
{
	CG_GGVar& gi(CG_GGVar::current());

	gi.add_inner<PLX_func<pl_add> >();
	gi.add_inner<PLX_func<pl_sub> >();
	gi.add_inner<PLX_func<pl_mod> >();

	gi.add_inner<PLX_func<pl_arr_mul> >();
	gi.add_inner<PLX_func<pl_arr_div> >();
	gi.add_inner<PLX_func<pl_arr_pow> >();
	gi.add_inner<PLX_func<pl_mat_mul> >();
	gi.add_inner<PLX_func<pl_mat_div> >();
	gi.add_inner<PLX_func<pl_mat_pow> >();


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
	pl2_call<pl_arr_div>::lk::cmap[type_flag<int64_t>::value<<4|type_flag<int64_t>::value]=int_div_hook_int_int;
	pl2_call<pl_mat_div>::lk::cmap[type_flag<int64_t>::value<<4|type_flag<int64_t>::value]=int_div_hook_int_int;


	for(int i=0;i<16;i++) for(int j=0;j<16;j++)
	{
		unsigned n=(i<<4)|j;
		if(i!=0&&j!=0) continue;

		if(pl2_call<pl_eq>::lk::cmap[n]==&pl2_dispatch_base<pl_eq>::value)
		{
			pl2_call<pl_eq>::lk::cmap[n]=&value_direct<false>;
		}
		if(pl2_call<pl_ne>::lk::cmap[n]==&pl2_dispatch_base<pl_ne>::value)
		{
			pl2_call<pl_ne>::lk::cmap[n]=&value_direct<true>;
		}
	}

}


EW_LEAVE
