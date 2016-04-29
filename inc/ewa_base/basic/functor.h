// Copyright 2014, Wenda han.  All rights reserved.
// https://github.com/vhwd/ew6_base
//
/// Use of this source code is governed by Apache License
// that can be found in the License file.
// Author: Wenda Han.

#ifndef __H_EW_BASIC_FUNCTOR__
#define __H_EW_BASIC_FUNCTOR__

#include "ewa_base/basic/exception.h"
#include "ewa_base/basic/pointer.h"
#include "ewa_base/basic/functor_detail.h"

EW_ENTER

extern DLLIMPEXP_EWA_BASE tl::int2type<1> _1;
extern DLLIMPEXP_EWA_BASE tl::int2type<2> _2;
extern DLLIMPEXP_EWA_BASE tl::int2type<3> _3;
extern DLLIMPEXP_EWA_BASE tl::int2type<4> _4;
extern DLLIMPEXP_EWA_BASE tl::int2type<5> _5;
extern DLLIMPEXP_EWA_BASE tl::int2type<6> _6;


//hbind(function,parameter1,parameter2,...,parametern);
//sample usage:
//double fma(double,double,double);
//auto f0=hbind(fma,1.0,2.0,3.0); f0();
//auto f1=hbind(fma,1.0,_1,3.0); f1(2.0);
//auto f2=hbind(fma,1.0,_2,_1); f2(3.0,2.0);
//auto f3=hbind(fma,_2,_3,_1); f3(3.0,1.0,2.0);
//auto f4=hbind(f2,3.0,2.0); f4();

// Functor f0 to f4 all call fma(1.0,2.0,3.0);


template<typename P0>
static inline BindImpl<void(P0)> hbind(P0 p0)
{
	return BindImpl<void(P0)>(p0);
}

template<typename P0,typename P1>
static inline BindImpl<void(P0,P1)> hbind(P0 p0,P1 p1)
{
	return BindImpl<void(P0,P1)>(p0,p1);
}

template<typename P0,typename P1,typename P2>
static inline BindImpl<void(P0,P1,P2)> hbind(P0 p0,P1 p1,P2 p2)
{
	return BindImpl<void(P0,P1,P2)>(p0,p1,p2);
}

template<typename P0,typename P1,typename P2,typename P3>
static inline BindImpl<void(P0,P1,P2,P3)> hbind(P0 p0,P1 p1,P2 p2,P3 p3)
{
	return BindImpl<void(P0,P1,P2,P3)>(p0,p1,p2,p3);
}

template<typename P0,typename P1,typename P2,typename P3,typename P4>
static inline BindImpl<void(P0,P1,P2,P3,P4)> hbind(P0 p0,P1 p1,P2 p2,P3 p3,P4 p4)
{
	return BindImpl<void(P0,P1,P2,P3,P4)>(p0,p1,p2,p3,p4);
}

