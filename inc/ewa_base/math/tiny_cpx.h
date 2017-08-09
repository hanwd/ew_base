
#ifndef __H_EW_MATH_TINY_CPX__
#define __H_EW_MATH_TINY_CPX__

#include "ewa_base/config.h"
#include "ewa_base/basic/hashing.h"
//#include "ewa_base/basic/string.h"
#include "ewa_base/math/math_promote.h"
#include <complex>
#include <cmath>

#ifndef M_PI
	#define M_PI       3.14159265358979323846
#endif

#ifndef M_PI_2
	#define M_PI_2     1.57079632679489661923
	#define M_PI_4     0.785398163397448309616
#endif

EW_ENTER


inline double sindeg(double deg)
{
	double np=deg/90.0;
	int64_t kp = (int64_t)np;
	if(np==double(kp))
	{
		switch(kp%4)
		{
		case -3:case 1: return  1.0;
		case -1:case 3: return -1.0;
		default: return 0.0;
		}
	}
	else
	{
		return ::sin(np*M_PI_2);
	}
}

inline double cosdeg(double deg)
{
	double np=deg/90.0;
	int kp=(int)np;
	if(np==double(kp))
	{
		switch(kp%4)
		{
		case 0: return  1.0;
		case -2:case 2: return -1.0;
		default: return 0.0;
		}
	}
	else
	{
		return ::cos(np*M_PI_2);
	}
}

inline double tandeg(double deg)
{
	double np = deg / 45.0;
	int64_t kp = (int64_t)np;
	if (np == double(kp))
	{
		switch ((kp % 8 + 8) % 8)
		{
		case 0: case 4: return 0;
		case 1: case 5: return 1;
		case 2: return std::numeric_limits<double>::max();
		case 6: return std::numeric_limits<double>::min();
		case 3: case 7: return -1;
		default: return 0.0;
		}
	}
	else
	{
		return tan(deg / 180.0 * M_PI);
	}
}

template<typename T>
class tiny_cpx
{
public:

	typedef T scalar;

	inline tiny_cpx(const T re=T(),const T im=T())
	{
		(*this)[0]=re;
		(*this)[1]=im;
	}

	inline tiny_cpx(const std::complex<T> &v)
	{
		(*this)[0]=v.real();
		(*this)[1]=v.imag();
	}

	template<typename Y>
	inline tiny_cpx(const tiny_cpx<Y> &c)
	{
		(*this)[0]=c[0];
		(*this)[1]=c[1];
	}

	inline operator scalar *()
	{
		return (scalar *)this;
	}

	inline operator const scalar *() const
	{
		return (const scalar *)this;
	}

	scalar real() const
	{
		return (*this)[0];
	}
	scalar imag() const
	{
		return (*this)[1];
	}

	scalar& real() { return (*this)[0]; }
	scalar& imag() { return (*this)[1]; }

	void real(scalar v)
	{
		(*this)[0]=v;
	}
	void imag(scalar v)
	{
		(*this)[1]=v;
	}

	template<typename T1>
	inline tiny_cpx<T>& operator= (const T1& _val)
	{
		real() = _val;
		return *this;
	}

	template<typename T1>
	inline tiny_cpx<T>& operator= (const tiny_cpx<T1>& _val)
	{
		real() = _val.real();
		imag() = _val.imag();
		return *this;
	}

	inline const tiny_cpx operator-() const
	{
		return tiny_cpx(-(*this)[0],-(*this)[1]);
	}

	inline const tiny_cpx &operator+() const
	{
		return *this;
	}

	double length() const
	{
		return ::sqrt((*this)[0]*(*this)[0]+(*this)[1]*(*this)[1]);
	}

	double phase() const
	{
		return ::atan2((*this)[1],(*this)[0]);
	}

	template<typename Y>
	inline tiny_cpx &operator += (const Y v)
	{
		(*this)[0] += v;
		return *this;
	}

	template<typename Y>
	inline tiny_cpx &operator += (const tiny_cpx<Y> &v)
	{
		(*this)[0] += v[0];
		(*this)[1] += v[1];
		return *this;
	}

