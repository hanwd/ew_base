
#ifndef __H_EW_MATH_MATH_OP__
#define __H_EW_MATH_MATH_OP__

#include "ewa_base/math/tiny_cpx.h"
#include "ewa_base/scripting/callable_data.h"
#include "ewa_base/scripting/callable_class.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE op_info
{
public:
	op_info(){}
	op_info(const String& s,int f=0):name(s),flag(f){}

	String name;
	int flag;
};


struct DLLIMPEXP_EWA_BASE pl_base0
{
	enum
	{
		DISPATCH_NUM=1<<0,
		DISPATCH_ARR=1<<1,
	};

	static inline const std::complex<double>& k(const dcomplex& v){return v;}
	template<typename T> static inline T k(T v){return v;}

	static inline bool metatable_call(CallableMetatable*, Variant&,Variant&, Variant&){ return false; }

	static inline bool metatable_call_var(Variant& r, Variant& v1, Variant& v2){return false;}


	static const op_info& info();
};


struct DLLIMPEXP_EWA_BASE pl_base1 : public pl_base0
{
	static const int dispatch_type=DISPATCH_NUM;
};

struct DLLIMPEXP_EWA_BASE pl_base2 : public pl_base0
{
	static const int dispatch_type=DISPATCH_NUM;
};

typedef tl::mk_typelist<int32_t,int64_t>::type TL_I;
typedef tl::mk_typelist<int32_t,int64_t,float,double>::type TL_1;
typedef tl::mk_typelist<int32_t,int64_t,float,double,tiny_cpx<float>,tiny_cpx<double> >::type TL_2;
typedef tl::mk_typelist<int32_t,int64_t,float,double,float,double>::type TL_S;

template<typename L,typename X,typename S>
struct swtich_tl_with_value : public tl::enable_tl<L,X,typename S::template at<L::template id<X>::value>::type>
{
	static const bool value=L::template id<X>::value>=0;
};

template<typename L,typename X,typename Y=X>
struct enable_tl_with_value : public tl::enable_tl<L,X,Y>
{
	static const bool value=L::template id<X>::value>=0;
};

template<typename V,typename Y>
struct enable_if_with_value : public tl::enable_if_c<V,Y>
{
	static const bool value=V::value;
};

template<typename T>
struct DLLIMPEXP_EWA_BASE pl_base1_numT : public enable_tl_with_value<TL_1,T,double>
{
	typedef T value_type;
	typedef double return_type;

	static double k(value_type v){return double(v);}

	static double sqrt(value_type x){return ::sqrt(k(x));}
	static double log(value_type x){return ::log(k(x));}
	static double log10(value_type x){return ::log10(k(x));}
	static double exp(value_type x){return ::exp(k(x));}
};


template<typename T>
struct DLLIMPEXP_EWA_BASE pl_base1_numT<tiny_cpx<T> > : public enable_tl_with_value<TL_1,T,dcomplex>
{
	typedef tiny_cpx<T> value_type;
	typedef dcomplex return_type;

	static const std::complex<double>& k(const dcomplex& v){return v;}
	static std::complex<double> k(const fcomplex& v){return std::complex<double>(v.real(),v.imag());}

	static return_type sqrt(value_type x){return std::sqrt(k(x));}
	static return_type log(value_type x){return std::log(k(x));}
	static return_type log10(value_type x){return std::log10(k(x));}
	static return_type exp(value_type x){return std::exp(k(x));}

};

struct DLLIMPEXP_EWA_BASE pl_base1_num : public pl_base1
{
	template<typename T>
	struct rebind : public pl_base1_numT<T>{};
};


