#ifndef __H_EW_BASIC_DLINK__
#define __H_EW_BASIC_DLINK__

#include "ewa_base/config.h"


EW_ENTER

class DLLIMPEXP_EWA_BASE DLink;

class DLLIMPEXP_EWA_BASE DLinkNode : private NonCopyable
{
public:
	friend class DLink;

	DLinkNode():_p_link_next(NULL),_p_link_prev(NULL){}

protected:
	DLinkNode* _p_link_next;
	DLinkNode* _p_link_prev;
};

class DLLIMPEXP_EWA_BASE DLink : protected DLinkNode
{
public:
	DLink():m_sz(0){_p_link_next=_p_link_prev=this;}
	DLink(const DLink&):m_sz(0){_p_link_next=_p_link_prev=this;}
	DLink& operator=(const DLink&){return *this;}

	bool empty() const {return _p_link_next==this;}
	size_t size() const {return m_sz;}

	void clear();

protected:

	
	void _do_link_next(DLinkNode* p);
	void _do_link_prev(DLinkNode* p);

	void _do_unlink(DLinkNode* p);
	void _do_unlink(DLinkNode* p1,DLinkNode* p2);

	static DLinkNode* _get_next(DLinkNode* p){return p->_p_link_next;}
	static DLinkNode* _get_prev(DLinkNode* p){return p->_p_link_prev;}

	size_t m_sz;
};

class DLinkPolicyDirectCast
{
public:
	template<typename T>
	static DLinkNode* cast_n(T* t){return static_cast<DLinkNode*>(t);}

	template<typename T>
	static T* cast_t(DLinkNode* p){return static_cast<T*>(p);}
};

template<int N>
class DLinkPolicyShiftCast
{
public:
	template<typename T>
	static DLinkNode* cast_n(T* t){return (DLinkNode*)(((char*)t)+N);}

	template<typename T>
	static T* cast_t(DLinkNode* p){return (T*)(((char*)p)-N);}
};

template<typename Y>
class DLinkPolicyBaseCast
{
public:
	template<typename T>
	static DLinkNode* cast_n(T* t){return static_cast<Y*>(t);}

	template<typename T>
	static T* cast_t(DLinkNode* p){return static_cast<T*>(static_cast<Y*>(p));}
};



template<typename T,typename P=DLinkPolicyDirectCast>
class DLinkT : public DLink
{
public:

	void LinkNext(T* p)
	{
		DLink::_do_link_next(P::cast_n(p));
	}

	void LinkPrev(T* p)
	{
		DLink::_do_link_prev(P::cast_n(p));
	}

	void UnLink(T* p)
	{
		DLink::_do_unlink(P::cast_n(p));
	}



	template<bool D,bool C>
	class iterator_t
	{
	public:
		iterator_t(DLinkNode* p=NULL,DLinkNode* r=NULL):node(p),link(r){}

		DLinkNode* node;
		DLinkNode* link;

		iterator_t& operator++()
		{
			EW_ASSERT(link!=node);
			node=D?DLink::_get_next(node):DLink::_get_prev(node);
			return *this;
		}
		iterator_t operator++(int)
		{
			EW_ASSERT(link!=node);
			iterator it(*this);
			node=D?DLink::_get_next(node):DLink::_get_prev(node);
			return it;
		}
		
		typename tl::meta_if<C,const T*,T*>::type operator*()
		{
			EW_ASSERT(link!=node);
			return P::cast_t<T>(node);
		}

		bool operator==(const iterator_t& it){return node==it.node;}
		bool operator!=(const iterator_t& it){return node!=it.node;}
	};


	template<bool D>
	void UnLink(iterator_t<D,0> p1,iterator_t<D,0> p2)
	{
		EW_ASSERT(p1.link==this);
		EW_ASSERT(p2.link==this);
		D? DLink::_do_unlink(p1.node,p2.node) : DLink::_do_unlink(DLink::_get_next(p2.node),DLink::_get_prev(p1.node)); 
	}


	typedef iterator_t<1,0> iterator;
	typedef iterator_t<0,0> reverse_iterator;
	typedef iterator_t<1,1> const_iterator;
	typedef iterator_t<0,1> const_reverse_iterator;

	iterator begin(){return iterator(_p_link_next,this);}
	iterator end(){return iterator(this,this);}
	reverse_iterator rbegin(){return reverse_iterator(_p_link_next,this);}
	reverse_iterator rend(){return reverse_iterator(this,this);}	
	const_iterator cbegin(){return const_iterator(_p_link_next,this);}
	const_iterator cend(){return const_iterator(this,this);}
	const_reverse_iterator crbegin(){return const_reverse_iterator(_p_link_next,this);}
	const_reverse_iterator crend(){return const_reverse_iterator(this,this);}


};



EW_LEAVE

#endif
