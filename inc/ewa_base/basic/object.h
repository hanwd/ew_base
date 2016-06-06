#ifndef __H_EW_BASIC_OBJECT__
#define __H_EW_BASIC_OBJECT__


#include "ewa_base/basic/atomic.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/collection/indexer_map.h"

#define DECLARE_OBJECT_INFO(TYPE,INFO)	\
	public:\
	typedef INFO infobase;\
	typedef ObjectInfoT<TYPE,INFO> infotype;\
	static infotype sm_info;\
	virtual infobase& GetObjectInfo() const{return sm_info;}\
	virtual const String& GetObjectName() const {return sm_info.GetName();}

#define DECLARE_OBJECT_CACHED_INFO(TYPE,INFO)	\
	public:\
	typedef INFO infobase;\
	typedef ObjectInfoCachedT<TYPE,INFO> infotype;\
	static  infotype sm_info;\
	virtual infobase& GetObjectInfo() const{return sm_info;}\
	virtual const String& GetObjectName() const {return sm_info.GetName();}\
	virtual void Serialize(Serializer&){}\
	virtual TYPE* DoClone(ObjectCloneState&){ return this; }


#define DECLARE_OBJECT_INFO_BASE(TYPE,INFO,BASE)	\
	DECLARE_OBJECT_INFO(TYPE,INFO)\
	typedef BASE basetype;\

#define IMPLEMENT_OBJECT_INFO_NAME(TYPE,INFO,NAME)	\
	TYPE::infotype TYPE::sm_info(NAME);\

#define IMPLEMENT_OBJECT_INFO(TYPE,INFO)	IMPLEMENT_OBJECT_INFO_NAME(TYPE,INFO,#TYPE)

#define IMPLEMENT_OBJECT_INFO_NAME_T1(TYPE,INFO,NAME)	\
	template<typename T1> typename TYPE<T1>::infotype  TYPE<T1>::sm_info(ew::ObjectNameT<T1>::MakeName(NAME));

#define IMPLEMENT_OBJECT_INFO_NAME_N1(TYPE,INFO,NAME)	\
	template<int N1> typename TYPE<N1>::infotype  TYPE<N1>::sm_info(String::Format("%s#%d",NAME,N1));

#define IMPLEMENT_OBJECT_INFO_NAME_T2(TYPE,INFO,NAME)	\
	template<typename T1,typename T2> typename TYPE<T1,T2>::infotype TYPE<T1,T2>::sm_info(ew::ObjectNameT<T2>::MakeName(ew::ObjectNameT<T1>::MakeName(NAME)));


#define IMPLEMENT_OBJECT_INFO_T1(TYPE,INFO)	IMPLEMENT_OBJECT_INFO_NAME_T1(TYPE,INFO,#TYPE)
#define IMPLEMENT_OBJECT_INFO_T2(TYPE,INFO)	IMPLEMENT_OBJECT_INFO_NAME_T2(TYPE,INFO,#TYPE)

#define IMPLEMENT_OBJECT_INFO_N1(TYPE,INFO)	IMPLEMENT_OBJECT_INFO_NAME_N1(TYPE,INFO,#TYPE)

EW_ENTER

class DLLIMPEXP_EWA_BASE Object;
class DLLIMPEXP_EWA_BASE Serializer;

class DLLIMPEXP_EWA_BASE ObjectInfo : private NonCopyable
{
public:

	ObjectInfo(const String& s);

	const String& GetName() const;

	virtual Object* CreateObject()=0;

	virtual Object* GetCachedInstance();

	virtual Object* GetHelperObject(const String&);

	template<typename T>
	T* GetHelperObjectT(const String& name)
	{
		return dynamic_cast<T*>(GetHelperObject(name));
	}

protected:
	virtual ~ObjectInfo();
	String m_sClassName;
};



template<typename T,typename INFO=ObjectInfo>
class ObjectInfoT : public INFO
{
public:
	typedef INFO basetype;
	ObjectInfoT(const String &name):basetype(name) {}

