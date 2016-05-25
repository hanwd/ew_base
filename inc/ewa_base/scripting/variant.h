#ifndef __H_EW_SCRIPTING_VARIANT__
#define __H_EW_SCRIPTING_VARIANT__

#include "ewa_base/config.h"
#include "ewa_base/basic.h"
#include "ewa_base/collection.h"
#include "ewa_base/math/tiny_cpx.h"
#include "ewa_base/scripting/detail/colondata.h"
#include "ewa_base/scripting/detail/idx_1t.h"
#include "ewa_base/serialization/serializer.h"
#include "ewa_base/scripting/callable_data.h"

EW_ENTER


typedef tl::mk_typelist<CallableData*,bool,int64_t,double>::type TL_VARIANT_TYPE1;

template<typename T,bool F=TL_VARIANT_TYPE1::template id<T>::value>=0>
class variant_handler_impl;

class DLLIMPEXP_EWA_BASE Variant
{
public:

	template<typename T,bool F>
	friend class variant_handler_impl;

	friend class Executor;

	inline Variant():flag(0)
	{
		data.pval=NULL;
	}

	inline Variant(CallableData* p)
	{
		if(p)
		{
			data.pval=p;
			data.pval->IncRef();
			flag=-1;
		}
		else
		{
			data.pval=NULL;
			flag=0;
		}
	}

#ifdef EW_C11
	template<typename T>
	explicit Variant(T&& v):flag(0)
	{
		reset(std::forward<typename std::remove_const<T>::type>(v));
	}
#else
	template<typename T>
	explicit Variant(const T& v):flag(0)
	{
		reset(v);
	}
#endif

	inline ~Variant()
	{
		if(flag==-1) data.pval->DecRef();
	}

	inline Variant(const Variant& o):data(o.data),flag(o.flag)
	{
		if(flag==-1) data.pval->IncRef();
	}

	inline Variant& operator=(const Variant& o)
	{
		if(o.flag==-1)
		{
			o.data.pval->IncRef();
		}
		if(flag==-1)
		{
			data.pval->DecRef();
		}

		data=o.data;
		flag=o.flag;

		return *this;
	}

#ifdef EW_C11
	inline Variant(Variant&& o) :flag(o.flag), data(o.data)
	{
		o.flag = 0;
	}
	inline Variant& operator=(Variant&& o)
	{
		swap(o);return *this;
	}
#endif

	inline int type() const
	{
		return flag>=0?flag:data.pval->type();
	}

	inline bool is_nil() const
	{
		return flag==0;
	}

	inline CallableData* kptr()
	{
		return flag==-1?data.pval:NULL;
	}

	inline CallableMetatable* get_metatable()
	{
		return flag==-1?data.pval->GetMetaTable():CallableData::sm_meta[flag];
	}

	inline const CallableData* kptr() const
	{
		return flag==-1?data.pval:NULL;
	}

	void kptr(CallableData* p);

	template<typename T>
	inline typename tl::enable_cv<T,CallableData>::type kptr(const DataPtrT<T>& p)
	{
		kptr(const_cast<T*>(p.get()));
	}

	void unique();

	// throw if not T
	template<typename T> T& get();
	template<typename T> const T& get() const;


	// change type to T if not T
	template<typename T> T& ref();

	// ref()+unique()
	template<typename T> T& ref_unique();

	template<typename T> T* ptr();
	template<typename T> const T* ptr() const;

	template<typename T> bool set(const T& v);
	template<typename T> bool get(T& v) const;

#ifdef EW_C11
	template<typename T> void reset(T&& v=T());
#else
	template<typename T> void reset(const T& v=T());
#endif

	template<typename T> typename tl::enable_if_c<tl::is_convertible<T,CallableData> >::type reset(T* v)
	{
		kptr(v);
	}

	void clear();

	inline void swap(Variant& o)
	{
		std::swap(flag, o.flag);
		std::swap(data, o.data);
	}

	bool operator==(const Variant& v2) const;
	bool operator!=(const Variant& v2) const;

	void Serialize(Serializer& ar);


protected:

	union variant_data_union
	{
		CallableData* pval;
		double dval;
		int64_t ival;
		bool bval;
	}data;

