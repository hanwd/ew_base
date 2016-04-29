#include "ewa_base/basic/object.h"
#include "ewa_base/basic/object_ex.h"
#include "ewa_base/collection/arr_1t.h"
#include "ewa_base/serialization/serializer.h"

EW_ENTER



ObjectData::~ObjectData()
{
	if(m_refcount.get()!=0)
	{
		System::LogTrace("m_refcount==%d while ObjectData destruct",m_refcount.get());
	}
}


ObjectGroup::ObjectGroup()
{
	
}

ObjectGroup::ObjectGroup(const ObjectGroup& o):impl(o.impl)
{
	for(size_t i=0;i<impl.size();i++)
	{
		impl[i]->IncRef();
	}
}

ObjectGroup& ObjectGroup::operator=(const ObjectGroup& o)
{
	if(this==&o) return *this;
	ObjectGroup(o).swap(*this);
	return *this;
}

ObjectGroup::~ObjectGroup()
{
	clear();
}


ObjectGroup::value_proxy& ObjectGroup::operator[](size_t n)
{
	return __proxy()[n];
}
const ObjectGroup::value_proxy& ObjectGroup::operator[](size_t n) const
{
	return __proxy()[n];
}

ObjectGroup::value_proxy& ObjectGroup::back()
{
	return __proxy().back();
}
const ObjectGroup::value_proxy& ObjectGroup::back() const
{
	return __proxy().back();
}

size_t ObjectGroup::size() const
{
	return impl.size();
}

bool ObjectGroup::empty() const
{
	return impl.empty();
}

void ObjectGroup::append(ObjectData* d)
{
	if(!d) return;
	impl.push_back(d);
	d->IncRef();
}

void ObjectGroup::Serialize(Serializer& ar)
{
	if(ar.is_reader())
	{
		arr_1t<DataPtrT<ObjectData> > tmp;
		ar & tmp;
		impl.swap(*(impl_type*)&tmp);
	}
	else
	{
		ar & impl;
	}

}

void ObjectGroup::remove(ObjectData* d)
{
	for(size_t i=0;i<impl.size();i++)
	{
		if(impl[i]==d)
		{
			std::swap(impl[i],impl.back());
			impl.pop_back();
			d->DecRef();
			return;
		}
	}
}

void ObjectGroup::clear()
{
	for(size_t i=0;i<impl.size();i++)
	{
		impl[i]->DecRef();
	}
	impl.clear();
}

void ObjectGroup::swap(ObjectGroup& o)
{
	impl.swap(o.impl);
}

IMPLEMENT_OBJECT_INFO(ObjectData,ObjectInfo)

EW_LEAVE
