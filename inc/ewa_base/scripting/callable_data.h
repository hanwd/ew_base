#ifndef __H_EW_SCRIPTING_VARIANT_DATA__
#define __H_EW_SCRIPTING_VARIANT_DATA__

#include "ewa_base/basic/object.h"
#include "ewa_base/math/tiny_cpx.h"
#include "ewa_base/collection/arr_xt.h"
#include "ewa_base/scripting/detail/idx_1t.h"
#include <complex>

EW_ENTER


class DLLIMPEXP_EWA_BASE CG_Variable;

class DLLIMPEXP_EWA_BASE HelpData;
class DLLIMPEXP_EWA_BASE CallableData;
class DLLIMPEXP_EWA_BASE CallableWrap;
template<typename T> class DLLIMPEXP_EWA_BASE CallableWrapT;

class DLLIMPEXP_EWA_BASE CallableObject;
class DLLIMPEXP_EWA_BASE CallableMetatable;
class DLLIMPEXP_EWA_BASE CallableClass;
class DLLIMPEXP_EWA_BASE CallableFunction;
class DLLIMPEXP_EWA_BASE CallableClosure;
class DLLIMPEXP_EWA_BASE CallableCoroutine;
class DLLIMPEXP_EWA_BASE CallableCode;
class DLLIMPEXP_EWA_BASE CallableModule;

class DLLIMPEXP_EWA_BASE Variant;
class DLLIMPEXP_EWA_BASE Executor;
class DLLIMPEXP_EWA_BASE VariantTable;
class DLLIMPEXP_EWA_BASE VariantCloneState;


template<typename T> class hash_t<std::complex<T> > : public hash_pod<std::complex<T> > {};

template<typename X,typename Y> class hash_t<std::pair<X,Y> >
{
public:
	uint32_t operator()(const std::pair<X,Y> & o)
	{
		hash_t<X> h1;hash_t<Y> h2;
		return h1(o.first)-h2(o.second);
	}
};


typedef tl::mk_typelist<
	CallableData*,
	bool,
	int64_t,
	double,
	dcomplex,
	String,
	VariantTable,
	arr_xt<int64_t>,
	arr_xt<double>,
	arr_xt<dcomplex>,
	arr_xt<Variant>
>::type TL_VARIANT_TYPES;


template<typename T>
class flag_type_helper
{
public:
	typedef T type;
	typedef T scalar;

	static const bool is_arr=false;
	static const bool is_scr=true;
};

template<>
class flag_type_helper<tl::nulltype>
{
public:
	typedef CallableData* type;
	typedef CallableData* scalar;

	static const bool is_arr=false;
	static const bool is_scr=true;
};

template<typename T>
class flag_type_helper<arr_xt<T> >
{
public:
	typedef arr_xt<T> type;
	typedef T scalar;

	static const bool is_arr=true;
	static const bool is_scr=false;

	static inline arr_xt_dims size_ptr(const type& v){return v.size_ptr();}
	static inline void size_chk(const type& v){}
};


template<int N>
struct flag_type : public flag_type_helper<typename TL_VARIANT_TYPES::template at<N>::type>
{
	static const int value=N>0?N:0;
};

template<typename T> 
struct type_flag : public flag_type<TL_VARIANT_TYPES::template id<T>::value>
{

};


class DLLIMPEXP_EWA_BASE CallableData : public ObjectData
{
public:
	typedef ObjectData basetype;

	enum
	{
		INVALID_CALL	=-2,
		STACK_BALANCED	=-1,
	};

	CallableData(int t=15):m_nType(t){}

	inline int type() const {return m_nType;}

	virtual int __getindex(Executor&,const String&); // top.s
	virtual int __setindex(Executor&,const String&); // top.s=val
	virtual int __getarray(Executor&,int);	// top[s]
	virtual int __setarray(Executor&,int);	// top[s]=val
	virtual int __fun_call(Executor&,int);	// top(...)
	virtual int __new_item(Executor&,int);	// new top(...)

	virtual int __getarray_index_range(Executor&,int);


	// return value, -1 : error, 0 : ok, 1 : ok but need to enlarge
	virtual int __update_idx(idx_1t&,intptr_t,intptr_t){return -1;}

	virtual bool __test_dims(arr_xt_dims&,int){return false;}

	// top=get_iterator(top)
	virtual void __get_iterator(Executor&,int);

	CallableData* DoClone(ObjectCloneState&){return NULL;}


	virtual uint32_t hashcode() const{hash_pod<void*> h;return h((void*)this);}

	virtual bool IsEqualTo(const CallableData* d) const {return this==d;}

	virtual HelpData* __get_helpdata(){ return NULL; }
	virtual void __set_helpdata(const String&){}

	virtual CallableClass* ToClass() {return NULL;}
	virtual CallableWrapT<VariantTable>* ToTable() {return NULL;}
	virtual CallableCoroutine* ToCoroutine(){return NULL;}
	virtual CallableMetatable* ToMetatable(){return NULL;}
	virtual CallableClosure* ToClosure(){return NULL;}
	virtual CallableFunction* ToFunction(){return NULL;}
	virtual CallableCode* ToCode(){return NULL;}

	virtual CallableModule* ToModule(){return NULL;}

	virtual CallableMetatable* GetMetaTable(){return NULL;}

	virtual bool ToValue(String& s,int n=0) const;
	virtual bool ToValue(int64_t&) const;
	virtual bool ToValue(bool&) const;
	virtual bool ToValue(double&) const;

	bool ToValue(int32_t& v) const;
	bool ToValue(float& v) const;
	bool ToValue(size_t& v) const;

	inline CallableData* Clone(int t=0)
	{
		basetype*p=basetype::Clone(t);
		return static_cast<CallableData*>(p);
	}


	DECLARE_OBJECT_INFO(CallableData,ObjectInfo);

protected:
	const int m_nType;
};


class DLLIMPEXP_EWA_BASE CallableObject : public CallableData
{
public:
	CallableData* DoClone(ObjectCloneState&) {return this;}
};


EW_LEAVE
#endif
