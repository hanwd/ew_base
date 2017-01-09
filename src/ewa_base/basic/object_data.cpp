#include "ewa_base/basic/object.h"
#include "ewa_base/basic/object_ex.h"
#include "ewa_base/collection/arr_1t.h"
#include "ewa_base/serialization/serializer.h"

EW_ENTER



void ObjectData::on_init_counter() 
{
	m_counter=3;
}

void ObjectData::on_fini_counter() 
{
	m_counter=0;
	delete this;
}

ObjectData* ObjectData::Clone(int t)
{
	ObjectCloneState cs(t);
	return cs.clone(this);
}

ObjectData* ObjectData::DoClone(ObjectCloneState&)
{
	return this;
}


ObjectData::~ObjectData()
{
	if(m_counter!=0)
	{
		System::LogTrace("m_counter!=0 while ObjectData destruct");
	}
}


ObjectData* ObjectCloneState::clone(ObjectData* d)
{
	int n=aClonedState.find1(d);
	if(n>=0)
	{
		return aClonedState.get(n).second;
	}

	ObjectData* v=d->DoClone(*this);
	aClonedState[d]=v;

	return v;
}

void ObjectCloneState::clear(){aClonedState.clear();}

IMPLEMENT_OBJECT_INFO(ObjectData,ObjectInfo)



EW_LEAVE
