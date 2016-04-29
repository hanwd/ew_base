// Copyright 2014, Wenda han.  All rights reserved.
// https://github.com/vhwd/ew6_base
//
/// Use of this source code is governed by Apache License
// that can be found in the License file.
// Author: Wenda Han.

#include "ewa_base/config.h"

EW_ENTER

typedef tl::nulltype nil_t;

template<typename T>
class functor_reference
{
	const functor_reference& operator=(const functor_reference&);
public:
	T& ref;
	functor_reference(T& f):ref(f) {}
	inline operator T&()
	{
		return ref;
	}
};

template<typename T>
class functor_pointer : public FakePtrT<T>
{
public:
	functor_pointer(T* p=NULL):FakePtrT<T>(p) {}

	inline operator T*()
	{
		return this->get();
	}
	inline operator const T*() const
	{
		return this->get();
	}
};

template<typename T>
inline functor_reference<T> mk_ref(T& v)
{
	return functor_reference<T>(v);
}

template<typename T>
inline functor_reference<const T> mk_cref(const T& v)
{
	return functor_reference<const T>(v);
}

template<typename T>
class ParamRealImpl
{
public:
	typedef T type;
};

template<typename T>
class ParamRealImpl<T*>
{
public:
	typedef functor_pointer<T> type;
};

template<typename T>
class ParamRealImpl<functor_reference<T> >
{
public:
	typedef T& type;
};

template<typename T>
class ParamRealImpl<functor_reference<const T> >
{
public:
	typedef const T& type;
};

#define BINDIMPL_EXPAND_TUPLE(T,K)\
	typedef typename tl::tuple_at<T,0>::type K##0;\
	typedef typename tl::tuple_at<T,1>::type K##1;\
	typedef typename tl::tuple_at<T,2>::type K##2;\
	typedef typename tl::tuple_at<T,3>::type K##3;\
	typedef typename tl::tuple_at<T,4>::type K##4;\
	typedef typename tl::tuple_at<T,5>::type K##5;\
	typedef typename tl::tuple_at<T,6>::type K##6;\
	typedef typename tl::tuple_at<T,7>::type K##7;\
	typedef typename tl::tuple_at<T,8>::type K##8;\
	typedef typename tl::tuple_at<T,9>::type K##9;

#define BINDIMPL_UNUSED_PARAM() \
		EW_UNUSED(k1);EW_UNUSED(k2);EW_UNUSED(k3);EW_UNUSED(k4);\
		EW_UNUSED(k5);EW_UNUSED(k6);EW_UNUSED(k7);EW_UNUSED(k8);


class BindImplBase
{
public:

};

template<typename X0>
class FunctorImpl : public BindImplBase
{
public:
	
};

template<typename Rt>
class FunctorImpl<Rt(*)()> : public BindImplBase
{
public:
	typedef Rt return_type;
	typedef Rt(*Ft)();

	typedef tl::tuple<Ft> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return ft();
	}
};

template<typename Rt,typename Z1>
class FunctorImpl<Rt(*)(Z1)> : public BindImplBase
{
public:
	typedef Rt return_type;
	typedef Rt(*Ft)(Z1);

	typedef tl::tuple<Ft,Z1> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return ft(k1);
	}
};

template<typename Rt,typename Sd>
class FunctorImpl<Rt(Sd::*)()> : public BindImplBase
{
public:
	typedef Rt return_type;
	typedef Rt(Sd::*Ft)();

	typedef tl::tuple<Ft,Sd*> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return (k1->*ft)();
	}
};

template<typename Rt,typename Z1,typename Z2>
class FunctorImpl<Rt(*)(Z1,Z2)> : public BindImplBase
{
public:
	typedef Rt return_type;
	typedef Rt(*Ft)(Z1,Z2);

	typedef tl::tuple<Ft,Z1,Z2> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return ft(k1,k2);
	}
};

template<typename Rt,typename Sd,typename Z2>
class FunctorImpl<Rt(Sd::*)(Z2)> : public BindImplBase
{
public:
	typedef Rt return_type;
	typedef Rt(Sd::*Ft)(Z2);

