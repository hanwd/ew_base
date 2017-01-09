#include "ewa_base/basic/dlink.h"

EW_ENTER

void DLink::clear()
{
	if(_p_link_next==this) return;
	DLinkNode* p1=_p_link_next;
	DLinkNode* p2=_p_link_prev;
	_p_link_next=_p_link_prev=this;
	p1->_p_link_prev=NULL;
	p2->_p_link_next=NULL;
	m_sz=0;
}

void DLink::_do_link_next(DLinkNode* p)
{
	EW_ASSERT(p->_p_link_prev==NULL && p->_p_link_next==NULL);
	p->_p_link_next=_p_link_next;
	p->_p_link_prev=this;
	_p_link_next->_p_link_prev=p;
	_p_link_next=p;
	m_sz++;
}

void DLink::_do_link_prev(DLinkNode* p)
{
	EW_ASSERT(p->_p_link_prev==NULL && p->_p_link_next==NULL);
	p->_p_link_next=this;
	p->_p_link_prev=_p_link_prev;
	_p_link_prev->_p_link_next=p;
	_p_link_prev=p;
	m_sz++;
}

void DLink::_do_unlink(DLinkNode* p)
{
	if(p->_p_link_prev) p->_p_link_prev->_p_link_next=p->_p_link_next;
	if(p->_p_link_next) p->_p_link_next->_p_link_prev=p->_p_link_prev;
	p->_p_link_prev=p->_p_link_next=NULL;
	m_sz--;
}

void DLink::_do_unlink(DLinkNode* p1,DLinkNode* p2)
{
	DLinkNode* p0=p1->_p_link_prev;
	DLinkNode* p3=p2->_p_link_prev;

	p1->_p_link_prev=NULL;
	p3->_p_link_next=NULL;

	p0->_p_link_next=p2;
	p2->_p_link_prev=p0;

	for(;p1;p1=p1->_p_link_next)
	{
		m_sz--;
	}
}


EW_LEAVE
