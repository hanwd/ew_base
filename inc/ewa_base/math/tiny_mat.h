#ifndef __H_EW_MATH_TINY_MAT__
#define __H_EW_MATH_TINY_MAT__

#include "ewa_base/math/tiny_vec.h"
#include "ewa_base/math/math_def.h"

EW_ENTER

template<typename T,int R,int C>
class tiny_mat
{
public:

	static const int N=R*C;
	typedef tiny_storage2<T, R,C> storage_type;

	tiny_mat(const storage_type& o):storage(o){}
	tiny_mat(){tiny_opx<T,T,N>::eq_set(storage.data(),T());}


	template<typename T2>
	tiny_mat& operator= (const tiny_mat<T2,R,C>& o){tiny_opx<T,T2,N>::eq_set(data(),o.data());return *this;}

	template<typename T2>
	tiny_mat& operator+=(const tiny_mat<T2,R,C>& o){tiny_opx<T,T2,N>::eq_add(data(),o.data());return *this;}

	template<typename T2>
	tiny_mat& operator-=(const tiny_mat<T2,R,C>& o){tiny_opx<T,T2,N>::eq_sub(data(),o.data());return *this;}

	template<typename T2>
	tiny_mat& operator= (const T2& o){tiny_opx<T,T2,N>::eq_set(data(),o);return *this;}

	template<typename T2>
	tiny_mat& operator+=(const T2& o){tiny_opx<T,T2,N>::eq_add(data(),o);return *this;}

	template<typename T2>
	tiny_mat& operator-=(const T2& o){tiny_opx<T,T2,N>::eq_sub(data(),o);return *this;}

	template<typename T2>
	tiny_mat& operator*=(const T2& o){tiny_opx<T,T2,N>::eq_mul(data(),o);return *this;}

	template<typename T2>
	tiny_mat& operator/=(const T2& o){tiny_opx<T,T2,N>::eq_div(data(),o);return *this;}

	bool operator==(const tiny_mat& o) const {return tiny_opx<T,T,N>::eq(data(),o.data());}
	bool operator!=(const tiny_mat& o) const {return !operator==(o);}


	T& operator()(int i,int j){return storage[i+j*R];}
	const T& operator()(int i,int j) const {return storage[i+j*R];}

	T* data(){return storage.data();}
	const T* data() const {return storage.data();}

private:
	storage_type storage;
};


template<typename T,int R,int C>
struct opx_scalar_mat_n : public tl::value_type<false>
{
	typedef T type;
	typedef tiny_mat<T,R,C> promoted;
};

template<typename T,int R,int C>
struct opx_scalar_mat_n<tiny_mat<T,R,C>,R,C> : public tl::value_type<true>
{
	typedef T type;
};

template<int R,int C>
struct opx_scalar_mat
{
	template<typename T>
	struct rebind : public opx_scalar_mat_n<T,R,C>{};
};

template<typename X,typename Y,int R,int C>
struct mat_promote : public opx_helper_promote<X,Y,cpx_promote,opx_scalar_mat<R,C>::template rebind>{};

template<typename T,int R,int C> class hash_t<tiny_mat<T,R,C> > : public hash_pod<tiny_mat<T,R,C> > {};

template<typename T,int L>
tiny_mat<T,L,L> diag(const tiny_vec<T,L>& v)
{
	tiny_mat<T,L,L> res;
	for(int i=0;i<L;i++) res(i,i)=v[i];
	return res;
}

template<typename T,int R,int C>
tiny_mat<T,C,R> tran(const tiny_mat<T,R,C>& v)
{
	tiny_mat<T,C,R> res;
	for(int i=0;i<R;i++) for(int j=0;j<C;j++) res(j,i)=v(i,j);
	return res;
}