template<typename P0,typename P1,typename P2,typename P3,typename P4,typename P5>
static inline BindImpl<void(P0,P1,P2,P3,P4,P5)> hbind(P0 p0,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
{
	return BindImpl<void(P0,P1,P2,P3,P4,P5)>(p0,p1,p2,p3,p4,p5);
}

template<typename P0,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6>
static inline BindImpl<void(P0,P1,P2,P3,P4,P5,P6)> hbind(P0 p0,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
{
	return BindImpl<void(P0,P1,P2,P3,P4,P5,P6)>(p0,p1,p2,p3,p4,p5,p6);
}

template<typename P0,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7>
static inline BindImpl<void(P0,P1,P2,P3,P4,P5,P6,P7)> hbind(P0 p0,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
{
	return BindImpl<void(P0,P1,P2,P3,P4,P5,P6,P7)>(p0,p1,p2,p3,p4,p5,p6,p7);
}



template<typename T>
class Functor;


template<typename T>
class Functor<T()>
{
public:

	class Invoker: public ObjectData
	{
		const Invoker& operator=(const Invoker&);
	public:
		virtual T Invoke()=0;
	};

	Functor() {}
	Functor(const Functor& o):invoker(o.invoker) {}
	template<typename G>
	Functor(const G& o)
	{
		*this=o;
	}

	Functor& operator=(const Functor& o)
	{
		invoker=o.invoker;
		return *this;
	}

	template<typename G>
	Functor& operator=(const G& o)
	{
		class KInvoker : public Invoker
		{
		public:
			G impl;
			KInvoker(const G& o):impl(o) {}
			virtual T Invoke()
			{
				return impl();
			}
		};
		invoker.reset(new KInvoker(o));
		return *this;
	}

	void reset(Invoker* p)
	{
		invoker.reset(p);
	}
	void clear()
	{
		invoker.reset(NULL);
	}

	template<typename X0>
	void bind(X0 x0)
	{
		(*this)=hbind(x0);
	}

	template<typename X0,typename X1>
	void bind(X0 x0,X1 x1)
	{
		(*this)=hbind(x0,x1);
	}

	template<typename X0,typename X1,typename X2>
	void bind(X0 x0,X1 x1,X2 x2)
	{
		(*this)=hbind(x0,x1,x2);
	}

	template<typename X0,typename X1,typename X2,typename X3>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3)
	{
		(*this)=hbind(x0,x1,x2,x3);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4)
	{
		(*this)=hbind(x0,x1,x2,x3,x4);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4,X5 x5)
	{
		(*this)=hbind(x0,x1,x2,x3,x4,x5);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5,typename X6>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4,X5 x5,X6 x6)
	{
		(*this)=hbind(x0,x1,x2,x3,x4,x5,x6);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5,typename X6,typename X7>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4,X5 x5,X6 x6,X7 x7)
	{
		(*this)=hbind(x0,x1,x2,x3,x4,x5,x6,x7);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5,typename X6,typename X7,typename X8>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4,X5 x5,X6 x6,X7 x7,X8 x8)
	{
		(*this)=hbind(x0,x1,x2,x3,x4,x5,x6,x7,x8);
	}


	inline T operator()()
	{
		if(!invoker) Exception::XBadFunctor();
		return invoker->Invoke();
	}
	inline operator bool() const
	{
		return invoker.get()!=NULL;
	}

private:
	DataPtrT<Invoker> invoker;
};

template<typename T,typename X>
class Functor<T(X)>
{
public:

	class Invoker: public ObjectData
	{
		const Invoker& operator=(const Invoker&);
	public:
		virtual T Invoke(X)=0;
	};

	void reset(Invoker* p)
	{
		invoker.reset(p);
	}
	void clear()
	{
		invoker.reset(NULL);
	}

	template<typename X0>
	void bind(X0 x0)
	{
		(*this)=hbind(x0);
	}

	template<typename X0,typename X1>
	void bind(X0 x0,X1 x1)
	{
		(*this)=hbind(x0,x1);
	}

	template<typename X0,typename X1,typename X2>
	void bind(X0 x0,X1 x1,X2 x2)
	{
		(*this)=hbind(x0,x1,x2);
	}

	template<typename X0,typename X1,typename X2,typename X3>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3)
	{
		(*this)=hbind(x0,x1,x2,x3);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4)
	{
		(*this)=hbind(x0,x1,x2,x3,x4);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4,X5 x5)
	{
		(*this)=hbind(x0,x1,x2,x3,x4,x5);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5,typename X6>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4,X5 x5,X6 x6)
	{
		(*this)=hbind(x0,x1,x2,x3,x4,x5,x6);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5,typename X6,typename X7>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4,X5 x5,X6 x6,X7 x7)
	{
		(*this)=hbind(x0,x1,x2,x3,x4,x5,x6,x7);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5,typename X6,typename X7,typename X8>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4,X5 x5,X6 x6,X7 x7,X8 x8)
	{
		(*this)=hbind(x0,x1,x2,x3,x4,x5,x6,x7,x8);
	}

	Functor() {}
	Functor(const Functor& o):invoker(o.invoker) {}
	template<typename G>
	Functor(const G& o)
	{
		*this=o;
	}

	Functor& operator=(const Functor& o)
	{
		invoker=o.invoker;
		return *this;
	}

	template<typename G>
	Functor& operator=(const G& o)
	{
		class KInvoker : public Invoker
		{
		public:
			G impl;
			KInvoker(const G& o):impl(o) {}
			virtual T Invoke(X x)
			{
				return impl(x);
			}
		};
		invoker.reset(new KInvoker(o));
		return *this;
	}

	inline T operator()(X x)
	{
		if(!invoker) Exception::XBadFunctor();
		return invoker->Invoke(x);
	}
	inline operator bool() const
	{
		return invoker.get()!=NULL;
	}

private:
	DataPtrT<Invoker> invoker;
};


template<typename T,typename X,typename Y>
class Functor<T(X,Y)>
{
public:

	class Invoker: public ObjectData
	{
		const Invoker& operator=(const Invoker&);
	public:
		virtual T Invoke(X,Y)=0;
	};

	void reset(Invoker* p)
	{
		invoker.reset(p);
	}
	void clear()
	{
		invoker.reset(NULL);
	}

	template<typename X0>
	void bind(X0 x0)
	{
		(*this)=hbind(x0);
	}

	template<typename X0,typename X1>
	void bind(X0 x0,X1 x1)
	{
		(*this)=hbind(x0,x1);
	}

	template<typename X0,typename X1,typename X2>
	void bind(X0 x0,X1 x1,X2 x2)
	{
		(*this)=hbind(x0,x1,x2);
	}

	template<typename X0,typename X1,typename X2,typename X3>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3)
	{
		(*this)=hbind(x0,x1,x2,x3);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4)
	{
		(*this)=hbind(x0,x1,x2,x3,x4);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4,X5 x5)
	{
		(*this)=hbind(x0,x1,x2,x3,x4,x5);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5,typename X6>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4,X5 x5,X6 x6)
	{
		(*this)=hbind(x0,x1,x2,x3,x4,x5,x6);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5,typename X6,typename X7>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4,X5 x5,X6 x6,X7 x7)
	{
		(*this)=hbind(x0,x1,x2,x3,x4,x5,x6,x7);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5,typename X6,typename X7,typename X8>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4,X5 x5,X6 x6,X7 x7,X8 x8)
	{
		(*this)=hbind(x0,x1,x2,x3,x4,x5,x6,x7,x8);
	}

	Functor() {}
	Functor(const Functor& o):invoker(o.invoker) {}
	template<typename G>
	Functor(const G& o)
	{
		*this=o;
	}

	Functor& operator=(const Functor& o)
	{
		invoker=o.invoker;
		return *this;
	}

	template<typename G>
	Functor& operator=(const G& o)
	{
		class KInvoker : public Invoker
		{
		public:
			G impl;
			KInvoker(const G& o):impl(o) {}
			virtual T Invoke(X x,Y y)
			{
				return impl(x,y);
			}
		};
		invoker.reset(new KInvoker(o));
		return *this;
	}

	inline T operator()(X x,Y y)
	{
		if(!invoker) Exception::XBadFunctor();
		return invoker->Invoke(x,y);
	}
	inline operator bool() const
	{
		return invoker.get()!=NULL;
	}

private:
	DataPtrT<Invoker> invoker;
};


