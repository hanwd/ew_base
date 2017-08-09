#ifndef __H_EW_MATH_TINY_VEC__
#define __H_EW_MATH_TINY_VEC__

#include "ewa_base/math/tiny_cpx.h"

EW_ENTER


template<typename T,int N>
class tiny_storage
{
public:
	T val[N];

	tiny_storage(){}

	tiny_storage(const T& v1, const T& v2)
	{
		(*this)[0] = v1;
		(*this)[1] = v2;
		if (N > 2) (*this)[2] = T();
		if (N > 3) (*this)[3] = T();
	}

	tiny_storage(const T& v1, const T& v2, const T& v3)
	{
		(*this)[0] = v1;
		(*this)[1] = v2;
		if (N>2) (*this)[2] = v3;
	}

	tiny_storage(const T& v1, const T& v2, const T& v3, const T& v4)
	{
		(*this)[0] = v1;
		(*this)[1] = v2;
		if (N>2) (*this)[2] = v3;
		if (N>3) (*this)[3] = v4;
	}

	T& operator[](size_t n)
	{
		EW_ASSERT(n<N);
		return val[n];
	}

	const T& operator[](size_t n) const
	{
		EW_ASSERT(n<N);
		return val[n];
	}

	T* data(){return (T*)this;}
	const T* data() const {return (const T*)this;}

};

template<typename T,int M,int N>
class tiny_storage2
{
public:
	T val[M][N];

	T& operator[](size_t n)
	{
		EW_ASSERT(n<M*N);
		return ((T*)val)[n];
	}

	const T& operator[](size_t n) const
	{
		EW_ASSERT(n<M*N);
		return ((T*)val)[n];
	}

	T* data(){return (T*)this;}
	const T* data() const {return (const T*)this;}

};
template<typename T1,typename T2,int N>
struct tiny_opx
{
	static const int M=N-1;
	typedef typename cpx_promote<T1,T2>::type T3;
	typedef tiny_opx<T1,T2,M> bopx;

	template<typename P>
	static void g(T1* lhs,const T2* rhs)
	{
		bopx::template g<P>(lhs,rhs);
		lhs[M]=P::g(rhs[M]);
	}

	template<typename P>
	static void k(T1* lhs,const T2& rhs)
	{
		bopx::template k<P>(lhs,rhs);
		lhs[M]=P::g(rhs);
	}


	static bool eq(const T1* lhs,const T2* rhs)
	{
		if(lhs[M]!=rhs[M]) return false;
		return bopx::eq(lhs,rhs);
	}

	static void eq_set(T1* lhs,const T2* rhs)
	{
		bopx::eq_set(lhs,rhs);
		lhs[M]=rhs[M];
	}

	static void eq_neg(T1* lhs,const T2* rhs)
	{
		bopx::eq_neg(lhs,rhs);
		lhs[M]=-rhs[M];
	}

	static void eq_add(T1* lhs,const T2* rhs)
	{
		bopx::eq_add(lhs,rhs);
		lhs[M]+=rhs[M];
	}

	static void eq_sub(T1* lhs,const T2* rhs)
	{
		bopx::eq_sub(lhs,rhs);
		lhs[M]-=rhs[M];
	}

	static void eq_mul(T1* lhs,const T2* rhs)
	{
		bopx::eq_mul(lhs,rhs);
		lhs[M]*=rhs[M];
	}

	static void eq_div(T1* lhs,const T2* rhs)
	{
		bopx::eq_div(lhs,rhs);
		lhs[M]/=rhs[M];
	}

	static void eq_set(T1* lhs,const T2& rhs)
	{
		bopx::eq_set(lhs,rhs);
		lhs[M]=rhs;
	}

	static void eq_add(T1* lhs,const T2& rhs)
	{
		bopx::eq_add(lhs,rhs);
		lhs[M]+=rhs;
	}
	static void eq_sub(T1* lhs,const T2& rhs)
	{
		bopx::eq_sub(lhs,rhs);
		lhs[M]-=rhs;
	}

	static void eq_mul(T1* lhs,const T2& rhs)
	{
		bopx::eq_mul(lhs,rhs);
		lhs[M]*=rhs;
	}

