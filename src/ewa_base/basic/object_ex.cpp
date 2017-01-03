
#include "ewa_base/basic/object.h"
#include "ewa_base/basic/object_ex.h"


EW_ENTER



TracedObject* traced_object_header = NULL;
AtomicSpin trace_object_mutex;

void TracedObject::DoInvoke(InvokeParam&){}

void TracedObject::Invoke(int t)
{
	InvokeParam ctx(t);
	Invoke(ctx);
}

void TracedObject::Invoke(InvokeParam& ctx)
{
	LockGuard<AtomicSpin> locker(trace_object_mutex);
	for (TracedObject* p = traced_object_header; p; p = p->m_pNext)
	{
		p->DoInvoke(ctx);
	}
}

void TracedObject::DoLinkObject(TracedObject* p, bool a)
{
	LockGuard<AtomicSpin> locker(trace_object_mutex);

	if (a)
	{
		p->m_pNext = traced_object_header;
		traced_object_header = p;
	}
	else if (traced_object_header == p)
	{
		traced_object_header = traced_object_header->m_pNext;

	}
	else if (traced_object_header)
	{
		for (TracedObject*prev = traced_object_header; TracedObject* next = prev->m_pNext; prev = next)
		{
			if (next != p) continue;
			prev->m_pNext = next->m_pNext;
			break;
		}
	}

}

TracedObject::TracedObject() :m_pNext(NULL)
{
	DoLinkObject(this, true);
}

TracedObject::TracedObject(const TracedObject&) : m_pNext(NULL)
{
	DoLinkObject(this, true);
}

TracedObject& TracedObject::operator=(const TracedObject&)
{
	return *this;
}

TracedObject::~TracedObject()
{
	DoLinkObject(this, false);
}


EW_LEAVE
