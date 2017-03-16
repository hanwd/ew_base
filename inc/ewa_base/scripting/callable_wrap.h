#ifndef __H_EW_SCRIPTING_VARIANT_WRAP__
#define __H_EW_SCRIPTING_VARIANT_WRAP__

#include "ewa_base/scripting/callable_data.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/stream.h"
#include "ewa_base/basic/clock.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE CallableWrap : public CallableData
{
public:
	CallableWrap(int t=0):CallableData(t){}

	virtual CallableMetatable* GetMetaTable();
};

template<typename T>
class CallableDataBaseT : public CallableWrap
{
public:

	typedef T type;

	CallableDataBaseT():CallableWrap(type_flag<type>::value){}
	CallableDataBaseT(const T& v):CallableWrap(type_flag<type>::value),value(v){}

#ifdef EW_C11
	CallableDataBaseT(T&& v):CallableWrap(type_flag<type>::value),value(std::forward<T>(v)){}
#endif

	type value;

	virtual int __update_idx(idx_1t& id,intptr_t n1,intptr_t n2)
	{
		return id.update(value,n1,n2);
	}

	virtual uint32_t hashcode() const
	{
		hash_t<type> h;return h(value);
	}

	bool IsEqualTo(const CallableData* d) const
	{
		const CallableDataBaseT<T>* p=dynamic_cast<const CallableDataBaseT<T>*>(d);
		return p&&p->value==value;
	}

	void Serialize(SerializerHelper sh)
	{
		Serializer& ar(sh.ref(0));
		ar & value;
	}

};


template<typename T>
class DLLIMPEXP_EWA_BASE CallableWrapT : public CallableDataBaseT<T>
{
public:
	typedef CallableDataBaseT<T> basetype;
	using basetype::value;

	CallableWrapT():basetype(){}
	CallableWrapT(const T& v):basetype(v){}

	virtual CallableData* DoClone(ObjectCloneState&){return new CallableWrapT(value);}

#ifdef EW_C11
	CallableWrapT(type&& v):basetype(std::move(v)){}
#endif

	DECLARE_OBJECT_INFO(CallableWrapT,ObjectInfo);
};


template<>
class DLLIMPEXP_EWA_BASE CallableWrapT<dcomplex> : public CallableDataBaseT<dcomplex>
{
public:
	typedef CallableDataBaseT<dcomplex> basetype;
	using basetype::value;

	CallableWrapT(){}
	CallableWrapT(const dcomplex& v):basetype(v){}

	virtual int __getindex(Executor&,const String&);
	virtual int __setindex(Executor&,const String&);

	virtual CallableData* DoClone(ObjectCloneState&){return new CallableWrapT(value);}

	DECLARE_OBJECT_INFO(CallableWrapT,ObjectInfo);

};


template<>
class DLLIMPEXP_EWA_BASE CallableWrapT<String> : public CallableDataBaseT<String>
{
public:
	typedef CallableDataBaseT<String> basetype;
	using basetype::value;

	CallableWrapT():basetype(){}
	CallableWrapT(const String& v):basetype(v){}

#ifdef EW_C11
	CallableWrapT(type&& v):basetype(std::forward<type>(v)){}
#endif

	virtual bool ToValue(String& v,int) const{v=value;return true;}

	virtual CallableData* DoClone(ObjectCloneState&){return new CallableWrapT(value);}

	virtual int __getarray(Executor& ewsl,int pm);
	virtual int __setarray(Executor& ewsl,int pm);
	virtual int __getarray_index_range(Executor& ewsl,int pm);
	virtual bool __test_dims(arr_xt_dims& dm, int op);

	DECLARE_OBJECT_INFO(CallableWrapT,ObjectInfo);
};

template<>
class DLLIMPEXP_EWA_BASE CallableWrapT<TimePoint> : public CallableDataBaseT<TimePoint>
{
public:
	typedef CallableDataBaseT<TimePoint> basetype;
	using basetype::value;

	CallableWrapT();
	CallableWrapT(const TimePoint& v):basetype(v){}

	virtual bool ToValue(String& v,int) const;
	virtual bool ToValue(int64_t& v) const;
	virtual bool ToValue(double& v) const;

	virtual int __getindex(Executor&,const String&);

	virtual CallableData* DoClone(ObjectCloneState&){return new CallableWrapT(value);}

	virtual CallableMetatable* GetMetaTable();

	DECLARE_OBJECT_INFO(CallableWrapT,ObjectInfo);

};

template<>
class DLLIMPEXP_EWA_BASE CallableWrapT<TimeSpan> : public CallableDataBaseT<TimeSpan>
{
public:
	typedef CallableDataBaseT<TimeSpan> basetype;
	using basetype::value;

	CallableWrapT():basetype(){}
	CallableWrapT(const TimeSpan& v):basetype(v){}

	virtual bool ToValue(String& v,int) const;
	virtual bool ToValue(int64_t& v) const;
	virtual bool ToValue(double& v) const;

	virtual CallableMetatable* GetMetaTable();
	virtual CallableData* DoClone(ObjectCloneState&){return new CallableWrapT(value);}

	DECLARE_OBJECT_INFO(CallableWrapT,ObjectInfo);
};



template<>
class DLLIMPEXP_EWA_BASE CallableWrapT<Stream> : public CallableDataBaseT<Stream>
{
public:
	typedef CallableDataBaseT<Stream> basetype;
	using basetype::value;

	CallableWrapT():basetype(){}
	CallableWrapT(const Stream& v):basetype(v){}

	virtual CallableMetatable* GetMetaTable();

	virtual CallableData* DoClone(ObjectCloneState&){return this;}

	StringBuffer<char> buffer;

	bool getline(String& val);

	DECLARE_OBJECT_INFO(CallableWrapT,ObjectInfo);

};

template<>
class DLLIMPEXP_EWA_BASE CallableWrapT<VariantTable>;

template<typename T>
class DLLIMPEXP_EWA_BASE CallableWrapT<arr_xt<T> >;


template<typename T>
class DLLIMPEXP_EWA_BASE CallableWrapT<StringBuffer<T> >;

EW_LEAVE
#endif