	int64_t flag;

};


DLLIMPEXP_EWA_BASE String string_format_variants(Variant* p,int n);

DEFINE_OBJECT_NAME(Variant,"Variant")


template<>
class variant_handler_impl<CallableData*,true>
{
public:

	static const int value=-1;
	typedef CallableData* T;

	static const T& raw(const Variant& v)
	{
		EW_ASSERT(v.flag<1);
		return v.data.pval;
	}

	static T& raw(Variant& v)
	{
		EW_ASSERT(v.flag<1);
		return v.data.pval;
	}

	static void clone(Variant& d,CallableData* p)
	{
		EW_ASSERT(p!=NULL);
		d.kptr(p->Clone());
	}

	template<typename X>
	static X* ref_unique(Variant& v,X* d)
	{
		v.kptr(d);return d;
	}

	template<typename X>
	static X* ref_unique(Variant& v,DataPtrT<X> d)
	{
		v.kptr(d.get());return d.get();
	}

};


template<typename T>
class variant_handler_impl<T,false>
{
public:

	static const int value=type_flag<T>::value;

	static const T& raw(const Variant& v)
	{
		EW_ASSERT(v.type()==value);
		return static_cast<const CallableDataBaseT<T>*>(v.data.pval)->value;
	}

	static T& raw(Variant& v)
	{
		EW_ASSERT(v.type()==value);
		return static_cast<CallableDataBaseT<T>*>(v.data.pval)->value;
	}

	static T* ptr(Variant& v)
	{
		if(v.flag!=-1)
		{
			return NULL;
		}

		if(type_flag<T>::value==0)
		{
			CallableDataBaseT<T>* p=dynamic_cast<CallableDataBaseT<T>*>(v.data.pval);
			if(!p) return NULL;
			return &p->value;
		}
		else if(type_flag<T>::value!=v.data.pval->type())
		{
			return NULL;
		}

		return &raw(v);
	}

	static T& ref(Variant& v)
	{
		T* p=ptr(v);
		if(!p)
		{
			v.kptr(new CallableWrapT<T>());
			return raw(v);
		}
		return *p;
	}

	static T& ref_unique(Variant& v)
	{
		T* p=ptr(v);
		if(!p)
		{
			v.kptr(new CallableWrapT<T>());
			return raw(v);
		}
		else if(v.data.pval->GetRef()!=1)
		{
			variant_handler_impl<CallableData*,true>::clone(v,v.data.pval);
			return raw(v);
		}
		return *p;
	}

	static T& ref_unique(Variant& v,const T& d)
	{
		T* p=ptr(v);
		if(!p||v.data.pval->GetRef()!=1)
		{
			v.kptr(new CallableWrapT<T>(d));
			return raw(v);
		}
		else
		{
			*p = d;
		}
		return *p;
	}

#ifdef EW_C11

	static T& ref_unique(Variant& v, T&& d)
	{
		T* p=ptr(v);
		if(!p||v.data.pval->GetRef()!=1)
		{
			v.kptr(new CallableWrapT<T>(std::forward<T>(d)));
			return raw(v);
		}
		else
		{
			*p = d;
		}
		return *p;
	}
#endif

};

template<typename T>
class variant_handler_impl<T,true>
{
public:
	static const int value=type_flag<T>::value;

	static const T& raw(const Variant& v)
	{
		EW_ASSERT(v.type()==value);
		return *(T*)&v.data;
	}

	static T& raw(Variant& v)
	{
		EW_ASSERT(v.type()==value);
		return *(T*)&v.data;
	}
	static T* ptr(Variant& v)
	{
		if(v.flag!=value) return NULL;
		return (T*)&v.data;
	}

	static T& get(Variant& v)
	{
		if(v.flag!=value) Exception::XBadCast();
		return raw(v);
	}

	static T& ref(Variant& v)
	{
		return v.flag!=value ? ref_unique(v,T()):raw(v);
	}

	static T& ref_unique(Variant& v)
	{
		return v.flag!=value ? ref_unique(v,T()):raw(v);
	}

	static T& ref_unique(Variant& v,const T& d)
	{
		if(v.flag==-1) v.data.pval->DecRef();
		v.flag=value;
		return (raw(v)=d);
	}
};


