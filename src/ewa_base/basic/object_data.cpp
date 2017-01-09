#include "ewa_base/basic/object.h"
#include "ewa_base/basic/object_ex.h"
#include "ewa_base/collection/arr_1t.h"
#include "ewa_base/serialization/serializer.h"

EW_ENTER



void ObjectData::on_destroy()
{
	delete this;
}

void ObjectData::on_created()
{

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
	if(m_refcount.get()!=0)
	{
		System::LogTrace("m_refcount==%d while ObjectData destruct",m_refcount.get());
	}
}



IMPLEMENT_OBJECT_INFO(ObjectData,ObjectInfo)



EW_LEAVE
