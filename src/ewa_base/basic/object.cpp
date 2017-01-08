#include "ewa_base/basic/object.h"

EW_ENTER

void Object::Serialize(Serializer&)
{

}

void ObjectData::on_fini_counter() 
{
	m_counter=0;
	delete this;
}

void ObjectData::on_init_counter() 
{
	m_counter=3;
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


IMPLEMENT_OBJECT_INFO(Object,ObjectInfo)

EW_LEAVE
