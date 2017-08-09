#ifndef __H_EW_BASIC_OBJECT__
#define __H_EW_BASIC_OBJECT__

#include "ewa_base/collection/indexer_map.h"
#include "ewa_base/basic/object_info.h"

#define DECLARE_OBJECT_INFO(TYPE,INFO)	\
	public:\
	typedef INFO infobase;\
	typedef ObjectInfoT<TYPE,INFO> infotype;\
	static infotype sm_info;\
	virtual infobase& GetObjectInfo() const{return sm_info;}\
	virtual const String& GetObjectName() const {return sm_info.GetName();}

#define DECLARE_OBJECT_INFO_NO_CREATE(TYPE,INFO)	\
	public:\
	typedef INFO infobase;\
	typedef INFO infotype;\
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
	virtual void Serialize(SerializerHelper){}\
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

class DLLIMPEXP_EWA_BASE Serializer;
class DLLIMPEXP_EWA_BASE SerializerHelper;
class DLLIMPEXP_EWA_BASE ObjectCloneState;

class DLLIMPEXP_EWA_BASE Object	: public mp_obj
{
public:

	virtual ~Object() {}
	virtual void Serialize(SerializerHelper sh);

	DECLARE_OBJECT_INFO(Object,ObjectInfo)
};



class DLLIMPEXP_EWA_BASE ObjectData : public Object
{
public:

	EW_FORCEINLINE ObjectData():m_counter(0){}
	EW_FORCEINLINE ObjectData(const ObjectData&):m_counter(0){}
	EW_FORCEINLINE ObjectData& operator=(const ObjectData&){return *this;}

	~ObjectData();

	// Increase reference counter,
	EW_FORCEINLINE void IncRef()
	{
		if(m_counter)
		{
			volatile uintptr_t* pcounter_addr=m_counter&0x1?&m_counter:(volatile uintptr_t*)m_counter;
			AtomicOps::fetch_add(pcounter_addr,(uintptr_t)2);
			//*pcounter_addr+=2;
		}
		else
		{
			on_init_counter();
		}
	}

	// Decrease reference counter,
	EW_FORCEINLINE void DecRef()
	{
		EW_ASSERT(m_counter!=0);

		volatile uintptr_t* pcounter_addr=m_counter&0x1?&m_counter:(volatile uintptr_t*)m_counter;
		if(*pcounter_addr==3)
		{
			on_fini_counter();
		}
		else
		{
			AtomicOps::fetch_sub(pcounter_addr,(uintptr_t)2);
			//*pcounter_addr-=2;
		}
	}

	// Get reference count.
	EW_FORCEINLINE int GetRef() const
	{
		return (m_counter&0x1?m_counter:*(volatile uintptr_t*)m_counter)>>1;
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
		pd->IncRef();
		pd->DecRef();
		return NULL;
	}


	// Increase reference count of p2, decrease reference count of p1
	template<typename T>
	static typename tl::enable_if_c<tl::is_convertible<T,ObjectData> >::type locked_reset(T*& p1,T* p2)
	{
		if(p1==p2) return;
		if(p2) p2->IncRef();
		if(p1) p1->DecRef();
		p1=p2;
	}

	DECLARE_OBJECT_INFO(ObjectData,ObjectInfo)

protected:

	virtual void on_fini_counter();
	virtual void on_init_counter();

	volatile uintptr_t m_counter;
};


class DLLIMPEXP_EWA_BASE ObjectCloneState
{
public:

	explicit inline ObjectCloneState(int t=0):type(t){}

	// if d is already cloned, return last cloned value else return d->Clone(*this)
	ObjectData* clone(ObjectData* d);

	// clear cloned state
	void clear();

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

DEFINE_OBJECT_NAME(dcomplex, "complex");
DEFINE_OBJECT_NAME(fcomplex, "complex32");

EW_LEAVE

#endif