template<typename T>
tiny_mat<T,4,4> m4_inverse(const tiny_mat<T,4,4>& m4)
{
	tiny_mat<T,4,4> res;

	typedef typename cpx_promote<T,double>::scalar scalar_type;
	scalar_type a1,a2,a3,a4;
	scalar_type b1,b2,b3,b4;
	scalar_type c1,c2,c3,c4;
	scalar_type d1,d2,d3,d4;

	a1=m4(0,0);
	a2=m4(1,0);
	a3=m4(2,0);
	a4=m4(3,0);
	b1=m4(0,1);
	b2=m4(1,1);
	b3=m4(2,1);
	b4=m4(3,1);
	c1=m4(0,2);
	c2=m4(1,2);
	c3=m4(2,2);
	c4=m4(3,2);
	d1=m4(0,3);
	d2=m4(1,3);
	d3=m4(2,3);
	d4=m4(3,3);

	scalar_type kk=
		-a1*b2*d3*c4+a1*b2*d4*c3-a1*d4*c2*b3-a1*d2*b4*c3
		+a1*d3*c2*b4+a1*d2*b3*c4+b2*d1*a3*c4-b2*d4*c1*a3
		-b2*d1*a4*c3+b2*d3*c1*a4-d1*a2*b3*c4+d1*a4*c2*b3
		+d2*b4*c1*a3+d2*b1*a4*c3+d3*c4*b1*a2-d3*c2*b1*a4
		-d3*c1*a2*b4-d1*a3*c2*b4-d2*b3*c1*a4-d2*b1*a3*c4
		-d4*c3*b1*a2+d4*c2*b1*a3+d4*c1*a2*b3+d1*a2*b4*c3;

	scalar_type ki=1.0/kk;

	T* pd=res.data();

	pd[ 0]= (-b2*d3*c4+b2*d4*c3-d4*c2*b3-d2*b4*c3+d3*c2*b4+d2*b3*c4)*ki;
	pd[ 1]=-(-d2*a4*c3-d3*c4*a2+d3*c2*a4-d4*c2*a3+d2*a3*c4+d4*c3*a2)*ki;
	pd[ 2]= ( d4*a2*b3-d3*a2*b4-d2*b3*a4-d4*b2*a3+d2*b4*a3+d3*b2*a4)*ki;
	pd[ 3]=-(-a4*c2*b3-a2*b4*c3-b2*a3*c4+a3*c2*b4+b2*a4*c3+a2*b3*c4)*ki;

	pd[ 4]=-(-d3*c4*b1+d4*c3*b1+d1*b3*c4-d4*c1*b3-d1*b4*c3+d3*c1*b4)*ki;
	pd[ 5]= (-a1*d3*c4+a1*d4*c3+d1*a3*c4-d4*c1*a3-d1*a4*c3+d3*c1*a4)*ki;
	pd[ 6]=-(-a1*d3*b4+a1*d4*b3+d1*a3*b4-d1*a4*b3+d3*b1*a4-d4*b1*a3)*ki;
	pd[ 7]= ( a1*b3*c4-a1*b4*c3+b4*c1*a3-b3*c1*a4-b1*a3*c4+b1*a4*c3)*ki;

	pd[ 8]= ( b2*d1*c4-b2*d4*c1+d2*b4*c1-d1*c2*b4-d2*b1*c4+d4*c2*b1)*ki;
	pd[ 9]=-( a1*d4*c2-a1*d2*c4-d4*c1*a2-d1*a4*c2+d2*c1*a4+d1*a2*c4)*ki;
	pd[10]= ( a1*b2*d4-a1*d2*b4-b2*d1*a4+d2*b1*a4-d4*b1*a2+d1*a2*b4)*ki;
	pd[11]=-( c4*a1*b2-c4*b1*a2-c1*a4*b2-c2*a1*b4+c2*b1*a4+c1*a2*b4)*ki;

	pd[12]=-( b2*d1*c3-b2*d3*c1-d1*c2*b3+d3*c2*b1+d2*b3*c1-d2*b1*c3)*ki;
	pd[13]= (-a1*d2*c3+a1*d3*c2+d2*c1*a3-d3*c1*a2-d1*a3*c2+d1*a2*c3)*ki;
	pd[14]=-( d3*a1*b2-d3*b1*a2-d1*a3*b2-d2*a1*b3+d2*b1*a3+d1*a2*b3)*ki;
	pd[15]= ( c3*a1*b2-c3*b1*a2-c1*a3*b2-c2*a1*b3+c2*b1*a3+c1*a2*b3)*ki;

	return res;
}

template <typename T>
T det(const tiny_mat<T, 2, 2>& m2)
{
	return m2(0, 0) * m2(1, 1) - m2(0, 1) * m2(1, 0);
}

template <typename T>
T det(const tiny_mat<T, 3, 3>& m3)
{
	return m3(0, 0) * m3(1, 1) * m3(2, 2)
		+ m3(0, 1) * m3(1, 2) * m3(2, 0)
		+ m3(0, 2) * m3(1, 0) * m3(2, 1)
		- m3(0, 0) * m3(1, 2) * m3(2, 1)
		- m3(0, 1) * m2(1, 0) * m3(2, 2)
		- m3(0, 2) * m2(1, 1) * m3(2, 0);
}