template<>
class variant_handler_impl<Variant,false>
{
public:

	typedef Variant T;
	static const T& raw(const Variant& v)
	{
		return v;
	}

	static T& raw(Variant& v)
	{
		return v;
	}
	static T* ptr(Variant& v)
	{
		return &v;
	}

	static T& get(Variant& v)
	{
		return v;
	}

	static T& ref(Variant& v)
	{
		return v;
	}

	static T& ref_unique(Variant& v)
	{
		return v;
	}

	static T& ref_unique(Variant& v,const T& d)
	{
		return (v=d);
	}
};


template<typename T> class variant_handler : public variant_handler_impl<T>{};


inline void Variant::unique()
{
	if(flag==-1&&data.pval->GetRef()!=1)
	{
		variant_handler<CallableData*>::clone(*this,data.pval);
	}
}

template<typename T>
inline T& Variant::get()
{
	T* p=variant_handler<T>::ptr(*this);
	if(!p) Exception::XBadCast();
	return *p;
}

template<typename T>
inline const T& Variant::get() const
{
	const T* p=variant_handler<T>::ptr(*this);
	if(!p) Exception::XBadCast();
	return *p;
}


template<typename T>
inline T& Variant::ref()
{
	return variant_handler<T>::ref(*this);
}

template<typename T>
inline T& Variant::ref_unique()
{
	return variant_handler<T>::ref_unique(*this);
}


template<typename T>
inline T* Variant::ptr()
{
	return variant_handler<T>::ptr(*this);
}

template<typename T>
inline const T* Variant::ptr() const
{
	return variant_handler<T>::ptr(*(Variant*)this);
}

template<typename T>
inline bool Variant::get(T& v) const
{
	const T* p=ptr<T>();
	if(!p) return false;
	v=*p;
	return true;
}

template<typename T>
inline bool Variant::set(const T& v)
{
	T* p=ptr<T>();
	if(!p) return false;
	*p=v;
	return true;
}

namespace detail
{
	template<typename T> struct variant_type_internal{typedef T type;};

	template<> struct variant_type_internal<long>{typedef int64_t type;};
	template<> struct variant_type_internal<int32_t>{typedef int64_t type;};
	template<> struct variant_type_internal<uint32_t>{typedef int64_t type;};
	template<> struct variant_type_internal<uint64_t>{typedef int64_t type;};

	template<> struct variant_type_internal<float>{typedef double type;};

	template<> struct variant_type_internal<std::complex<double> >{typedef dcomplex type;};
	template<> struct variant_type_internal<std::complex<float> >{typedef dcomplex type;};
	template<> struct variant_type_internal<fcomplex>{typedef dcomplex type;};

	template<> struct variant_type_internal<const char*>{typedef String type;};
	template<> struct variant_type_internal<char*>{typedef String type;};

	template<int N> struct variant_type_internal<const char[N]>{typedef String type;};

	template<typename T> struct variant_type_internal<const T> : public variant_type_internal<T>{};
	template<typename T> struct variant_type_internal<T&> : public variant_type_internal<T>{};

	template<typename T> struct variant_type_internal<DataPtrT<T> > : public variant_type_internal<CallableData*>{};
	template<typename T> struct variant_type_internal<T*> : public variant_type_internal<CallableData*>{};

}

#ifdef EW_C11
template<typename T>
EW_FORCEINLINE void Variant::reset(T &&v)
{
	typedef typename std::remove_const<T>::type type0;
	typedef typename std::remove_reference<T>::type type1;
	typedef typename detail::variant_type_internal<type1>::type type2;
	variant_handler<type2>::ref_unique(*this,std::forward<type0>(v));
}
#else
template<typename T>
EW_FORCEINLINE void Variant::reset(const T& v)
{
	variant_handler<typename detail::variant_type_internal<T>::type>::ref_unique(*this, v);
}
#endif

EW_FORCEINLINE void Variant::kptr(CallableData* p)
{
	if(p)
	{
		p->IncRef();
		if(flag==-1)
		{
			data.pval->DecRef();
		}
		else
		{
			flag=-1;
		}
		data.pval=p;
		return;
	}
	else if(flag==-1)
	{
		data.pval->DecRef();
	}

	flag=0;
	data.pval=NULL;

}

