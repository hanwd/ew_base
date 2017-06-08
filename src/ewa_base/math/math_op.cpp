#include "ewa_base/math/math_op.h"
#include "ewa_base/math/math_def.h"
#include "ewa_base/util/strlib.h"

EW_ENTER

op_info::op_info(const String& s,int f):name(s),op_name("__"+string_split(s,".").back()),flag(f){}
op_info::op_info(const String& s,const String& o,int f):name(s),op_name(o),flag(f){}

bool is_nan(double d)
{
	return !(d>0.0||d<=0.0);
}

bool is_nan(float d)
{
	return !(d>0.0f||d<=0.0f);
}

double round_digit(double v, unsigned n)
{
	if (v == 0.0) return v;
	double s = 1.0;
	if (v<0)
	{
		v = -v; 
		s = -1.0;
	}

	double _stepping = ::pow(10.0, ::ceil(::log10(v) - n));
	return s*_stepping*::round(v / _stepping);
}

template<typename T>
dcomplex pl_base1_triT<tiny_cpx<T> >::asin(value_type x)
{
#if _MSC_VER>1600
	return std::asin(k(x));
#else
	dcomplex _tmp=pl_log::g(dcomplex(-x.imag(),x.real())+pl_sqrt::g(-pl_pow::g(x,2)+1.0));
	return dcomplex(_tmp.imag(),-_tmp.real());
#endif
}

template<typename T>
dcomplex pl_base1_triT<tiny_cpx<T> >::acos(value_type x)
{
#if _MSC_VER>1600
	return std::acos(k(x));
#else
	dcomplex _tmp=pl_log::g(x+pl_sqrt::g(pl_pow::g(x,2)-1.0));
	return dcomplex(_tmp.imag(),-_tmp.real());
#endif
}

template<typename T>
dcomplex pl_base1_triT<tiny_cpx<T> >::atan(value_type x)
{
#if _MSC_VER>1600
	return std::atan(k(x));
#else
	dcomplex _tmp =pl_log::g(dcomplex(1.0+x.imag(),-x.real()))-pl_log::g(dcomplex(1.0-x.imag(),x.real()));
	return dcomplex(-0.5*_tmp.imag(),0.5*_tmp.real());
#endif
}

template struct pl_base1_triT<dcomplex>;
template struct pl_base1_triT<fcomplex>;

const op_info& pl_base0::info(){static op_info sm_info("",0);return sm_info;}

const op_info& pl_neg::info(){static op_info sm_info("math.neg",1);return sm_info;}
const op_info& pl_abs::info(){static op_info sm_info("math.abs",1);return sm_info;}
const op_info& pl_arg::info(){static op_info sm_info("math.arg",1);return sm_info;}
const op_info& pl_exp::info(){static op_info sm_info("math.exp",1);return sm_info;}
const op_info& pl_log::info(){static op_info sm_info("math.log",1);return sm_info;}
const op_info& pl_log10::info(){static op_info sm_info("math.log10",1);return sm_info;}
const op_info& pl_sqrt::info(){static op_info sm_info("math.sqrt",1);return sm_info;}

const op_info& pl_real::info(){static op_info sm_info("math.real",1);return sm_info;}
const op_info& pl_imag::info(){static op_info sm_info("math.imag",1);return sm_info;}
const op_info& pl_conj::info(){static op_info sm_info("math.conj",1);return sm_info;}
const op_info& pl_cpx::info(){static op_info sm_info("math.complex",0);return sm_info;}

const op_info& pl_sin::info(){static op_info sm_info("math.sin",1);return sm_info;}
const op_info& pl_cos::info(){static op_info sm_info("math.cos",1);return sm_info;}
const op_info& pl_tan::info(){static op_info sm_info("math.tan",1);return sm_info;}
const op_info& pl_asin::info(){static op_info sm_info("math.asin",1);return sm_info;}
const op_info& pl_acos::info(){static op_info sm_info("math.acos",1);return sm_info;}
const op_info& pl_atan::info(){static op_info sm_info("math.atan",1);return sm_info;}

#if EW_HAS_TRI_H
const op_info& pl_asinh::info(){static op_info sm_info("math.asinh",1);return sm_info;}
const op_info& pl_acosh::info(){static op_info sm_info("math.acosh",1);return sm_info;}
const op_info& pl_atanh::info(){static op_info sm_info("math.atanh",1);return sm_info;}
#endif

const op_info& pl_ne::info(){static op_info sm_info("rel.ne",0);return sm_info;}
const op_info& pl_eq::info(){static op_info sm_info("rel.eq",0);return sm_info;}
const op_info& pl_gt::info(){static op_info sm_info("rel.gt",0);return sm_info;}
const op_info& pl_ge::info(){static op_info sm_info("rel.ge",0);return sm_info;}
const op_info& pl_lt::info(){static op_info sm_info("rel.lt",0);return sm_info;}
const op_info& pl_le::info(){static op_info sm_info("rel.le",0);return sm_info;}

const op_info& pl_add::info(){static op_info sm_info("math.add",0);return sm_info;}
const op_info& pl_sub::info(){static op_info sm_info("math.sub",0);return sm_info;}
const op_info& pl_mod::info(){static op_info sm_info("math.mod",0);return sm_info;}

const op_info& pl_mul::info(){static op_info sm_info("math.mul",0);return sm_info;}
const op_info& pl_div::info(){static op_info sm_info("math.div",0);return sm_info;}
const op_info& pl_pow::info(){static op_info sm_info("math.pow",0);return sm_info;}

const op_info& pl_arr_mul::info(){static op_info sm_info("math.dot_mul",0);return sm_info;}
const op_info& pl_arr_div::info(){static op_info sm_info("math.dot_div",0);return sm_info;}
const op_info& pl_arr_pow::info(){static op_info sm_info("math.dot_pow",0);return sm_info;}

const op_info& pl_mat_mul::info(){static op_info sm_info("math.mul",0);return sm_info;}
const op_info& pl_mat_div::info(){static op_info sm_info("math.div",0);return sm_info;}
const op_info& pl_mat_pow::info(){static op_info sm_info("math.pow",0);return sm_info;}

const op_info& pl_bw_not::info(){static op_info sm_info("bitwise.not","__bitwise_not",0);return sm_info;}
const op_info& pl_bw_and::info(){static op_info sm_info("bitwise.and","__bitwise_and",0);return sm_info;}
const op_info& pl_bw_or::info(){static op_info sm_info("bitwise.or","__bitwise_or",0);return sm_info;}
const op_info& pl_bw_xor::info(){static op_info sm_info("bitwise.xor","__bitwise_xor",0);return sm_info;}

const op_info& pl_bw_shl::info(){static op_info sm_info("bitwise.shl",0);return sm_info;}
const op_info& pl_bw_shr::info(){static op_info sm_info("bitwise.shr",0);return sm_info;}

EW_LEAVE

