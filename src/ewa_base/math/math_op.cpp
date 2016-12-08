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

class const_floating_values
{
public:
	const_floating_values(double v):val1(v),val2(-v){}
	const_floating_values(double v,float f):val1(v,f),val2(-v,-f){}

	class values
	{
	public:
		values(double v):f64(v),f32(v){}
		values(double v,float f):f64(v),f32(f){}
		double f64;
		float f32;

		inline operator float() const {return f32;}
		inline operator double() const {return f64;}
	};

	values val1,val2;
};


const_floating_values make_floating_values(uint16_t v1,uint16_t v2,uint16_t v3,uint16_t v4)
{
	union
	{
		uint16_t ival[4];
		double fval;
	}fvalue;

	fvalue.ival[0]=v1;
	fvalue.ival[1]=v2;
	fvalue.ival[2]=v3;
	fvalue.ival[3]=v4;

	return fvalue.fval;
}

const const_floating_values _floating_nan=make_floating_values(0,0,0,32760);
const const_floating_values _floating_inf=make_floating_values(0,0,0,32752);
const const_floating_values _floating_max(std::numeric_limits<double>::max(),std::numeric_limits<float>::max());

template<typename T>
T numeric_trait_floating<T>::min_value()
{
	return _floating_max.val2;
}

template<typename T>
T numeric_trait_floating<T>::max_value()
{
	return _floating_max.val1;
}

template<typename T>
T numeric_trait_floating<T>::inf_value()
{
	return _floating_inf.val1;
}

template<typename T>
T numeric_trait_floating<T>::nan_value()
{
	return _floating_nan.val1;
}



template<typename T>
T numeric_trait_integer<T>::min_value()
{
	return tl::is_signed_type<T>::value?max_value()+T(1):0;
}

template<typename T>
T numeric_trait_integer<T>::max_value()
{
	return std::numeric_limits<T>::max();
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


template class numeric_trait_floating<double>;
template class numeric_trait_floating<float>;

template class numeric_trait_integer<int32_t>;
template class numeric_trait_integer<uint32_t>;
template class numeric_trait_integer<int64_t>;
template class numeric_trait_integer<uint64_t>;


EW_LEAVE