	typedef tl::tuple<Ft,Sd*,Z2> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return (k1->*ft)(k2);
	}
};

template<typename Rt,typename Z1,typename Z2,typename Z3>
class FunctorImpl<Rt(*)(Z1,Z2,Z3)> : public BindImplBase
{
public:
	typedef Rt return_type;
	typedef Rt(*Ft)(Z1,Z2,Z3);

	typedef tl::tuple<Ft,Z1,Z2,Z3> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return ft(k1,k2,k3);
	}
};

template<typename Rt,typename Sd,typename Z2,typename Z3>
class FunctorImpl<Rt(Sd::*)(Z2,Z3)> : public BindImplBase
{
public:
	typedef Rt return_type;
	typedef Rt(Sd::*Ft)(Z2,Z3);

	typedef tl::tuple<Ft,Sd*,Z2,Z3> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return (k1->*ft)(k2,k3);
	}
};

template<typename Rt,typename Z1,typename Z2,typename Z3,typename Z4>
class FunctorImpl<Rt(*)(Z1,Z2,Z3,Z4)> : public BindImplBase
{
public:
	typedef Rt return_type;
	typedef Rt(*Ft)(Z1,Z2,Z3,Z4);

	typedef tl::tuple<Ft,Z1,Z2,Z3,Z4> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return ft(k1,k2,k3,k4);
	}
};

template<typename Rt,typename Sd,typename Z2,typename Z3,typename Z4>
class FunctorImpl<Rt(Sd::*)(Z2,Z3,Z4)> : public BindImplBase
{
public:
	typedef Rt return_type;
	typedef Rt(Sd::*Ft)(Z2,Z3,Z4);

	typedef tl::tuple<Ft,Sd*,Z2,Z3,Z4> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return (k1->*ft)(k2,k3,k4);
	}
};

template<typename Rt,typename Z1,typename Z2,typename Z3,typename Z4,typename Z5>
class FunctorImpl<Rt(*)(Z1,Z2,Z3,Z4,Z5)> : public BindImplBase
{
public:
	typedef Rt return_type;
	typedef Rt(*Ft)(Z1,Z2,Z3,Z4,Z5);

	typedef tl::tuple<Ft,Z1,Z2,Z3,Z4,Z5> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return ft(k1,k2,k3,k4,k5);
	}
};

template<typename Rt,typename Sd,typename Z2,typename Z3,typename Z4,typename Z5>
class FunctorImpl<Rt(Sd::*)(Z2,Z3,Z4,Z5)> : public BindImplBase
{
public:
	typedef Rt return_type;
	typedef Rt(Sd::*Ft)(Z2,Z3,Z4,Z5);

	typedef tl::tuple<Ft,Sd*,Z2,Z3,Z4,Z5> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return (k1->*ft)(k2,k3,k4,k5);
	}
};

template<typename Rt,typename Z1,typename Z2,typename Z3,typename Z4,typename Z5,typename Z6>
class FunctorImpl<Rt(*)(Z1,Z2,Z3,Z4,Z5,Z6)> : public BindImplBase
{
public:
	typedef Rt return_type;
	typedef Rt(*Ft)(Z1,Z2,Z3,Z4,Z5,Z6);

	typedef tl::tuple<Ft,Z1,Z2,Z3,Z4,Z5,Z6> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return ft(k1,k2,k3,k4,k5,k6);
	}
};


template<typename Rt,typename Sd,typename Z2,typename Z3,typename Z4,typename Z5,typename Z6>
class FunctorImpl<Rt(Sd::*)(Z2,Z3,Z4,Z5,Z6)> : public BindImplBase
{
public:
	typedef Rt return_type;
	typedef Rt(Sd::*Ft)(Z2,Z3,Z4,Z5,Z6);

	typedef tl::tuple<Ft,Sd*,Z2,Z3,Z4,Z5,Z6> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return (k1->*ft)(k2,k3,k4,k5,k6);
	}
};