EW_FORCEINLINE void Variant::clear()
{
	if(flag==-1) data.pval->DecRef();
	data.pval=NULL;
	flag=0;
}

template<>
class hash_t<Variant>
{
public:
	inline uint32_t operator()(const Variant& o)
	{
		switch(o.type())
		{
		case 0: return 0;
		case 1:return hash_t<bool>()(variant_handler<bool>::raw(o));
		case 2:return hash_t<int64_t>()(variant_handler<int64_t>::raw(o));
		case 3:return hash_t<double>()(variant_handler<double>::raw(o));
		default: return o.kptr()?o.kptr()->hashcode():0;
		}
	}
};

template<template<unsigned> class P,typename T>
class lookup_table_4bit
{
public:
	static T cmap[16];

	static inline T test(unsigned text)
	{
		EW_ASSERT(text<16);
		return cmap[text];
	}
};

template<template<unsigned> class P,typename T>
class lookup_table_8bit
{
public:
	static inline T test(unsigned text)
	{
		EW_ASSERT(text<256);
		return cmap[text];
	}

	static T cmap[256];
};

template<template<unsigned> class P,typename T>
T lookup_table_4bit<P,T>::cmap[16]={
	P<0x00>::value,P<0x01>::value,P<0x02>::value,P<0x03>::value,
	P<0x04>::value,P<0x05>::value,P<0x06>::value,P<0x07>::value,
	P<0x08>::value,P<0x09>::value,P<0x0A>::value,P<0x0B>::value,
	P<0x0C>::value,P<0x0D>::value,P<0x0E>::value,P<0x0F>::value
};