template<typename T,typename X,typename Y,typename Z>
class Functor<T(X,Y,Z)>
{
public:

	class Invoker: public ObjectData
	{
		const Invoker& operator=(const Invoker&);
	public:
		virtual T Invoke(X,Y,Z)=0;
	};

	void reset(Invoker* p)
	{
		invoker.reset(p);
	}
	void clear()
	{
		invoker.reset(NULL);
	}

	template<typename X0>
	void bind(X0 x0)
	{
		(*this)=hbind(x0);
	}

	template<typename X0,typename X1>
	void bind(X0 x0,X1 x1)
	{
		(*this)=hbind(x0,x1);
	}

	template<typename X0,typename X1,typename X2>
	void bind(X0 x0,X1 x1,X2 x2)
	{
		(*this)=hbind(x0,x1,x2);
	}

	template<typename X0,typename X1,typename X2,typename X3>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3)
	{
		(*this)=hbind(x0,x1,x2,x3);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4)
	{
		(*this)=hbind(x0,x1,x2,x3,x4);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4,X5 x5)
	{
		(*this)=hbind(x0,x1,x2,x3,x4,x5);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5,typename X6>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4,X5 x5,X6 x6)
	{
		(*this)=hbind(x0,x1,x2,x3,x4,x5,x6);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5,typename X6,typename X7>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4,X5 x5,X6 x6,X7 x7)
	{
		(*this)=hbind(x0,x1,x2,x3,x4,x5,x6,x7);
	}

	template<typename X0,typename X1,typename X2,typename X3,typename X4,typename X5,typename X6,typename X7,typename X8>
	void bind(X0 x0,X1 x1,X2 x2,X3 x3,X4 x4,X5 x5,X6 x6,X7 x7,X8 x8)
	{
		(*this)=hbind(x0,x1,x2,x3,x4,x5,x6,x7,x8);
	}

	Functor() {}
	Functor(const Functor& o):invoker(o.invoker) {}
	template<typename G>
	Functor(const G& o)
	{
		*this=o;
	}

	Functor& operator=(const Functor& o)
	{
		invoker=o.invoker;
		return *this;
	}

	template<typename G>
	Functor& operator=(const G& o)
	{
		class KInvoker : public Invoker
		{
		public:
			G impl;
			KInvoker(const G& o):impl(o) {}
			virtual T Invoke(X x,Y y,Z z)
			{
				return impl(x,y,z);
			}
		};
		invoker.reset(new KInvoker(o));
		return *this;
	}

	inline T operator()(X x,Y y,Z z)
	{
		if(!invoker) Exception::XBadFunctor();
		return invoker->Invoke(x,y,z);
	}
	inline operator bool() const
	{
		return invoker.get()!=NULL;
	}

private:
	DataPtrT<Invoker> invoker;
};


EW_LEAVE

#endif