struct DLLIMPEXP_EWA_BASE pl_sqrt : public pl_base1_num
{
	template<typename T>
	static typename rebind<T>::type g(const T& v){return pl_base1_numT<T>::sqrt(v);}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_exp : public pl_base1_num
{
	template<typename T>
	static typename rebind<T>::type g(const T& v){return pl_base1_numT<T>::exp(v);}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_log : public pl_base1_num
{
	template<typename T>
	static typename rebind<T>::type g(const T& v){return pl_base1_numT<T>::log(v);}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_log10 : public pl_base1_num
{
	template<typename T>
	static typename rebind<T>::type g(const T& v){return pl_base1_numT<T>::log10(v);}

	static const op_info& info();
};


#if defined(_MSC_VER) && _MSC_VER>1600
#define EW_HAS_TRI_H 1
#else
#define EW_HAS_TRI_H 0
#endif


template<typename T>
struct DLLIMPEXP_EWA_BASE pl_base1_triT : public enable_tl_with_value<TL_1,T,double>
{
	typedef T value_type;
	typedef double return_type;

	static double k(value_type v){return double(v);}

	static return_type sin(value_type x){return ::sin(k(x));}
	static return_type cos(value_type x){return ::cos(k(x));}
	static return_type tan(value_type x){return ::tan(k(x));}
	static return_type asin(value_type x){return ::asin(k(x));}
	static return_type acos(value_type x){return ::acos(k(x));}
	static return_type atan(value_type x){return ::atan(k(x));}

#if EW_HAS_TRI_H
	static return_type asinh(value_type x){return std::asinh(std::complex<double>(x,0)).real();}
	static return_type acosh(value_type x){return  std::acosh(std::complex<double>(x,0)).real();}
	static return_type atanh(value_type x){return  std::atanh(std::complex<double>(x,0)).real();}
#endif

};

template<typename T>
struct DLLIMPEXP_EWA_BASE pl_base1_triT<tiny_cpx<T> > : public enable_tl_with_value<TL_1,T,dcomplex>
{
	typedef const tiny_cpx<T>& value_type;
	typedef dcomplex return_type;

	static const std::complex<double>& k(const dcomplex& v){return v;}
	static std::complex<double> k(const fcomplex& v){return std::complex<double>(v.real(),v.imag());}

	static return_type sin(value_type x){return std::sin(k(x));}
	static return_type cos(value_type x){return std::cos(k(x));}
	static return_type tan(value_type x){return std::tan(k(x));}

	static return_type asin(value_type x);
	static return_type acos(value_type x);
	static return_type atan(value_type x);

#if EW_HAS_TRI_H
	static return_type asinh(value_type x){return std::asinh(k(x));}
	static return_type acosh(value_type x){return std::acosh(k(x));}
	static return_type atanh(value_type x){return std::atanh(k(x));}
#endif

};

struct DLLIMPEXP_EWA_BASE pl_base1_tri : public pl_base1
{
	template<typename T>
	struct rebind : public pl_base1_triT<T>{};
};

struct DLLIMPEXP_EWA_BASE pl_sin : public pl_base1_tri
{
	template<typename T>
	static typename rebind<T>::type g(const T& v){return pl_base1_triT<T>::sin(v);}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_cos : public pl_base1_tri
{
	template<typename T>
	static typename rebind<T>::type g(const T& v){return pl_base1_triT<T>::cos(v);}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_tan : public pl_base1_tri
{
	template<typename T>
	static typename rebind<T>::type g(const T& v){return pl_base1_triT<T>::tan(v);}

	static const op_info& info();
};


struct DLLIMPEXP_EWA_BASE pl_asin : public pl_base1_tri
{
	template<typename T>
	static typename rebind<T>::type g(const T& v){return pl_base1_triT<T>::asin(v);}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_acos : public pl_base1_tri
{
	template<typename T>
	static typename rebind<T>::type g(const T& v){return pl_base1_triT<T>::acos(v);}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_atan : public pl_base1_tri
{
	template<typename T>
	static typename rebind<T>::type g(const T& v){return pl_base1_triT<T>::atan(v);}

	static const op_info& info();
};

#if EW_HAS_TRI_H
struct DLLIMPEXP_EWA_BASE pl_asinh : public pl_base1_tri
{
	template<typename T>
	static typename rebind<T>::type g(const T& v){return pl_base1_triT<T>::asinh(v);}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_acosh : public pl_base1_tri
{
	template<typename T>
	static typename rebind<T>::type g(const T& v){return pl_base1_triT<T>::acosh(v);}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_atanh : public pl_base1_tri
{
	template<typename T>
	static typename rebind<T>::type g(const T& v){return pl_base1_triT<T>::atanh(v);}

	static const op_info& info();
};
#endif


struct DLLIMPEXP_EWA_BASE pl_neg : public pl_base1
{

	template<typename T>
	struct rebind : public enable_tl_with_value<TL_2,T>{};

	static int64_t g(int64_t a){return -a;}
	static double g(double a){return -a;}
	static dcomplex g(const dcomplex& a){return -a;}

	static const op_info& info();
};


struct DLLIMPEXP_EWA_BASE pl_abs : public pl_base1
{

	template<typename T>
	struct rebind : public swtich_tl_with_value<TL_2,T,TL_S>{};

	static int64_t g(int64_t a){return a>0?a:-a;}
	static double g(double a){return a>0?a:-a;}
	static double g(const dcomplex& a){return a.length();}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_arg : public pl_base1
{

	template<typename T>
	struct rebind : public enable_tl_with_value<TL_2,T,double>{};

	static double g(int64_t){return 0.0;}
	static double g(double){return 0.0;}
	static double g(const dcomplex& a){ return ::atan2(a.imag(),a.real());}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_bw_not : public pl_base1
{
	template<typename T> struct rebind : public enable_tl_with_value<TL_I,T>{};

	template<typename T>
	static typename rebind<T>::type g(T v1){return ~v1;}

	static const op_info& info();
};


struct DLLIMPEXP_EWA_BASE pl_conj : public pl_base1
{

	template<typename T>
	struct rebind : public enable_tl_with_value<TL_2,T>{};

	static int64_t g(int64_t a){return a;}
	static double g(double a){return a;}
	static dcomplex g(const dcomplex& a){return dcomplex(a.real(),-a.imag());}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_real : public pl_base1
{

	template<typename T>
	struct rebind : public enable_tl_with_value<TL_2,T,double>{};

	static double g(int64_t a){return a;}
	static double g(double a){return a;}
	static double g(const dcomplex& a){return a.real();}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_imag : public pl_base1
{

	template<typename T>
	struct rebind : public enable_tl_with_value<TL_2,T,double>{};

	static double g(int64_t){return 0.0;}
	static double g(double){return 0.0;}
	static double g(const dcomplex& a){return a.imag();}

	static const op_info& info();
};



struct DLLIMPEXP_EWA_BASE pl_cpx : public pl_base2
{
	template<typename T1,typename T2>
	struct rebind : public enable_if_with_value<cpx_promote<T1,T2>,dcomplex>{};

	static dcomplex g(double v1,double v2){return dcomplex(v1,v2);}
	static dcomplex g(const dcomplex& v1,double v2){return dcomplex(v1.real(),v1.imag()+v2);}
	static dcomplex g(double v1,const dcomplex& v2){return dcomplex(v1-v2.imag(),v2.real());}
	static dcomplex g(const dcomplex& v1,const dcomplex& v2){return dcomplex(v1.real()-v2.imag(),v2.real()-v1.imag());}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_base2_num : public pl_base2
{
	template<typename T1,typename T2>
	struct rebind : public cpx_promote<T1,T2>{};
};




struct DLLIMPEXP_EWA_BASE pl_base2_mat : public pl_base2
{
	static const int dispatch_type=DISPATCH_ARR;

	template<typename T1,typename T2>
	struct rebind : public arr_promote<T1,T2>{};
		
};

struct DLLIMPEXP_EWA_BASE pl_add : public pl_base2_num
{
	template<typename T1,typename T2>
	static typename rebind<T1,T2>::type g(const T1& v1,const T2& v2){return v1+v2;}

	static inline bool metatable_call(CallableMetatable* mt, Variant& r,Variant& v1, Variant& v2)
	{ 
		return mt->__add(r,v1,v2);;
	}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_sub : public pl_base2_num
{
	template<typename T1,typename T2>
	static typename rebind<T1,T2>::type g(const T1& v1,const T2& v2){return v1-v2;}

	static inline bool metatable_call(CallableMetatable* mt, Variant& r, Variant& v1, Variant& v2)
	{
		return mt->__sub(r, v1, v2);;
	}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_mul : public pl_base2_num
{
	template<typename T1,typename T2>
	static typename rebind<T1,T2>::type g(const T1& v1,const T2& v2){return v1*v2;}

	static inline bool metatable_call(CallableMetatable* mt, Variant& r, Variant& v1, Variant& v2)
	{
		return mt->__mul(r, v1, v2);;
	}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_arr_mul : public pl_mul
{
	static const op_info& info();
};


struct DLLIMPEXP_EWA_BASE pl_mat_mul : public pl_base2_mat
{

	template<typename T1,typename T2>
	static typename rebind<T1,T2>::type g(const T1& v1,const T2& v2){return v1*v2;}

	template<typename T1,typename T2>
	static typename rebind<arr_xt<T1>,T2>::type g(const arr_xt<T1>& v1,const T2& v2)
	{
		typedef rebind<T1,T2>::type type;
		arr_xt<type> result;
		size_t sz=v1.size();
		result.resize(v1.size_ptr(),sz);
		for(size_t i=0;i<sz;i++) result[i]=v1[i]*v2;
		return EW_MOVE(result);
		
	}
	template<typename T1,typename T2>
	static typename rebind<T1,arr_xt<T2> >::type g(const T1& v1,const arr_xt<T2>& v2)
	{
		typedef rebind<T1,T2>::type type;
		arr_xt<type> result;
		size_t sz=v2.size();
		result.resize(v2.size_ptr(),sz);
		for(size_t i=0;i<sz;i++) result[i]=v1*v2[i];
		return EW_MOVE(result);
	}

	template<typename T1,typename T2>
	static typename rebind<arr_xt<T1>,arr_xt<T2> >::type g(const arr_xt<T1>& v1,const arr_xt<T2>& v2)
	{
		typedef rebind<T1,T2>::type type;
		arr_xt<type> result;

		const arr_xt_dims& s1=v1.size_ptr();
		const arr_xt_dims& s2=v2.size_ptr();

		size_t k3=s1[2]*s1[3]*s1[4]*s1[5];

		if(s1[1]!=s2[0]||k3!=s2[2]*s2[3]*s2[4]*s2[5])
		{
			Exception::XError("invalid_matrix_size");
		}

		result.resize(s1[0],s2[1],s1[2],s1[3],s1[4],s1[5]);

		for(size_t k=0;k<k3;k++) for(size_t i=0;i<s1[0];i++) for(size_t j=0;j<s2[1];j++)
		{
			for(size_t n=0;n<s1[1];n++)
			{
				result(i,j,k)+=v1(i,n,k)*v2(n,j,k);
			}
		}

		return EW_MOVE(result);
	}

	static inline bool metatable_call(CallableMetatable* mt, Variant& r, Variant& v1, Variant& v2)
	{
		return mt->__mul(r, v1, v2);;
	}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_div : public pl_base2_num
{
	template<typename T1,typename T2>
	static typename rebind<T1,T2>::type g(const T1& v1,const T2& v2){return v1/v2;}

	static inline bool metatable_call(CallableMetatable* mt, Variant& r, Variant& v1, Variant& v2)
	{
		return mt->__div(r, v1, v2);;
	}

	static const op_info& info();
};


struct DLLIMPEXP_EWA_BASE pl_arr_div : public pl_div
{
	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_mat_div : public pl_base2_mat
{
	template<typename T1,typename T2>
	static typename rebind<T1,T2>::type g(const T1& v1,const T2& v2){return v1/v2;}

	template<typename T1,typename T2>
	static typename rebind<arr_xt<T1>,T2>::type g(const arr_xt<T1>& v1,const T2& v2)
	{
		typedef rebind<T1,T2>::type type;
		arr_xt<type> result;
		size_t sz=v1.size();
		result.resize(v1.size_ptr(),sz);
		for(size_t i=0;i<sz;i++) result[i]=v1[i]/v2;
		return EW_MOVE(result);
		
	}
	template<typename T1,typename T2>
	static typename rebind<T1,arr_xt<T2> >::type g(const T1& v1,const arr_xt<T2>& v2)
	{
		typedef rebind<T1,T2>::type type;
		arr_xt<type> result;
		Exception::XError("matrix_div_not_implement");
		return EW_MOVE(result);
	}

	template<typename T1,typename T2>
	static typename rebind<arr_xt<T1>,arr_xt<T2> >::type g(const arr_xt<T1>& v1,const arr_xt<T2>& v2)
	{
		typedef rebind<T1,T2>::type type;
		arr_xt<type> result;
		Exception::XError("matrix_div_not_implement");
		return EW_MOVE(result);
	}

	static inline bool metatable_call(CallableMetatable* mt, Variant& r, Variant& v1, Variant& v2)
	{
		return mt->__div(r, v1, v2);;
	}

	static const op_info& info();
};


struct DLLIMPEXP_EWA_BASE pl_pow : public pl_base2_num
{

	static double k(int64_t v){return double(v);}
	static double k(double v){return double(v);}
	static const std::complex<double>& k(const dcomplex& v){return (const std::complex<double>&)(v);}

	template<typename T1>
	static typename rebind<T1,int64_t>::type g(const T1& v1,int64_t v2)
	{
		return std::pow(k(v1),int32_t(v2));
	}

	template<typename T1>
	static typename rebind<T1,int64_t>::type g(const T1& v1,int32_t v2)
	{
		return std::pow(k(v1),int32_t(v2));
	}

	template<typename T1>
	static typename rebind<T1,double>::type g(const T1& v1,double v2)
	{
		return std::pow(k(v1),v2);
	}

	template<typename T1>
	static typename rebind<T1,dcomplex>::type g(const T1& v1,const dcomplex& v2)
	{
		return std::pow(k(v1),k(v2));
	}


	static const op_info& info();
};



struct DLLIMPEXP_EWA_BASE pl_arr_pow : public pl_pow
{
	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_mat_pow : public pl_base2_mat
{

	template<typename T1,typename T2>
	static typename rebind<T1,T2>::type g(const T1& v1,const T2& v2){return pl_pow::g(v1,v2);}

	template<typename T1,typename T2>
	static typename rebind<arr_xt<T1>,T2>::type g(const arr_xt<T1>& v1,const T2& v2)
	{
		typedef rebind<T1,T2>::type type;
		arr_xt<type> result;
		Exception::XError("matrix_pow_not_implement");
		return EW_MOVE(result);
		
	}
	template<typename T1,typename T2>
	static typename rebind<T1,arr_xt<T2> >::type g(const T1& v1,const arr_xt<T2>& v2)
	{
		typedef rebind<T1,T2>::type type;
		arr_xt<type> result;
		Exception::XError("matrix_pow_not_implement");
		return EW_MOVE(result);
	}

	template<typename T1,typename T2>
	static typename rebind<arr_xt<T1>,arr_xt<T2> >::type g(const arr_xt<T1>& v1,const arr_xt<T2>& v2)
	{
		typedef rebind<T1,T2>::type type;
		arr_xt<type> result;
		Exception::XError("matrix_pow_not_implement");		
		return EW_MOVE(result);
	}

	static const op_info& info();
};


struct DLLIMPEXP_EWA_BASE pl_mod : public pl_base2_num
{

	template<typename T1,typename T2> struct rebind : public scr_promote<T1,T2>{};

	static double g(double v1,double v2){ return v1-::floor(v1/v2)*v2;}
	static double g(int64_t v1,double v2){ return v1-::floor(v1/v2)*v2;;}
	static double g(double v1,int64_t v2){ return v1-::floor(v1/v2)*v2;;}
	static int64_t g(int64_t v1,int64_t v2){ return v1%v2;}

	static const op_info& info();
};



template<typename T1,typename T2>
struct DLLIMPEXP_EWA_BASE pl_base2_relT
{
	typedef typename scr_promote<T1,T2>::type scalar;
	typedef bool return_type;

	static inline return_type lt(const T1& v1,const T2& v2){return scalar(v1)< scalar(v2);}
	static inline return_type le(const T1& v1,const T2& v2){return scalar(v1)<=scalar(v2);}
	static inline return_type gt(const T1& v1,const T2& v2){return scalar(v1)> scalar(v2);}
	static inline return_type ge(const T1& v1,const T2& v2){return scalar(v1)>=scalar(v2);}
	static inline return_type eq(const T1& v1,const T2& v2){return scalar(v1)==scalar(v2);}
	static inline return_type ne(const T1& v1,const T2& v2){return scalar(v1)!=scalar(v2);}
};

template<typename T1>
struct DLLIMPEXP_EWA_BASE pl_base2_relT<T1,T1>
{
	typedef T1 scalar;
	typedef scalar T2;
	typedef bool return_type;

	static inline return_type lt(const T1& v1,const T2& v2){return (v1)< (v2);}
	static inline return_type le(const T1& v1,const T2& v2){return (v1)<=(v2);}
	static inline return_type gt(const T1& v1,const T2& v2){return (v1)> (v2);}
	static inline return_type ge(const T1& v1,const T2& v2){return (v1)>=(v2);}
	static inline return_type eq(const T1& v1,const T2& v2){return (v1)==(v2);}
	static inline return_type ne(const T1& v1,const T2& v2){return (v1)!=(v2);}
};


struct DLLIMPEXP_EWA_BASE pl_base2_rel : public pl_base2
{
	static const int dispatch_type=DISPATCH_ARR;

	template<typename T1,typename T2>
	struct rebind : public enable_if_with_value<scr_promote<T1,T2>,bool>{};

	template<typename T1>
	struct rebind1
	{
		typedef bool type;
		static const bool value = true;
	};
	template<typename T1,typename T2>
	struct rebind2 : public tl::select_base < tl::is_same_type<T1, T2>::value, rebind1<T1>, rebind<T1, T2> >{};

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_eq : public pl_base2_rel
{
	template<typename T1, typename T2>
	struct rebind : public rebind2<T1, T2>{};

	template<typename T1,typename T2>
	static inline typename rebind<T1,T2>::type g(const T1& v1,const T2& v2){return pl_base2_relT<T1,T2>::eq(v1,v2);}

	static inline bool metatable_call(CallableMetatable* mt, Variant& r, Variant& v1, Variant& v2)
	{
		return mt->__eq(r, v1, v2);;
	}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_ne : public pl_base2_rel
{
	template<typename T1, typename T2>
	struct rebind : public rebind2<T1, T2>{};

	template<typename T1,typename T2>
	static inline typename rebind<T1,T2>::type g(const T1& v1,const T2& v2){return pl_base2_relT<T1,T2>::ne(v1,v2);}

	static inline bool metatable_call(CallableMetatable* mt, Variant& r, Variant& v1, Variant& v2)
	{
		return mt->__ne(r, v1, v2);;
	}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_gt : public pl_base2_rel
{
	template<typename T1,typename T2>
	static inline typename rebind<T1,T2>::type g(const T1& v1,const T2& v2){return pl_base2_relT<T1,T2>::gt(v1,v2);}

	static inline bool metatable_call(CallableMetatable* mt, Variant& r, Variant& v1, Variant& v2)
	{
		return mt->__gt(r, v1, v2);;
	}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_ge : public pl_base2_rel
{
	template<typename T1,typename T2>
	static inline typename rebind<T1,T2>::type g(const T1& v1,const T2& v2){return pl_base2_relT<T1,T2>::ge(v1,v2);}

	static inline bool metatable_call(CallableMetatable* mt, Variant& r, Variant& v1, Variant& v2)
	{
		return mt->__ge(r, v1, v2);;
	}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_lt : public pl_base2_rel
{
	template<typename T1,typename T2>
	static inline typename rebind<T1,T2>::type g(const T1& v1,const T2& v2){return pl_base2_relT<T1,T2>::lt(v1,v2);}

	static inline bool metatable_call(CallableMetatable* mt, Variant& r, Variant& v1, Variant& v2)
	{
		return mt->__lt(r, v1, v2);;
	}

	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_le : public pl_base2_rel
{
	template<typename T1,typename T2>
	static inline typename rebind<T1,T2>::type g(const T1& v1,const T2& v2){return pl_base2_relT<T1,T2>::le(v1,v2);}

	static inline bool metatable_call(CallableMetatable* mt, Variant& r, Variant& v1, Variant& v2)
	{
		return mt->__le(r, v1, v2);;
	}

	static const op_info& info();
};

template<typename T1,typename T2> struct pl2_bw_rebind : public tl::value_type<false>{};
template<> struct pl2_bw_rebind<int64_t,int64_t> : public tl::value_type<true>{typedef int64_t type;};

struct DLLIMPEXP_EWA_BASE pl_base2_bw : public pl_base2
{
	template<typename T1,typename T2>
	struct rebind : public pl2_bw_rebind<T1,T2>{};
};

struct DLLIMPEXP_EWA_BASE pl_bw_and : public pl_base2_bw
{
	static int64_t g(int64_t x,int64_t y){return x&y;}
	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_bw_or : public pl_base2_bw
{
	static int64_t g(int64_t x,int64_t y){return x|y;}
	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_bw_xor : public pl_base2_bw
{
	static int64_t g(int64_t x,int64_t y){return x^y;}
	static const op_info& info();
};

struct DLLIMPEXP_EWA_BASE pl_bw_shl : public pl_base2_bw
{
	static int64_t g(int64_t x,int64_t y){return x<<y;}
	static const op_info& info();

	static inline bool metatable_call(CallableMetatable* mt, Variant& r, Variant& v1, Variant& v2)
	{
		return mt->__shl(r, v1, v2);;
	}
};

struct DLLIMPEXP_EWA_BASE pl_bw_shr : public pl_base2_bw
{
	static int64_t g(int64_t x,int64_t y){return x>>y;}
	static const op_info& info();

	static inline bool metatable_call(CallableMetatable* mt, Variant& r, Variant& v1, Variant& v2)
	{
		return mt->__shr(r, v1, v2);;
	}
};


EW_LEAVE

#endif