template<typename X,typename Y,int R,int C,int L>
typename mat_promote<X,Y,R,C>::promoted operator*(const tiny_mat<X,R,L>& lhs,const tiny_mat<Y,L,C>& rhs)
{
	typename mat_promote<X,Y,R,C>::promoted res;
	for(int i=0;i<R;i++)
	{
		for(int j=0;j<C;j++)
		{
			for(int k=0;k<L;k++)
			{
				res(i,j)+=lhs(i,k)*rhs(k,j);
			}
		}
	}
	return res;
}


template<typename X,typename Y,int N>
typename vec_promote<X,Y,N>::promoted operator*(const tiny_mat<X,N,N>& lhs,const tiny_vec<Y,N>& rhs)
{
	typename vec_promote<X,Y,N>::promoted res;
	for(int i=0;i<N;i++)
	{
		for(int k=0;k<N;k++)
		{
			res[N]+=lhs(i,k)*rhs[k];
		}
	}
	return res;
}


template<typename X,typename Y>
typename vec_promote<X,Y,3>::promoted operator*(const tiny_mat<X,4,4>& lhs,const tiny_vec<Y,3>& rhs)
{
	typename vec_promote<X,Y,3>::promoted res;
	res[0]=lhs(0,0)*rhs[0]+lhs(0,1)*rhs[1]+lhs(0,2)*rhs[2]+lhs(0,3);
	res[1]=lhs(1,0)*rhs[0]+lhs(1,1)*rhs[1]+lhs(1,2)*rhs[2]+lhs(1,3);
	res[2]=lhs(2,0)*rhs[0]+lhs(2,1)*rhs[1]+lhs(2,2)*rhs[2]+lhs(2,3);
	return res;
}


template<typename X,typename Y,int R,int C>
typename mat_promote<X,Y,R,C>::promoted operator+(const tiny_mat<X,R,C>& lhs,const tiny_mat<Y,R,C>& rhs)
{
	typename mat_promote<X,Y,R,C>::promoted::storage_type res;
	tiny_opx<X,Y,R*C>::eq_add(res.data(),lhs.data(),rhs.data());
	return res;
}

template<typename X,typename Y,int R,int C>
typename mat_promote<X,Y,R,C>::promoted operator-(const tiny_mat<X,R,C>& lhs,const tiny_mat<Y,R,C>& rhs)
{
	typename mat_promote<X,Y,R,C>::promoted::storage_type res;
	tiny_opx<X,Y,R*C>::eq_sub(res.data(),lhs.data(),rhs.data());
	return res;
}

template<typename X,typename Y,int R,int C>
typename mat_promote<X,Y,R,C>::promoted operator+(const tiny_mat<X,R,C>& lhs,const Y& rhs)
{
	typename mat_promote<X,Y,R,C>::promoted::storage_type res;
	tiny_opx<X,Y,R*C>::eq_add(res.data(),lhs.data(),rhs);
	return res;
}

template<typename X,typename Y,int R,int C>
typename mat_promote<X,Y,R,C>::promoted operator-(const tiny_mat<X,R,C>& lhs,const Y& rhs)
{
	typename mat_promote<X,Y,R,C>::promoted::storage_type res;
	tiny_opx<X,Y,R*C>::eq_sub(res.data(),lhs.data(),rhs);
	return res;
}

template<typename X,typename Y,int R,int C>
typename mat_promote<X,Y,R,C>::promoted operator*(const tiny_mat<X,R,C>& lhs,const Y& rhs)
{
	typename mat_promote<X,Y,R,C>::promoted::storage_type res;
	tiny_opx<X,Y,R*C>::eq_mul(res.data(),lhs.data(),rhs);
	return res;
}

template<typename X,typename Y,int R,int C>
typename mat_promote<X,Y,R,C>::promoted operator/(const tiny_mat<X,R,C>& lhs,const Y& rhs)
{
	typename mat_promote<X,Y,R,C>::promoted::storage_type res;
	tiny_opx<X,Y,R*C>::eq_div(res.data(),lhs.data(),rhs);
	return res;
}

template<typename X,typename Y,int R,int C>
typename mat_promote<X,Y,R,C>::promoted operator+(const X& lhs,const tiny_mat<Y,R,C>& rhs)
{
	typename mat_promote<X,Y,R,C>::promoted::storage_type res;
	tiny_opx<X,Y,R*C>::eq_add(res.data(),lhs,rhs.data());
	return res;
}

template<typename X,typename Y,int R,int C>
typename mat_promote<X,Y,R,C>::promoted operator-(const X& lhs,const tiny_mat<Y,R,C>& rhs)
{
	typename mat_promote<X,Y,R,C>::promoted::storage_type res;
	tiny_opx<X,Y,R*C>::eq_sub(res.data(),lhs,rhs.data());
	return res;
}