template<typename Rt,typename Z1,typename Z2,typename Z3,typename Z4,typename Z5,typename Z6,typename Z7>
class FunctorImpl<Rt(*)(Z1,Z2,Z3,Z4,Z5,Z6,Z7)> : public BindImplBase
{
public:
	typedef Rt return_type;
	typedef Rt(*Ft)(Z1,Z2,Z3,Z4,Z5,Z6,Z7);

	typedef tl::tuple<Ft,Z1,Z2,Z3,Z4,Z5,Z6,Z7> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return ft(k1,k2,k3,k4,k5,k6,k7);
	}
};

template<typename Rt,typename Sd,typename Z2,typename Z3,typename Z4,typename Z5,typename Z6,typename Z7>
class FunctorImpl<Rt(Sd::*)(Z2,Z3,Z4,Z5,Z6,Z7)> : public BindImplBase
{
public:

	typedef Rt return_type;
	typedef Rt(Sd::*Ft)(Z2,Z3,Z4,Z5,Z6,Z7);

	typedef tl::tuple<Ft,Sd*,Z2,Z3,Z4,Z5,Z6,Z7> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return (k1->*ft)(k2,k3,k4,k5,k6,k7);
	}
};

template<typename Rt,typename Z1,typename Z2,typename Z3,typename Z4,typename Z5,typename Z6,typename Z7,typename Z8>
class FunctorImpl<Rt(*)(Z1,Z2,Z3,Z4,Z5,Z6,Z7,Z8)> : public BindImplBase
{
public:
	typedef Rt return_type;
	typedef Rt(*Ft)(Z1,Z2,Z3,Z4,Z5,Z6,Z7,Z8);

	typedef tl::tuple<Ft,Z1,Z2,Z3,Z4,Z5,Z6,Z7,Z8> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return ft(k1,k2,k3,k4,k5,k6,k7,k8);
	}
};

template<typename Rt,typename Sd,typename Z2,typename Z3,typename Z4,typename Z5,typename Z6,typename Z7,typename Z8>
class FunctorImpl<Rt(Sd::*)(Z2,Z3,Z4,Z5,Z6,Z7,Z8)> : public BindImplBase
{
public:
	typedef Rt return_type;
	typedef Rt(Sd::*Ft)(Z2,Z3,Z4,Z5,Z6,Z7,Z8);

	typedef tl::tuple<Ft,Sd*,Z2,Z3,Z4,Z5,Z6,Z7,Z8> param_type;
	BINDIMPL_EXPAND_TUPLE(param_type,K);

	static inline Rt g(Ft ft,K1 k1,K2 k2,K3 k3,K4 k4,K5 k5,K6 k6,K7 k7,K8 k8)
	{
		BINDIMPL_UNUSED_PARAM();
		return (k1->*ft)(k2,k3,k4,k5,k6,k7,k8);
	}
};

template<typename K0,typename K1=nil_t,typename K2=nil_t,typename K3=nil_t,typename K4=nil_t,typename K5=nil_t,typename K6=nil_t,typename K7=nil_t,typename K8=nil_t>
class ParamDispatch
{
public:
	static inline K0 g(K0 k0,K1 k1=K1(),K2 k2=K2(),K3 k3=K3(),K4 k4=K4(),K5 k5=K5(),K6 k6=K6(),K7 k7=K7(),K8 k8=K8())
	{
		BINDIMPL_UNUSED_PARAM();
		return k0;
	}
};

#define PARM_SELECTION(X) \
	template<typename K1,typename K2,typename K3,typename K4,typename K5,typename K6,typename K7,typename K8>\
	class ParamDispatch<tl::int2type<X>,K1,K2,K3,K4,K5,K6,K7,K8>\
	{\
	public:\
		typedef tl::int2type<X> K0;\
		static inline K##X g(K0 k0,K1 k1=K1(),K2 k2=K2(),K3 k3=K3(),K4 k4=K4(),K5 k5=K5(),K6 k6=K6(),K7 k7=K7(),K8 k8=K8())\
		{\
			(void)&k0;\
			(void)&k1;(void)&k2;(void)&k3;(void)&k4;\
			(void)&k5;(void)&k6;(void)&k7;(void)&k8;\
			return k##X;\
		}\
	};