	static void eq_div(T1* lhs,const T2& rhs)
	{
		bopx::eq_div(lhs,rhs);
		lhs[M]/=rhs;
	}

	static T3 dot(const T1* lhs,const T2* rhs)
	{
		return bopx::dot(lhs,rhs)+lhs[M]*rhs[M];
	}

	static void add(T3* res,const T1* lhs,const T2* rhs)
	{
		bopx::add(res,lhs,rhs);
		res[M]=lhs[M]+rhs[M];
	}

	static void sub(T3* res,const T1* lhs,const T2* rhs)
	{
		bopx::sub(res,lhs,rhs);
		res[M]=lhs[M]-rhs[M];
	}

	static void mul(T3* res,const T1* lhs,const T2* rhs)
	{
		bopx::mul(res,lhs,rhs);
		res[M]=lhs[M]*rhs[M];
	}

	static void div(T3* res,const T1* lhs,const T2* rhs)
	{
		bopx::div(res,lhs,rhs);
		res[M]=lhs[M]/rhs[M];
	}

	static void add(T3* res,const T1* lhs,const T2& rhs)
	{
		bopx::add(res,lhs,rhs);
		res[M]=lhs[M]+rhs;
	}

	static void sub(T3* res,const T1* lhs,const T2& rhs)
	{
		bopx::sub(res,lhs,rhs);
		res[M]=lhs[M]-rhs;
	}

	static void mul(T3* res,const T1* lhs,const T2& rhs)
	{
		bopx::mul(res,lhs,rhs);
		res[M]=lhs[M]*rhs;
	}

	static void div(T3* res,const T1* lhs,const T2& rhs)
	{
		bopx::div(res,lhs,rhs);
		res[M]=lhs[M]/rhs;
	}

	static void add(T3* res,const T1& lhs,const T2* rhs)
	{
		bopx::add(res,lhs,rhs);
		res[M]=lhs+rhs[M];
	}

	static void sub(T3* res,const T1& lhs,const T2* rhs)
	{
		bopx::sub(res,lhs,rhs);
		res[M]=lhs-rhs[M];
	}

	static void mul(T3* res,const T1& lhs,const T2* rhs)
	{
		bopx::mul(res,lhs,rhs);
		res[M]=lhs*rhs[M];
	}

	static void div(T3* res,const T1& lhs,const T2* rhs)
	{
		bopx::div(res,lhs,rhs);
		res[M]=lhs/rhs[M];
	}
};

template<typename T1,typename T2>
struct tiny_opx<T1,T2,1>
{
	static const int M=0;
	static const int N=1;

	typedef typename cpx_promote<T1,T2>::type T3;

	template<typename P>
	static void g(T1* lhs,const T2* rhs)
	{
		lhs[M]=P::g(rhs[M]);
	}

	template<typename P>
	static void k(T1* lhs,const T2& rhs)
	{
		lhs[M]=P::g(rhs);
	}
	static bool eq(const T1* lhs,const T2* rhs)
	{
		return lhs[M]==rhs[M];
	}

	static void eq_set(T1* lhs,const T2* rhs)
	{
		lhs[M]=rhs[M];
	}

	static void eq_neg(T1* lhs,const T2* rhs)
	{
		lhs[M]=-rhs[M];
	}

	static void eq_add(T1* lhs,const T2* rhs)
	{
		lhs[M]+=rhs[M];
	}
	static void eq_sub(T1* lhs,const T2* rhs)
	{
		lhs[M]-=rhs[M];
	}

	static void eq_mul(T1* lhs,const T2* rhs)
	{
		lhs[M]*=rhs[M];
	}

	static void eq_div(T1* lhs,const T2* rhs)
	{
		lhs[M]/=rhs[M];
	}

	static void eq_set(T1* lhs,const T2& rhs)
	{
		lhs[M]=rhs;
	}

	static void eq_add(T1* lhs,const T2& rhs)
	{
		lhs[M]+=rhs;
	}
	static void eq_sub(T1* lhs,const T2& rhs)
	{
		lhs[M]-=rhs;
	}