	Object *CreateObject()
	{
		return new T();
	}
};

template<typename T,typename INFO=ObjectInfo>
class ObjectInfoCachedT : public INFO
{
public:
	typedef INFO basetype;

	ObjectInfoCachedT(const String &name) :basetype(name)
	{

	}

	T *CreateObject()
	{
		static DataPtrT<T> gInstance(new T);
		return gInstance.get();
	}

	virtual T* GetCachedInstance()
	{ 
		return CreateObject(); 
	}

};



class DLLIMPEXP_EWA_BASE Object

#ifdef EW_OBJECT_USE_MP_ALLOC
	: public mp_obj
#endif

{
public:

	virtual ~Object() {}
	virtual void Serialize(Serializer& ar);

	DECLARE_OBJECT_INFO(Object,ObjectInfo)
};

class DLLIMPEXP_EWA_BASE ObjectCloneState;

class DLLIMPEXP_EWA_BASE ObjectData : public Object
{
public:

	ObjectData(){}
	ObjectData(const ObjectData&){}

	ObjectData& operator=(const ObjectData&){return *this;}
	~ObjectData();

	// Increase reference counter,
	inline void IncRef(){m_refcount++;}

	// Decrease reference counter,
	inline void DecRef()
	{
		EW_ASSERT(m_refcount.get()>0);
		if(--m_refcount==0)
		{
			delete this;
		}
	}

	// Get reference count.
	inline int GetRef() const
	{
		return m_refcount.get();
	}

	virtual ObjectData* DoClone(ObjectCloneState&);


	ObjectData* Clone(int t=0);

	template<typename T>
	T* CloneT(int t=0)
	{
		ObjectData* pd=Clone(t);
		if(!pd) return NULL;
		T* pt=dynamic_cast<T*>(pd);
		if(pt) return pt;
		delete pd;
		return NULL;
	}


	// Increase reference count of p2, decrease reference count of p1
	static void locked_reset(ObjectData*&p1,ObjectData* p2)
	{
		if(p1==p2) return;
		if(p2) p2->IncRef();
		if(p1) p1->DecRef();
		p1=p2;
	}

	template<typename T>
	static typename tl::enable_if_c<tl::is_convertible<T,ObjectData>,void>::type locked_reset(T*& p1,ObjectData* p2)
	{
		locked_reset(reinterpret_cast<ObjectData*&>(p1),p2);
	}

	DECLARE_OBJECT_INFO(ObjectData,ObjectInfo)

protected:

	//virtual void on_destroy() {}
	//virtual void on_created() {}

	AtomicInt32 m_refcount;
};



class DLLIMPEXP_EWA_BASE ObjectCloneState
{
public:

	explicit ObjectCloneState(int t=0):type(t){}

	// if d is already cloned, return last cloned value else return d->Clone(*this)
	ObjectData* clone(ObjectData* d);
	
	// clear cloned state
	void clear(){aClonedState.clear();}

	int type;
private:
	indexer_map<const ObjectData*,ObjectData*> aClonedState;
};


template<typename T>
class FunctionCreator
{
public:

	static Object* Create()
	{
		return new T();
	}
};


// ObjectName is used to generate template objects' names.

DEFINE_OBJECT_NAME(String,"str");
DEFINE_OBJECT_NAME(bool,"i01");
DEFINE_OBJECT_NAME(char,"i08");
DEFINE_OBJECT_NAME(wchar_t,"wch");
DEFINE_OBJECT_NAME(int32_t,"i32");
DEFINE_OBJECT_NAME(int64_t,"i64");
DEFINE_OBJECT_NAME(uint32_t,"u32");
DEFINE_OBJECT_NAME(uint64_t,"u64");
DEFINE_OBJECT_NAME(float32_t,"f32");
DEFINE_OBJECT_NAME(float64_t,"f64");

EW_LEAVE

#endif