PARM_SELECTION(1)
PARM_SELECTION(2)
PARM_SELECTION(3)
PARM_SELECTION(4)
PARM_SELECTION(5)
PARM_SELECTION(6)
PARM_SELECTION(7)
PARM_SELECTION(8)


#define BINDIMPL_OPERATOR0()\
	inline Rt operator()()\
	{\
		return basetype::g(\
			tl::tuple_at<tuple_type,0>::g(data),\
			tl::tuple_at<tuple_type,1>::g(data),\
			tl::tuple_at<tuple_type,2>::g(data),\
			tl::tuple_at<tuple_type,3>::g(data),\
			tl::tuple_at<tuple_type,4>::g(data),\
			tl::tuple_at<tuple_type,5>::g(data),\
			tl::tuple_at<tuple_type,6>::g(data),\
			tl::tuple_at<tuple_type,7>::g(data),\
			tl::tuple_at<tuple_type,8>::g(data)\
		);\
	}\


#define BIND_PM_K1 K1 k1
#define BIND_PM_K2 BIND_PM_K1,K2 k2
#define BIND_PM_K3 BIND_PM_K2,K3 k3
#define BIND_PM_K4 BIND_PM_K3,K4 k4
#define BIND_PM_K5 BIND_PM_K4,K5 k5
#define BIND_PM_K6 BIND_PM_K5,K6 k6
#define BIND_PM_K7 BIND_PM_K6,K7 k7
#define BIND_PM_K8 BIND_PM_K7,K8 k8

#define BIND_PM_X1 K1
#define BIND_PM_X2 BIND_PM_X1,K2
#define BIND_PM_X3 BIND_PM_X2,K3
#define BIND_PM_X4 BIND_PM_X3,K4
#define BIND_PM_X5 BIND_PM_X4,K5
#define BIND_PM_X6 BIND_PM_X5,K6
#define BIND_PM_X7 BIND_PM_X6,K7
#define BIND_PM_X8 BIND_PM_X7,K8

#define BIND_PM_Y1 k1
#define BIND_PM_Y2 BIND_PM_Y1,k2
#define BIND_PM_Y3 BIND_PM_Y2,k3
#define BIND_PM_Y4 BIND_PM_Y3,k4
#define BIND_PM_Y5 BIND_PM_Y4,k5
#define BIND_PM_Y6 BIND_PM_Y5,k6
#define BIND_PM_Y7 BIND_PM_Y6,k7
#define BIND_PM_Y8 BIND_PM_Y7,k8

#define BINDIMPL_OPERATOR_K(X,Y,Z)\
	inline Rt operator()(X)\
	{\
		return basetype::g(tl::tuple_at<tuple_type,0>::g(data),\
						   ParamDispatch<P1,Y>::g(tl::tuple_at<tuple_type,1>::g(data),Z),\
						   ParamDispatch<P2,Y>::g(tl::tuple_at<tuple_type,2>::g(data),Z),\
						   ParamDispatch<P3,Y>::g(tl::tuple_at<tuple_type,3>::g(data),Z),\
						   ParamDispatch<P4,Y>::g(tl::tuple_at<tuple_type,4>::g(data),Z),\
						   ParamDispatch<P5,Y>::g(tl::tuple_at<tuple_type,5>::g(data),Z),\
						   ParamDispatch<P6,Y>::g(tl::tuple_at<tuple_type,6>::g(data),Z),\
						   ParamDispatch<P7,Y>::g(tl::tuple_at<tuple_type,7>::g(data),Z),\
						   ParamDispatch<P8,Y>::g(tl::tuple_at<tuple_type,8>::g(data),Z)\
						  );\
	}