	static void eq_mul(T1* lhs,const T2& rhs)
	{
		lhs[M]*=rhs;
	}

	static void eq_div(T1* lhs,const T2& rhs)
	{
		lhs[M]/=rhs;
	}

	static T3 dot(const T1* lhs,const T2* rhs)
	{
		return lhs[M]*rhs[M];
	}

	static void add(T3* res,const T1* lhs,const T2* rhs)
	{
		res[M]=lhs[M]+rhs[M];
	}

	static void sub(T3* res,const T1* lhs,const T2* rhs)
	{
		res[M]=lhs[M]-rhs[M];
	}

	static void mul(T3* res,const T1* lhs,const T2* rhs)
	{
		res[M]=lhs[M]*rhs[M];
	}

	static void div(T3* res,const T1* lhs,const T2* rhs)
	{
		res[M]=lhs[M]/rhs[M];
	}

	static void add(T3* res,const T1* lhs,const T2& rhs)
	{
		res[M]=lhs[M]+rhs;
	}

	static void sub(T3* res,const T1* lhs,const T2& rhs)
	{
		res[M]=lhs[M]-rhs;
	}

	static void mul(T3* res,const T1* lhs,const T2& rhs)
	{
		res[M]=lhs[M]*rhs;
	}

	static void div(T3* res,const T1* lhs,const T2& rhs)
	{
		res[M]=lhs[M]/rhs;
	}

	static void add(T3* res,const T1& lhs,const T2* rhs)
	{
		res[M]=lhs+rhs[M];
	}

	static void sub(T3* res,const T1& lhs,const T2* rhs)
	{
		res[M]=lhs-rhs[M];
	}

	static void mul(T3* res,const T1& lhs,const T2* rhs)
	{
		res[M]=lhs*rhs[M];
	}

	static void div(T3* res,const T1& lhs,const T2* rhs)
	{
		res[M]=lhs/rhs[M];
	}
};


template<typename T,int R,int C>
class tiny_mat;

template<typename T,int N>
class tiny_vec
{
public:

	typedef tiny_storage<T,N> storage_type;

	tiny_vec(const storage_type& o):storage(o){}
	tiny_vec(){tiny_opx<T,T,N>::eq_set(data(),T());}

	template<typename Y>
	tiny_vec(const tiny_vec<Y,N>& o)
	{
		tiny_opx<T,Y,N>::eq_set(data(),o.data());
	}

	tiny_vec(const T& v1,const T& v2)
	{
		(*this)[0]=v1;
		(*this)[1]=v2;
		if (N > 2) (*this)[2] = T();
		if (N > 3) (*this)[3] = T();
	}

	tiny_vec(const T& v1,const T& v2,const T& v3)
	{
		(*this)[0]=v1;
		(*this)[1]=v2;
		if(N>2) (*this)[2]=v3;
	}

	tiny_vec(const T& v1,const T& v2,const T& v3,const T& v4)
	{
		(*this)[0]=v1;
		(*this)[1]=v2;
		if(N>2) (*this)[2]=v3;
		if(N>3) (*this)[3]=v4;
	}


	const tiny_mat<T,N,1>& as_row() const {return *(const tiny_mat<T,N,1>*)this;}
	tiny_mat<T,N,1>& as_row() {return *(tiny_mat<T,N,1>*)this;}

	const tiny_mat<T,1,N>& as_col() const {return *(const tiny_mat<T,1,N>*)this;}
	tiny_mat<T,1,N>& as_col() {return *(tiny_mat<T,1,N>*)this;}

	template<typename T2>
	tiny_vec& operator= (const tiny_vec<T2,N>& o){tiny_opx<T,T2,N>::eq_set(data(),o.data());return *this;}

	template<typename T2>
	tiny_vec& operator+=(const tiny_vec<T2,N>& o){tiny_opx<T,T2,N>::eq_add(data(),o.data());return *this;}

	template<typename T2>
	tiny_vec& operator-=(const tiny_vec<T2,N>& o){tiny_opx<T,T2,N>::eq_sub(data(),o.data());return *this;}

	template<typename T2>
	tiny_vec& operator*=(const tiny_vec<T2,N>& o){tiny_opx<T,T2,N>::eq_mul(data(),o.data());return *this;}

