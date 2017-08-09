#include "ewa_base/basic/object.h"
#include "ewa_base/serialization/serializer.h"

EW_ENTER

void Object::Serialize(SerializerHelper sh)
{
	sh.ref(0);
}





IMPLEMENT_OBJECT_INFO(Object,ObjectInfo)

EW_LEAVE