template<template<unsigned> class P,typename T>
T lookup_table_8bit<P,T>::cmap[256]={
	P<0x00>::value,P<0x01>::value,P<0x02>::value,P<0x03>::value,
	P<0x04>::value,P<0x05>::value,P<0x06>::value,P<0x07>::value,
	P<0x08>::value,P<0x09>::value,P<0x0A>::value,P<0x0B>::value,
	P<0x0C>::value,P<0x0D>::value,P<0x0E>::value,P<0x0F>::value,
	P<0x10>::value,P<0x11>::value,P<0x12>::value,P<0x13>::value,
	P<0x14>::value,P<0x15>::value,P<0x16>::value,P<0x17>::value,
	P<0x18>::value,P<0x19>::value,P<0x1A>::value,P<0x1B>::value,
	P<0x1C>::value,P<0x1D>::value,P<0x1E>::value,P<0x1F>::value,
	P<0x20>::value,P<0x21>::value,P<0x22>::value,P<0x23>::value,
	P<0x24>::value,P<0x25>::value,P<0x26>::value,P<0x27>::value,
	P<0x28>::value,P<0x29>::value,P<0x2A>::value,P<0x2B>::value,
	P<0x2C>::value,P<0x2D>::value,P<0x2E>::value,P<0x2F>::value,
	P<0x30>::value,P<0x31>::value,P<0x32>::value,P<0x33>::value,
	P<0x34>::value,P<0x35>::value,P<0x36>::value,P<0x37>::value,
	P<0x38>::value,P<0x39>::value,P<0x3A>::value,P<0x3B>::value,
	P<0x3C>::value,P<0x3D>::value,P<0x3E>::value,P<0x3F>::value,
	P<0x40>::value,P<0x41>::value,P<0x42>::value,P<0x43>::value,
	P<0x44>::value,P<0x45>::value,P<0x46>::value,P<0x47>::value,
	P<0x48>::value,P<0x49>::value,P<0x4A>::value,P<0x4B>::value,
	P<0x4C>::value,P<0x4D>::value,P<0x4E>::value,P<0x4F>::value,
	P<0x50>::value,P<0x51>::value,P<0x52>::value,P<0x53>::value,
	P<0x54>::value,P<0x55>::value,P<0x56>::value,P<0x57>::value,
	P<0x58>::value,P<0x59>::value,P<0x5A>::value,P<0x5B>::value,
	P<0x5C>::value,P<0x5D>::value,P<0x5E>::value,P<0x5F>::value,
	P<0x60>::value,P<0x61>::value,P<0x62>::value,P<0x63>::value,
	P<0x64>::value,P<0x65>::value,P<0x66>::value,P<0x67>::value,
	P<0x68>::value,P<0x69>::value,P<0x6A>::value,P<0x6B>::value,
	P<0x6C>::value,P<0x6D>::value,P<0x6E>::value,P<0x6F>::value,
	P<0x70>::value,P<0x71>::value,P<0x72>::value,P<0x73>::value,
	P<0x74>::value,P<0x75>::value,P<0x76>::value,P<0x77>::value,
	P<0x78>::value,P<0x79>::value,P<0x7A>::value,P<0x7B>::value,
	P<0x7C>::value,P<0x7D>::value,P<0x7E>::value,P<0x7F>::value,
	P<0x80>::value,P<0x81>::value,P<0x82>::value,P<0x83>::value,
	P<0x84>::value,P<0x85>::value,P<0x86>::value,P<0x87>::value,
	P<0x88>::value,P<0x89>::value,P<0x8A>::value,P<0x8B>::value,
	P<0x8C>::value,P<0x8D>::value,P<0x8E>::value,P<0x8F>::value,
	P<0x90>::value,P<0x91>::value,P<0x92>::value,P<0x93>::value,
	P<0x94>::value,P<0x95>::value,P<0x96>::value,P<0x97>::value,
	P<0x98>::value,P<0x99>::value,P<0x9A>::value,P<0x9B>::value,
	P<0x9C>::value,P<0x9D>::value,P<0x9E>::value,P<0x9F>::value,
	P<0xA0>::value,P<0xA1>::value,P<0xA2>::value,P<0xA3>::value,
	P<0xA4>::value,P<0xA5>::value,P<0xA6>::value,P<0xA7>::value,
	P<0xA8>::value,P<0xA9>::value,P<0xAA>::value,P<0xAB>::value,
	P<0xAC>::value,P<0xAD>::value,P<0xAE>::value,P<0xAF>::value,
	P<0xB0>::value,P<0xB1>::value,P<0xB2>::value,P<0xB3>::value,
	P<0xB4>::value,P<0xB5>::value,P<0xB6>::value,P<0xB7>::value,
	P<0xB8>::value,P<0xB9>::value,P<0xBA>::value,P<0xBB>::value,
	P<0xBC>::value,P<0xBD>::value,P<0xBE>::value,P<0xBF>::value,
	P<0xC0>::value,P<0xC1>::value,P<0xC2>::value,P<0xC3>::value,
	P<0xC4>::value,P<0xC5>::value,P<0xC6>::value,P<0xC7>::value,
	P<0xC8>::value,P<0xC9>::value,P<0xCA>::value,P<0xCB>::value,
	P<0xCC>::value,P<0xCD>::value,P<0xCE>::value,P<0xCF>::value,
	P<0xD0>::value,P<0xD1>::value,P<0xD2>::value,P<0xD3>::value,
	P<0xD4>::value,P<0xD5>::value,P<0xD6>::value,P<0xD7>::value,
	P<0xD8>::value,P<0xD9>::value,P<0xDA>::value,P<0xDB>::value,
	P<0xDC>::value,P<0xDD>::value,P<0xDE>::value,P<0xDF>::value,
	P<0xE0>::value,P<0xE1>::value,P<0xE2>::value,P<0xE3>::value,
	P<0xE4>::value,P<0xE5>::value,P<0xE6>::value,P<0xE7>::value,
	P<0xE8>::value,P<0xE9>::value,P<0xEA>::value,P<0xEB>::value,
	P<0xEC>::value,P<0xED>::value,P<0xEE>::value,P<0xEF>::value,
	P<0xF0>::value,P<0xF1>::value,P<0xF2>::value,P<0xF3>::value,
	P<0xF4>::value,P<0xF5>::value,P<0xF6>::value,P<0xF7>::value,
	P<0xF8>::value,P<0xF9>::value,P<0xFA>::value,P<0xFB>::value,
	P<0xFC>::value,P<0xFD>::value,P<0xFE>::value,P<0xFF>::value
};


EW_LEAVE

#endif
