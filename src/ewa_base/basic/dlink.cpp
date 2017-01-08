#include "ewa_base/basic/dlink.h"

EW_ENTER

void DLink::clear()
{
	if(next==this) return;
	DLinkNode* p1=next;
	DLinkNode* p2=prev;
	next=prev=this;
	p1->prev=NULL;
	p2->next=NULL;
	m_sz=0;
}

void DLink::_do_link_next(DLinkNode* p)
{
	EW_ASSERT(p->prev==NULL && p->next==NULL);
	p->next=next;
	p->prev=this;
	next->prev=p;
	next=p;
	m_sz++;
}

void DLink::_do_link_prev(DLinkNode* p)
{
	EW_ASSERT(p->prev==NULL && p->next==NULL);
	p->next=this;
	p->prev=prev;
	prev->next=p;
	prev=p;
	m_sz++;
}

void DLink::_do_unlink(DLinkNode* p)
{
	if(p->prev) p->prev->next=p->next;
	if(p->next) p->next->prev=p->prev;
	p->prev=p->next=NULL;
	m_sz--;
}

void DLink::_do_unlink(DLinkNode* p1,DLinkNode* p2)
{
	DLinkNode* p0=p1->prev;
	DLinkNode* p3=p2->prev;

	p1->prev=NULL;
	p3->next=NULL;

	p0->next=p2;
	p2->prev=p0;

	for(;p1;p1=p1->next)
	{
		m_sz--;
	}
}


EW_LEAVE