	template<typename T2>
	tiny_vec& operator/=(const tiny_vec<T2,N>& o){tiny_opx<T,T2,N>::eq_div(data(),o.data());return *this;}

	template<typename T2>
	tiny_vec& operator= (const T2& o){tiny_opx<T,T2,N>::eq_set(data(),o);return *this;}

	template<typename T2>
	tiny_vec& operator+=(const T2& o){tiny_opx<T,T2,N>::eq_add(data(),o);return *this;}

	template<typename T2>
	tiny_vec& operator-=(const T2& o){tiny_opx<T,T2,N>::eq_sub(data(),o);return *this;}

	template<typename T2>
	tiny_vec& operator*=(const T2& o){tiny_opx<T,T2,N>::eq_mul(data(),o);return *this;}

	const tiny_vec& operator+(void) const{return *this;}

	tiny_vec operator-(void) const
	{
		tiny_vec tmp(*this);tmp*=-1.0;return tmp;
	}

	template<typename T2>
	tiny_vec& operator/=(const T2& o){tiny_opx<T,T2,N>::eq_div(data(),o);return *this;}


	void seta(const T& o){tiny_opx<T,T,N>::eq_set(data(),o);}

	bool operator==(const tiny_vec& o) const {return tiny_opx<T,T,N>::eq(data(),o.data());}
	bool operator!=(const tiny_vec& o) const {return !operator==(o);}

	T& operator[](size_t n){return storage[n];}
	const T& operator[](size_t n) const {return storage[n];}

	const T* data() const{return storage.data();}
	T* data(){return storage.data();}

	double length() const;

	void set2(T i,T j)
	{
		(*this)[0]=i;
		(*this)[1]=j;
	}

	void set3(T i,T j,T k)
	{
		(*this)[0]=i;
		(*this)[1]=j;
		if(N>2) (*this)[2]=k;
	}

	void set4(T i,T j,T k,T w)
	{
		(*this)[0]=i;
		(*this)[1]=j;
		if(N>2) (*this)[2]=k;
		if(N>3) (*this)[3]=w;
	}

protected:
	storage_type storage;
};


template<typename T, int N> class hash_t<tiny_vec<T, N> > : public hash_pod<tiny_vec<T, N> >{};


template<typename H,int N, int D>
struct vec_promote_real : public H {};

template<typename H,int D>
struct vec_promote_real<H,1,D> : public H
{
	typedef typename H::type scalar;
	typedef typename tiny_vec<typename H::type,D> promoted;
	typedef scalar type;
};

template<typename H,int D>
struct vec_promote_real<H,2,D> : public H
{
	typedef typename H::type scalar;
	typedef typename tiny_vec<typename H::type,D> promoted;
	typedef promoted type;
};

template<typename X, typename Y>
struct vec_promote : public vec_promote_real<cpx_promote<X, Y>, cpx_promote<X, Y>::value ? 1 : 0,3>{};

template<typename X, typename Y,int D>
struct vec_promote<tiny_vec<X,D>, Y > : public vec_promote_real<cpx_promote<X, Y>, cpx_promote<X, Y>::value ? 2 : 0,D>{};

template<typename X, typename Y,int D>
struct vec_promote<X, tiny_vec<Y,D> > : public vec_promote_real<cpx_promote<X, Y>, cpx_promote<X, Y>::value ? 2 : 0,D>{};

template<typename X, typename Y,int D>
struct vec_promote<tiny_vec<X,D>, tiny_vec<Y,D> > : public vec_promote_real<cpx_promote<X, Y>, cpx_promote<X, Y>::value ? 2 : 0,D>{};


template<typename X, typename Y,int D>
struct vec_promote_n : public vec_promote_real<cpx_promote<X, Y>, cpx_promote<X, Y>::value?2:0, D>{};



template<typename X,typename Y,int N>
inline typename vec_promote_n<X,Y,N>::promoted operator + (const tiny_vec<X,N> &lhs,const Y rhs)
{
	typename vec_promote_n<X,Y,N>::promoted::storage_type res;
	tiny_opx<X,Y,N>::add(res.data(),lhs.data(),rhs);
	return res;
}