#define BINDIMPL_OPERATOR1() BINDIMPL_OPERATOR0() BINDIMPL_OPERATOR_K(BIND_PM_K1,BIND_PM_X1,BIND_PM_Y1)
#define BINDIMPL_OPERATOR2() BINDIMPL_OPERATOR1() BINDIMPL_OPERATOR_K(BIND_PM_K2,BIND_PM_X2,BIND_PM_Y2)
#define BINDIMPL_OPERATOR3() BINDIMPL_OPERATOR2() BINDIMPL_OPERATOR_K(BIND_PM_K3,BIND_PM_X3,BIND_PM_Y3)
#define BINDIMPL_OPERATOR4() BINDIMPL_OPERATOR3() BINDIMPL_OPERATOR_K(BIND_PM_K4,BIND_PM_X4,BIND_PM_Y4)
#define BINDIMPL_OPERATOR5() BINDIMPL_OPERATOR4() BINDIMPL_OPERATOR_K(BIND_PM_K5,BIND_PM_X5,BIND_PM_Y5)
#define BINDIMPL_OPERATOR6() BINDIMPL_OPERATOR5() BINDIMPL_OPERATOR_K(BIND_PM_K6,BIND_PM_X6,BIND_PM_Y6)
#define BINDIMPL_OPERATOR7() BINDIMPL_OPERATOR6() BINDIMPL_OPERATOR_K(BIND_PM_K7,BIND_PM_X7,BIND_PM_Y7)
#define BINDIMPL_OPERATOR8() BINDIMPL_OPERATOR7() BINDIMPL_OPERATOR_K(BIND_PM_K8,BIND_PM_X8,BIND_PM_Y8)

#define BINDIMPL_TYPENAME0() 
#define BINDIMPL_TYPENAME1() BINDIMPL_TYPENAME0(); typedef typename BindImplHelper<tuple_type,param_type,1>::type K1;
#define BINDIMPL_TYPENAME2() BINDIMPL_TYPENAME1(); typedef typename BindImplHelper<tuple_type,param_type,2>::type K2;
#define BINDIMPL_TYPENAME3() BINDIMPL_TYPENAME2(); typedef typename BindImplHelper<tuple_type,param_type,3>::type K3;
#define BINDIMPL_TYPENAME4() BINDIMPL_TYPENAME3(); typedef typename BindImplHelper<tuple_type,param_type,4>::type K4;
#define BINDIMPL_TYPENAME5() BINDIMPL_TYPENAME4(); typedef typename BindImplHelper<tuple_type,param_type,5>::type K5;
#define BINDIMPL_TYPENAME6() BINDIMPL_TYPENAME5(); typedef typename BindImplHelper<tuple_type,param_type,6>::type K6;
#define BINDIMPL_TYPENAME7() BINDIMPL_TYPENAME6(); typedef typename BindImplHelper<tuple_type,param_type,7>::type K7;
#define BINDIMPL_TYPENAME8() BINDIMPL_TYPENAME7(); typedef typename BindImplHelper<tuple_type,param_type,8>::type K8;

#define BINDIMPL_DECL_DATA()	\
	typedef FunctorImpl<X0> basetype;\
	typedef typename basetype::return_type Rt;\
	typedef typename basetype::param_type param_type;\
	typedef typename tl::tuple_mp<param_orig,ParamRealImpl>::type tuple_type;\
	BINDIMPL_EXPAND_TUPLE(tuple_type,P);\
	tuple_type data;

template<typename T1,typename T2,int N>
class BindImplHelper
{
public:
	static const int value=tl::tuple_id<T1,tl::int2type<N> >::value;
	typedef typename tl::tuple_at<T2,value>::type type;
};

template<typename Ft>
class BindImpl;

template<typename X0>
class BindImpl<void(X0)> : public FunctorImpl<X0>
{
public:
	typedef tl::tuple<X0> param_orig;
	BINDIMPL_DECL_DATA();
	BINDIMPL_TYPENAME0();
	BINDIMPL_OPERATOR0();