	template<typename Y>
	inline tiny_cpx &operator -= (const Y v)
	{
		(*this)[0] -= v;
		return *this;
	}
	template<typename Y>
	inline tiny_cpx &operator -= (const tiny_cpx<Y> &v)
	{
		(*this)[0] -= v[0];
		(*this)[1] -= v[1];
		return *this;
	}

	template<typename Y>
	inline tiny_cpx &operator *= (const Y &v)
	{
		(*this)[0] *= v;
		(*this)[1] *= v;
		return *this;
	}

	template<typename Y>
	inline tiny_cpx &operator *= (const tiny_cpx<Y> &v)
	{
		T _re=(*this)[0]*v[0]-(*this)[1]*v[1];
		T _im=(*this)[0]*v[1]+(*this)[1]*v[0];
		(*this)[0]=_re;
		(*this)[1]=_im;
		return *this;
	}

	template<typename Y>
	inline tiny_cpx &operator /= (const Y v)
	{
		(*this)[0] /= v;
		(*this)[1] /= v;
		return *this;
	}

	template<typename Y>
	inline tiny_cpx &operator /= (const tiny_cpx<Y> &rhs)
	{
		T _r2=rhs[0]*rhs[0]+rhs[1]*rhs[1];
		T _re=((*this)[0]*rhs[0]+(*this)[1]*rhs[1])/_r2;
		T _im=((*this)[1]*rhs[0]-(*this)[0]*rhs[1])/_r2;
		(*this)[0]=_re;
		(*this)[1]=_im;
		return *this;
	}

	inline operator const std::complex<T> &() const
	{
		return *(const std::complex<T> *)(this);
	}

private:
	T val[2];
};

template<typename T> class hash_t<tiny_cpx<T> > : public hash_pod<tiny_cpx<T> > {};

typedef tiny_cpx<double> dcomplex;
typedef tiny_cpx<float>	fcomplex;


template<typename H, int N>
struct cpx_promote_real : public H {};

template<typename H>
struct cpx_promote_real<H, 1> : public H
{
	typedef typename H::type scalar;
	typedef typename tiny_cpx<typename H::type> promoted;
	typedef scalar type;
};

template<typename H>
struct cpx_promote_real<H, 2> : public H
{
	typedef typename H::type scalar;
	typedef typename tiny_cpx<typename H::type> promoted;
	typedef promoted type;
};

template<typename X, typename Y>
struct cpx_promote : public cpx_promote_real<scr_promote<X, Y>, scr_promote<X, Y>::value ? 1 : 0>{};

template<typename X, typename Y>
struct cpx_promote<tiny_cpx<X>, Y > : public cpx_promote_real<scr_promote<X, Y>, scr_promote<X, Y>::value ? 2 : 0>{};

template<typename X, typename Y>
struct cpx_promote<X, tiny_cpx<Y> > : public cpx_promote_real<scr_promote<X, Y>, scr_promote<X, Y>::value ? 2 : 0>{};

template<typename X, typename Y>
struct cpx_promote<tiny_cpx<X>, tiny_cpx<Y> > : public cpx_promote_real<scr_promote<X, Y>, scr_promote<X, Y>::value ? 2 : 0>{};


template<typename X,typename Y>
inline typename cpx_promote<X,Y>::promoted operator + (const tiny_cpx<X> &lhs,const Y rhs)
{
	return typename cpx_promote<X,Y>::promoted(lhs[0] + rhs,lhs[1]);
}
template<typename X,typename Y>
inline typename cpx_promote<X,Y>::promoted operator + (const X lhs,const tiny_cpx<Y> &rhs)
{
	return typename cpx_promote<X,Y>::promoted(lhs + rhs[0],rhs[1]);
}
template<typename X,typename Y>
inline typename cpx_promote<X,Y>::promoted operator + (const tiny_cpx<X> &lhs,const tiny_cpx<Y> &rhs)
{
	return typename cpx_promote<X,Y>::promoted(lhs[0] + rhs[0],lhs[1] + rhs[1]);
}