template<typename X,typename Y,int N>
inline typename vec_promote_n<X,Y,N>::promoted operator - (const tiny_vec<X,N> &lhs,const Y rhs)
{
	typename vec_promote_n<X,Y,N>::promoted::storage_type res;
	tiny_opx<X,Y,N>::sub(res.data(),lhs.data(),rhs);
	return res;
}

template<typename X,typename Y,int N>
inline typename vec_promote_n<X,Y,N>::promoted operator * (const tiny_vec<X,N> &lhs,const Y rhs)
{
	typename vec_promote_n<X,Y,N>::promoted::storage_type res;
	tiny_opx<X,Y,N>::mul(res.data(),lhs.data(),rhs);
	return res;
}

template<typename X,typename Y,int N>
inline typename vec_promote_n<X,Y,N>::promoted operator / (const tiny_vec<X,N> &lhs,const Y rhs)
{
	typename vec_promote_n<X,Y,N>::promoted::storage_type res;
	tiny_opx<X,Y,N>::div(res.data(),lhs.data(),rhs);
	return res;
}

template<typename X,typename Y,int N>
inline typename vec_promote_n<X,Y,N>::promoted operator + (const X& lhs,const tiny_vec<Y,N>& rhs)
{
	typename vec_promote_n<X,Y,N>::promoted::storage_type res;
	tiny_opx<X,Y,N>::add(res.data(),lhs,rhs.data());
	return res;
}

template<typename X,typename Y,int N>
inline typename vec_promote_n<X,Y,N>::promoted operator - (const X& lhs,const tiny_vec<Y,N>& rhs)
{
	typename vec_promote_n<X,Y,N>::promoted::storage_type res;
	tiny_opx<X,Y,N>::sub(res.data(),lhs,rhs.data());
	return res;
}

template<typename X,typename Y,int N>
inline typename vec_promote_n<X,Y,N>::promoted operator * (const X& lhs,const tiny_vec<Y,N>& rhs)
{
	typename vec_promote_n<X,Y,N>::promoted::storage_type res;
	tiny_opx<X,Y,N>::mul(res.data(),lhs,rhs.data());
	return res;
}

template<typename X,typename Y,int N>
inline typename vec_promote_n<X,Y,N>::promoted operator / (const X& lhs,const tiny_vec<Y,N>& rhs)
{
	typename vec_promote_n<X,Y,N>::promoted::storage_type res;
	tiny_opx<X,Y,N>::div(res.data(),lhs,rhs.data());
	return res;
}

template<typename X,typename Y,int N>
inline typename vec_promote_n<X,Y,N>::promoted operator + (const tiny_vec<X,N> &lhs,const tiny_vec<Y,N>& rhs)
{
	typename vec_promote_n<X,Y,N>::promoted::storage_type res;
	tiny_opx<X,Y,N>::add(res.data(),lhs.data(),rhs.data());
	return res;
}

template<typename X,typename Y,int N>
inline typename vec_promote_n<X,Y,N>::promoted operator - (const tiny_vec<X,N> &lhs,const tiny_vec<Y,N>& rhs)
{
	typename vec_promote_n<X,Y,N>::promoted::storage_type res;
	tiny_opx<X,Y,N>::sub(res.data(),lhs.data(),rhs.data());
	return res;
}

template<typename X,typename Y,int N>
inline typename vec_promote_n<X,Y,N>::promoted operator * (const tiny_vec<X,N> &lhs,const tiny_vec<Y,N>& rhs)
{
	typename vec_promote_n<X,Y,N>::promoted::storage_type res;
	tiny_opx<X,Y,N>::mul(res.data(),lhs.data(),rhs.data());
	return res;
}

template<typename X,typename Y,int N>
inline typename vec_promote_n<X,Y,N>::promoted operator / (const tiny_vec<X,N> &lhs,const tiny_vec<Y,N>& rhs)
{
	typename vec_promote_n<X,Y,N>::promoted::storage_type res;
	tiny_opx<X,Y,N>::div(res.data(),lhs.data(),rhs.data());
	return res;
}