template<typename X,typename Y,int R,int C>
typename mat_promote<X,Y,R,C>::promoted operator*(const X& lhs,const tiny_mat<Y,R,C>& rhs)
{
	typename mat_promote<X,Y,R,C>::promoted::storage_type res;
	tiny_opx<X,Y,R*C>::eq_mul(res.data(),lhs,rhs.data());
	return res;
}

template<typename X,typename Y,int R,int C>
typename mat_promote<X,Y,R,C>::promoted operator/(const X& lhs,const tiny_mat<Y,R,C>& rhs)
{
	typename mat_promote<X,Y,R,C>::promoted::storage_type res;
	tiny_opx<X,Y,R*C>::eq_div(res.data(),lhs,rhs.data());
	return res;
}


template<typename T,int R,int C>
class type_mat : public tiny_mat<T,R,C>
{
public:
	typedef tiny_mat<T,R,C> basetype;
	type_mat(){}
	type_mat(const basetype& o):basetype(o){}
};


template<typename X>
class type_mat<X,4,4> : public tiny_mat<X,4,4>
{
public:
	typedef tiny_mat<X,4,4> basetype;
	typedef tiny_vec<X,3> vec3;

	type_mat(){LoadIdentity();}
	type_mat(const basetype& o):basetype(o){}

	bool IsIdentity() const
	{
		static mat4d id;
		return *this==id;
	}

	void LoadIdentity()
	{
		basetype& m4(*this);
		tiny_vec<double,4> v4;
		v4[0]=v4[1]=v4[2]=v4[3]=1.0;
		m4=diag(v4);
	}

	template<typename Y>
	void Translate(const tiny_vec<Y,3>& v)
	{
		basetype& m4(*this);
		m4(0,3)+=m4(0,0)*v[0]+m4(0,1)*v[1]+m4(0,2)*v[2];
		m4(1,3)+=m4(1,0)*v[0]+m4(1,1)*v[1]+m4(1,2)*v[2];
		m4(2,3)+=m4(2,0)*v[0]+m4(2,1)*v[1]+m4(2,2)*v[2];
	}

	template<typename Y>
	void Scale(const tiny_vec<Y,3>& v)
	{
		basetype& m4(*this);
		for(size_t i=0;i<4;i++)
		{
			m4(i,0)*=v[0];
			m4(i,1)*=v[1];
			m4(i,2)*=v[2];
		}
	}

	void Scale(X v)
	{
		basetype& m4(*this);
		for(size_t i=0;i<4;i++)
		{
			m4(i,0)*=v;
			m4(i,1)*=v;
			m4(i,2)*=v;
		}	
	}

	template<typename Y>
	void Rotate(double deg,const tiny_vec<Y,3>& dir)
	{
		if(deg==0.0) return;
		double L=dir.length();
		if(L==0.0) return;

		basetype& m4(*this);
		basetype  _m;

		double C=cosdeg(deg);
		double S=sindeg(deg);
		double T=1.0-C;

		_m(0,0)=C+dir[0]*dir[0]*T;
		_m(0,1)=dir[0]*dir[1]*T-dir[2]*S;
		_m(0,2)=dir[0]*dir[2]*T+dir[1]*S;
		_m(0,3)=0.0;
		_m(1,0)=dir[0]*dir[1]*T+dir[2]*S;
		_m(1,1)=C+dir[1]*dir[1]*T;
		_m(1,2)=dir[1]*dir[2]*T-dir[0]*S;
		_m(1,3)=0.0;
		_m(2,0)=dir[0]*dir[2]*T-dir[1]*S;
		_m(2,1)=dir[1]*dir[2]*T+dir[0]*S;
		_m(2,2)=C+dir[2]*dir[2]*T;
		_m(2,3)=0.0;
		_m(3,0)=_m(3,1)=_m(3,2)=0.0;
		_m(3,3)=1.0;

		m4=m4*_m;
	}

	void Inverse()
	{
		(*this)=m4_inverse(*this);
	}

	void MultMatrix(const type_mat& m)
	{
		basetype& m4(*this);
		m4=m4*m;
	}

	void MultMatrix(type_mat m,double u)
	{
		m(0,3)*=u;
		m(1,3)*=u;
		m(2,3)*=u;
		MultMatrix(m);
	}

};


EW_LEAVE


namespace tl
{
	template<typename T,int R,int C> struct is_pod<ew::tiny_mat<T,R,C> > : public value_type<true>{};
};


#endif
