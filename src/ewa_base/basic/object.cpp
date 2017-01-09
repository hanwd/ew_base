#include "ewa_base/basic/object.h"

EW_ENTER

void Object::Serialize(Serializer&)
{

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
