#ifndef __H_EW_BASIC_OBJECT_INFO__
#define __H_EW_BASIC_OBJECT_INFO__


#include "ewa_base/basic/atomic.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/basic/dlink.h"
#include "ewa_base/basic/bitflags.h"
#include "ewa_base/collection/indexer_set.h"
#include "ewa_base/collection/arr_1t.h"


#define INVOKETARGET_DEFINE(X)\
class InvokeTarget_##X : public ew::ObjectInfo\
{\
public:\
	InvokeTarget_##X() :ObjectInfo(#X){}\
	void DoInvoke(ew::InvokeParam& ipm)\
				{\
		if (\
			ipm.type != ew::InvokeParam::TYPE_CALL_ARGV ||\
			ipm.argvs.size() < 1 ||\
			ipm.argvs[0] != #X)	return;\
			RealInvoke();\
				}\
	void RealInvoke();\
}invoke_target_##X;\
void InvokeTarget_##X::RealInvoke()

EW_ENTER


class DLLIMPEXP_EWA_BASE Object;
class DLLIMPEXP_EWA_BASE Serializer;
class DLLIMPEXP_EWA_BASE ObjectInfo;
class DLLIMPEXP_EWA_BASE InvokeParam;
class DLLIMPEXP_EWA_BASE CreatorUseInfo;

class DLLIMPEXP_EWA_BASE InvokeParam
{
public:
	enum
	{
		TYPE_NONE,
		TYPE_INIT,
		TYPE_FINI,
		TYPE_TRY_RELEASE_MEM,
		TYPE_CALL_ARGV,
	};


	explicit InvokeParam(int t = TYPE_NONE) :type(t){pobj=NULL;}
	virtual ~InvokeParam(){}

	int type;
	BitFlags flags;
	Object* pobj;
	arr_1t<String> argvs;
	indexer_set<ObjectInfo*> infos;


	virtual void OnInvoke(ObjectInfo*);


};

class DLLIMPEXP_EWA_BASE ObjectInfo : public DLinkNode, private NonCopyableAndNonNewable
{
public:


	static void Invoke(InvokeParam&);

	static void Invoke(int t);
	static void Invoke(const String& s);

	ObjectInfo(const String& s = "");

	const String& GetName() const;

	virtual Object* CreateObject();

	virtual Object* GetCachedInstance();

	virtual Object* GetHelperObject(const String&);

	virtual void DoInvoke(InvokeParam&);

	template<typename T>
	T* GetHelperObjectT(const String& name)
	{
		return dynamic_cast<T*>(GetHelperObject(name));
	}

protected:
	virtual ~ObjectInfo();
	String m_sClassName;

};


template<typename T, typename INFO = ObjectInfo>
class ObjectInfoT : public INFO
{
public:
	typedef INFO basetype;
	ObjectInfoT(const String &name) :basetype(name) {}

	Object *CreateObject()
	{
		return new T();
	}
};

EW_LEAVE

#endif