template<typename X,typename Y>
inline typename vec_promote_n<X,Y,3>::promoted cross (const tiny_vec<X,3> &lhs,const tiny_vec<Y,3>& rhs)
{
	typename vec_promote_n<X,Y,3>::promoted::storage_type res;
	res[0]=lhs[1]*rhs[2]-lhs[2]*rhs[1];
	res[1]=lhs[2]*rhs[0]-lhs[0]*rhs[2];
	res[2]=lhs[0]*rhs[1]-lhs[1]*rhs[0];
	return res;
}

//template <typename X, typename Y>
//inline typename vec_promote_n<X, Y, 3>::promoted cross(const tiny_vec<X, 2>& lhs, const tiny_vec<Y, 2>& rhs)
//{
//	typename vec_promote_n<X, Y, 3>::promoted::storage_type res;
//	res[0] = 0;
//	res[1] = 0;
//	res[2] = lhs[0] * rhs[1] - lhs[1] * rhs[0];
//	return res;
//}

template <typename X, typename Y>
inline typename cpx_promote<X, Y>::type cross(const tiny_vec<X, 2>& lhs, const tiny_vec<Y, 2>& rhs)
{
	return lhs[0] * rhs[1] - lhs[1] * rhs[0];
}

template<typename X,typename Y,int N>
inline typename vec_promote_n<X,Y,N>::scalar dot (const tiny_vec<X,N> &lhs,const tiny_vec<Y,N> rhs)
{
	return tiny_opx<X,Y,N>::dot(lhs.data(),rhs.data());
}

template<typename T, int N>
inline tiny_vec<tiny_cpx<T>, N> conj(const tiny_vec<tiny_cpx<T>, N>& v)
{
	tiny_vec<tiny_cpx<T>, N> vec;
	for (int i = 0; i < N; i++) { vec[i] = conj(v[i]); }
	return vec;
}

template<typename T,int N>
double tiny_vec<T,N>::length() const
{
	return ::sqrt(tiny_opx<T,T,N>::dot(data(),data()));
}


template<typename T>
void v3_cshift(tiny_vec<T,3>& v,int d)
{
	switch(d%3)
	{
	case 1:
	case -2:
		{
			T tmp(v[0]);
			v[0]=v[2];
			v[2]=v[1];
			v[1]=tmp;
		}
		break;
	case 2:
	case -1:
		{
			T tmp(v[0]);
			v[0]=v[1];
			v[1]=v[2];
			v[2]=tmp;
		}
		break;
	}
}

EW_LEAVE


EW_ENTER

// 2017.06.22 Dai.Weifeng
// make_vector
template<typename T>
tiny_vec<T, 1> make_vec(T val)
{
	tiny_vec<T, 1> v; v[0] = val;
	return v;
}

template<typename T>
tiny_vec<T, 2> make_vec(T val0, T val1)
{
	tiny_vec<T, 2> v; v[0] = val0; v[1] = val1;
	return v;
}

template<typename T>
tiny_vec<T, 3> make_vec(T val0, T val1, T val2)
{
	tiny_vec<T, 3> v; v[0] = val0; v[1] = val1; v[2] = val2;
	return v;
}

template<typename T>
tiny_vec<T, 4> make_vec(T val0, T val1, T val2, T val3)
{
	tiny_vec<T, 4> v; v[0] = val0; v[1] = val1; v[2] = val2; v[3] = val3;
	return v;
}

template<typename T>
tiny_vec<T, 5> make_vec(T val0, T val1, T val2, T val3, T val4)
{
	tiny_vec<T, 5> v; v[0] = val0; v[1] = val1; v[2] = val2; v[3] = val3; v[4] = val4;
	return v;
}

template<typename T>
tiny_vec<T, 6> make_vec(T val0, T val1, T val2, T val3, T val4, T val5)
{
	tiny_vec<T, 6> v; v[0] = val0; v[1] = val1; v[2] = val2; v[3] = val3; v[4] = val4; v[5] = val5;
	return v;
}

EW_LEAVE

namespace tl
{
	template<typename T,int N> struct is_pod<ew::tiny_vec<T,N> > : public value_type<true>{};
};


#endif