	BindImpl(P0 p0):data(p0){}
};

template<typename X0,typename X1>
class BindImpl<void(X0,X1)> : public FunctorImpl<X0>
{
public:
	typedef tl::tuple<X0,X1> param_orig;
	BINDIMPL_DECL_DATA();
	BINDIMPL_TYPENAME1();
	BINDIMPL_OPERATOR1();

	BindImpl(P0 p0,P1 p1):data(p0,p1){}
};


template<typename X0,typename X1,typename X2>
class BindImpl<void(X0,X1,X2)> : public FunctorImpl<X0>
{
public:
	typedef tl::tuple<X0,X1,X2> param_orig;
	BINDIMPL_DECL_DATA();	
	BINDIMPL_TYPENAME2();
	BINDIMPL_OPERATOR2();

	BindImpl(P0 p0,P1 p1,P2 p2):data(p0,p1,p2){}
};


template<typename X0,typename X1,typename X2,typename X3>
class BindImpl<void(X0,X1,X2,X3)> : public FunctorImpl<X0>
{
public:
	typedef tl::tuple<X0,X1,X2,X3> param_orig;
	BINDIMPL_DECL_DATA();
	BINDIMPL_TYPENAME3();
	BINDIMPL_OPERATOR3();

	BindImpl(P0 p0,P1 p1,P2 p2,P3 p3):data(p0,p1,p2,p3){}
};


template<typename X0,typename X1,typename X2,typename X3,typename X4>
class BindImpl<void(X0,X1,X2,X3,X4)> : public FunctorImpl<X0>
{
public:
	typedef tl::tuple<X0,X1,X2,X3,X4> param_orig;
	BINDIMPL_DECL_DATA();
	BINDIMPL_TYPENAME4();
	BINDIMPL_OPERATOR4();

	BindImpl(P0 p0,P1 p1,P2 p2,P3 p3,P4 p4):data(p0,p1,p2,p3,p4){}
};


template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5>
class BindImpl<void(X0,X1,X2,X3,X4,X5)> : public FunctorImpl<X0>
{
public:
	typedef tl::tuple<X0,X1,X2,X3,X4,X5> param_orig;
	BINDIMPL_DECL_DATA();
	BINDIMPL_TYPENAME5();
	BINDIMPL_OPERATOR5();

	BindImpl(P0 p0,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5):data(p0,p1,p2,p3,p4,p5){}
};


template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5,typename X6>
class BindImpl<void(X0,X1,X2,X3,X4,X5,X6)> : public FunctorImpl<X0>
{
public:
	typedef tl::tuple<X0,X1,X2,X3,X4,X5,X6> param_orig;
	BINDIMPL_DECL_DATA();
	BINDIMPL_TYPENAME6();
	BINDIMPL_OPERATOR6();

	BindImpl(P0 p0,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6):data(p0,p1,p2,p3,p4,p5,p6){}
};


template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5,typename X6,typename X7>
class BindImpl<void(X0,X1,X2,X3,X4,X5,X6,X7)> : public FunctorImpl<X0>
{
public:
	typedef tl::tuple<X0,X1,X2,X3,X4,X5,X6,X7> param_orig;
	BINDIMPL_DECL_DATA();
	BINDIMPL_TYPENAME7();
	BINDIMPL_OPERATOR7();

	BindImpl(P0 p0,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7):data(p0,p1,p2,p3,p4,p5,p6,p7){}
};


template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5,typename X6,typename X7,typename X8>
class BindImpl<void(X0,X1,X2,X3,X4,X5,X6,X7,X8)> : public FunctorImpl<X0>
{
public:
	typedef tl::tuple<X0,X1,X2,X3,X4,X5,X6,X7,X8> param_orig;
	BINDIMPL_DECL_DATA();
	BINDIMPL_TYPENAME8();
	BINDIMPL_OPERATOR8();

	BindImpl(P0 p0,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,P8 p8):data(p0,p1,p2,p3,p4,p5,p6,p7,p8){}
};



EW_LEAVE

