
#include "ewa_base/memory/gc_ptr.h"
#include "ewa_base/logging/logger.h"

EW_ENTER

uint32_t gc_state::nNumberOfObject=0;
uint32_t gc_state::nNumberOfObjectLast=0;

AtomicMutex gc_state::tRecursiveMutex;
gc_obj* gc_state::pLinkOfAllObject=NULL;

uint32_t gc_state::nStepingDelta=1024*128;
gc_state::addrinfo_type gc_state::tCurrentObject;

BitFlags gc_state::flags;

gc_state* gc_state::_gp_instance=NULL;


gc_state::gc_state()
{
	tPtrLink_head._gc_next=&tPtrLink_tail;
	tPtrLink_head._gc_prev=NULL;
	tPtrLink_tail._gc_next=NULL;
	tPtrLink_tail._gc_prev=&tPtrLink_head;
}

void gc_state::_ensure_created()
{
	static gc_state gInstance;
	_gp_instance=&gInstance;
}


void gc_state::gc_nolock(bool force_marking_)
{
	ElapsedTimer tt;
	tt.tick();

	uint32_t p1=nNumberOfObject;

	if(force_marking_)
	{
		for(gc_obj* p=pLinkOfAllObject;p;p=p->_gc_next)
		{
			p->_gc_tags=GC_TAG_FLAG;
		}
	}
	
	pending_q1.reset();
	for(gc_dlinkptr_type* p=tPtrLink_head._gc_next;p!=&tPtrLink_tail;p=p->_gc_next)
	{
		gc_obj* o=p->m_ptr;
		if(o)
		{
			o->_gc_tags=GC_TAG_MARK;
			o->_gc_mark(pending_q1);
		}
	}

	try
	{
		while(!pending_q1.empty())
		{
			pending_q2.reset();
			for(gc_mark_queue::iterator it=pending_q1.begin();it!=pending_q1.end();++it)
			{
				(*it)->_gc_mark(pending_q2);
			}
			pending_q1.swap(pending_q2);
		}
	}
	catch(...)
	{
		System::LogError("error in gc_state::gc_nolock");
		return;
	}
	
	gc_obj* p=gc_state::pLinkOfAllObject;
	gc_obj* _pLinkOfConnectedObjects=NULL;

	gc_state::pLinkOfAllObject=NULL;

	while(p)
	{
		gc_obj* x=p;
		p=p->_gc_next;;

		if(x->_gc_tags==GC_TAG_MARK)
		{
			x->_gc_tags=GC_TAG_FLAG;
			x->_gc_next=_pLinkOfConnectedObjects;
			_pLinkOfConnectedObjects=x;
			continue;
		}

		gc_state::destroy(x);
	}

	gc_state::pLinkOfAllObject=_pLinkOfConnectedObjects;
	int p2=gc_state::nNumberOfObject;
	tt.tack();

	gc_state::nNumberOfObjectLast=gc_state::nNumberOfObject;

	if(gc_state::flags.get(gc_state::FLAG_SHOW_GC_INFO))
	{
		this_logger().LogMessage("gc: %d -> %d objects, %g sec used",p1,p2,tt.get());
	}

}

void gc_force_collect()
{
	gc_state::current().gc(true);
}

EW_LEAVE
