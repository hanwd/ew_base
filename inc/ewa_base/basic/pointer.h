#ifndef __H_EW_SMART_PTR__
#define __H_EW_SMART_PTR__

#include "ewa_base/basic/object.h"
#include "ewa_base/basic/pointer_detail.h"
#include "ewa_base/basic/platform.h"

EW_ENTER

// LitePtrT almost same as T*, but will initalize m_ptr to null pointer in default constructor.
template<typename T>
class LitePtrT : public detail::BasePtrT<T,detail::po_lite<T> >
{
public:

	typedef detail::BasePtrT<T,detail::po_lite<T> > basetype;
	typedef T* pointer;
	using basetype::m_ptr;

	LitePtrT(pointer p=NULL):basetype(p) {}

	inline operator pointer()
	{
		return m_ptr;
	}

	inline operator const T*() const
	{
		return m_ptr;
	}

	inline pointer operator=(pointer p)
	{
		m_ptr=p;
		return m_ptr;
	}
};

// Reference counted, T must derive from ObjectData.
template<typename T>
class DataPtrT : public detail::BasePtrT<T,detail::po_data<T> >
{
public:
    typedef detail::BasePtrT<T,detail::po_data<T> > basetype;

	template<typename X>
	DataPtrT(const DataPtrT<X>& o)
	{
		basetype::reset((X*)o.get());
	}

	DataPtrT(T *p=NULL):basetype(p) {}

	bool cast_and_set(ObjectData* b)
	{
		T* p=dynamic_cast<T*>(b);

		if(p)
		{
			basetype::reset(p);
			return true;
		}

		if(b && b->GetRef()==0)
		{
			delete b;
		}

		basetype::reset(NULL);
		return false;
	}

};

// AutoPtrT almost same as std::auto_ptr, but no copy constructor and operator=
template<typename T>
class AutoPtrT : public detail::BasePtrT<T,detail::po_auto<T> > , private NonCopyable
{
	using detail::BasePtrT<T,detail::po_auto<T> >::m_ptr;

public:
	explicit AutoPtrT(T *p=NULL) : detail::BasePtrT<T,detail::po_auto<T> >(p) {}

	inline T *release()
	{
		T *tmp=m_ptr;
		m_ptr=NULL;
		return tmp;
	}

};

template<typename T>
class TempPtrT : public AutoPtrT<T>
{
public:

	TempPtrT() {}
	TempPtrT(T* p):AutoPtrT<T>(p) {}
	TempPtrT(const TempPtrT& o)
	{
		this->swap(const_cast<TempPtrT&>(o));
	}
	TempPtrT& operator=(const TempPtrT& o)
	{
		this->swap(const_cast<TempPtrT&>(o));
		return *this;
	}
};


// if T is not derived from ObjectData, it's same as LitePtrT, else DataPtrT
template<typename T>
class FakePtrT : public tl::meta_if<tl::is_convertible<T,ObjectData>::value,DataPtrT<T>,LitePtrT<T> >::type
{
public:
	typedef typename tl::meta_if<tl::is_convertible<T,ObjectData>::value,DataPtrT<T>,LitePtrT<T> >::type basetype;
	FakePtrT(T* p=NULL):basetype(p) {}

};


template<typename T>
class KO_Pointer
{
public:
	typedef T* type;
	typedef type const_reference;
	static type invalid_value(){return NULL;}
	static void destroy(type& o){ delete o; }
};

template<typename T>
class SharedPtrT
{
public:

	typedef T* pointer;
	typedef const T* const_pointer;

	SharedPtrT(pointer p = NULL)
	{
		handle.reset(p);
	}

	SharedPtrT(const SharedPtrT& o) :handle(o.handle)
	{

	}

	SharedPtrT(const KO_Handle<KO_Pointer<T> >& o) :handle(o)
	{

	}

	void reset(pointer p)
	{
		handle.reset(p);
	}
	
	pointer get()
	{
		return handle;
	}

	const_pointer get() const
	{
		return handle;
	}

	pointer operator->()
	{
		return get();
	}

	const_pointer operator->() const
	{
		return get();
	}

	operator bool() const
	{
		return get() != NULL;
	}

	void swap(SharedPtrT& o)
	{
		handle.swap(o);
	}

	mutable KO_Handle<KO_Pointer<T> > handle;
};

template<typename T>
class WeakPtrT
{
public:

	WeakPtrT(){}
	WeakPtrT(const SharedPtrT<T>& o) :handle(o.handle){}
	WeakPtrT(const WeakPtrT<T>& o) :handle(o.handle){}

	WeakPtrT& operator=(const SharedPtrT<T>& o)
	{
		handle = o.handle;
		return *this;
	}

	WeakPtrT& operator=(const WeakPtrT<T>& o)
	{
		handle = o.handle;
		return *this;
	}

	SharedPtrT<T> lock()
	{
		return handle.lock();
	}

	void swap(WeakPtrT& o)
	{
		handle.swap(o);
	}

	mutable KO_Weak<KO_Pointer<T> > handle;
};


template<typename T,typename B=void>
class ObjectT : public tl::meta_if<tl::is_same_type<B,void>::value,tl::emptytype,B>::type
{
public:
	typedef T obj_type;

	ObjectT() {}
	ObjectT(T* p):m_refData(p) {}

	inline void SetData(T* d)
	{
		m_refData.reset(d);
	}

	inline T* GetData()
	{
		return m_refData.get();
	}

	inline const T* GetData() const
	{
		return m_refData.get();
	}

	void UnShare()
	{
		if(m_refData && m_refData->GetRef()!=1) SetData(static_cast<T*>(m_refData->Clone(1)));
	}

	void swap(ObjectT& other)
	{
		m_refData.swap(other.m_refData);
	}

protected:
	DataPtrT<T> m_refData;
};


#define EW_BASEPTRT_REL_OP(X) \
	template<typename T,typename P1,typename P2>\
	inline bool operator X(const detail::BasePtrT<T,P1>& lhs,const detail::BasePtrT<T,P2>& rhs)\
	{\
		return lhs.get() X rhs.get();\
	}\
	template<typename T,typename P>\
	inline bool operator X(const detail::BasePtrT<T,P>& lhs,T* rhs)\
	{\
		return lhs.get() X rhs;\
	}\
	template<typename T,typename P>\
	inline bool operator X(T* lhs,const detail::BasePtrT<T,P>& rhs)\
	{\
		return lhs X rhs.get();\
	}\

EW_BASEPTRT_REL_OP(==)
EW_BASEPTRT_REL_OP(!=)
EW_BASEPTRT_REL_OP(>=)
EW_BASEPTRT_REL_OP(>)
EW_BASEPTRT_REL_OP(<)
EW_BASEPTRT_REL_OP(<=)

#undef EW_BASEPTRT_REL_OP


template<typename T> class hash_t<DataPtrT<T> >
{
public:
	inline uint32_t operator()(const DataPtrT<T>& val)
	{
		return (uintptr_t(val.get()))>>4;
	}
};


EW_LEAVE


#endif