template<typename X,typename Y>
inline typename cpx_promote<X,Y>::promoted operator - (const tiny_cpx<X> &lhs,const Y rhs)
{
	return typename cpx_promote<tiny_cpx<X>,Y>::type(lhs[0] - rhs,lhs[1]);
}
template<typename X,typename Y>
inline typename cpx_promote<X,Y>::promoted operator - (const X lhs,const tiny_cpx<Y> &rhs)
{
	return typename cpx_promote<tiny_cpx<X>,Y>::type(lhs - rhs[0],-rhs[1]);
}
template<typename X,typename Y>
inline typename cpx_promote<X,Y>::promoted operator - (const tiny_cpx<X> &lhs,const tiny_cpx<Y> &rhs)
{
	return typename cpx_promote<X,Y>::promoted(lhs[0] - rhs[0],lhs[1] - rhs[1]);
}

template<typename X,typename Y>
inline typename cpx_promote<X,Y>::promoted operator * (const tiny_cpx<X> &lhs,const Y rhs)
{
	return typename cpx_promote<X,Y>::promoted(lhs[0] * rhs,lhs[1] * rhs);
}
template<typename X,typename Y>
inline typename cpx_promote<X,Y>::promoted operator * (const X lhs,const tiny_cpx<Y> &rhs)
{
	return typename cpx_promote<tiny_cpx<X>,Y>::type(lhs * rhs[0], lhs * rhs[1]);
}
template<typename X,typename Y>
inline typename cpx_promote<X,Y>::promoted operator * (const tiny_cpx<X> &lhs,const tiny_cpx<Y> &rhs)
{
	typename cpx_promote<X,Y>::scalar tmp_re=lhs[0]*rhs[0]-lhs[1]*rhs[1];
	typename cpx_promote<X,Y>::scalar tmp_im=lhs[0]*rhs[1]+lhs[1]*rhs[0];
	return typename cpx_promote<X,Y>::promoted(tmp_re,tmp_im);
}


template<typename X,typename Y>
inline typename cpx_promote<X,Y>::promoted operator / (const tiny_cpx<X> &lhs,const Y rhs)
{
	return typename cpx_promote<tiny_cpx<X>,Y>::type(lhs[0] / rhs,lhs[1] / rhs);
}
template<typename X,typename Y>
inline typename cpx_promote<X,Y>::promoted operator / (const X lhs,const tiny_cpx<Y> &rhs)
{
	double _r2=rhs[0]*rhs[0]+rhs[1]*rhs[1];
	return typename cpx_promote<X,Y>::promoted
		   (
			   +lhs*rhs[0]/_r2,
			   -lhs*rhs[1]/_r2
		   );
}
template<typename X,typename Y>
inline typename cpx_promote<X,Y>::promoted operator / (const tiny_cpx<X> &lhs,const tiny_cpx<Y> &rhs)
{
	double _r2=rhs[0]*rhs[0]+rhs[1]*rhs[1];
	return typename cpx_promote<X,Y>::promoted
		   (
			   (lhs[0]*rhs[0]+lhs[1]*rhs[1])/_r2,
			   (lhs[1]*rhs[0]-lhs[0]*rhs[1])/_r2
		   );
}

template<typename T>
inline bool operator==(const tiny_cpx<T> &lhs,const tiny_cpx<T> &rhs)
{
	return lhs[0]==rhs[0] && lhs[1]==rhs[1];
}

template<typename T>
inline bool operator!=(const tiny_cpx<T> &lhs,const tiny_cpx<T> &rhs)
{
	return lhs[0]!=rhs[0] || lhs[1]!=rhs[1];
}

template<typename T>
inline tiny_cpx<T> conj(const tiny_cpx<T>& v)
{
	return tiny_cpx<T>(v.real, -v.imag);
}

template<typename T>
inline tiny_cpx<T> cpx_rotate(const tiny_cpx<T>& v, double angle)
{
	return v * tiny_cpx<T>(cos(angle), sin(angle));
}



EW_LEAVE

namespace tl
{
	template<typename T> struct is_pod<ew::tiny_cpx<T> > : public value_type<true>{};
};


#endif


