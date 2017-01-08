#ifndef __H_EW_BASIC_DLINK__
#define __H_EW_BASIC_DLINK__

#include "ewa_base/config.h"


EW_ENTER


class DLinkNode
{
public:
	DLinkNode():next(NULL),prev(NULL){}
	DLinkNode(const DLinkNode&):next(NULL),prev(NULL){}
	DLinkNode& operator=(const DLinkNode&){return *this;}
	~DLinkNode(){}

	DLinkNode* next;
	DLinkNode* prev;

};

class DLink : private DLinkNode
{
public:
	DLink():m_sz(0){next=prev=this;}
	DLink(const DLink&):m_sz(0){next=prev=this;}
	DLink& operator=(const DLink&){return *this;}

	bool empty() const {return next==this;}
	size_t size() const {return m_sz;}

	void clear();

protected:

	
	void _do_link_next(DLinkNode* p);
	void _do_link_prev(DLinkNode* p);

	void _do_unlink(DLinkNode* p);
	void _do_unlink(DLinkNode* p1,DLinkNode* p2);

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

	template<bool D,bool C>
	class iterator_t
	{
	public:
		iterator_t(DLinkNode* p,DLink* r):node(p),link(r){}
		iterator_t(){}
		LitePtrT<DLinkNode> node,link;
		iterator_t& operator++()
		{
			EW_ASSERT(link!=node);
			node=D?node->next:node->prev;
			return *this;
		}
		iterator_t operator++(int)
		{
			EW_ASSERT(link!=node);
			iterator it(*this);
			node=D?node->next:node->prev;
			return it;
		}
		
		typename tl::meta_if<C,const T*,T*>::type operator*()
		{
			EW_ASSERT(link!=node);
			return *P::cast_t(node.get());
		}

		bool operator==(const iterator_t& it){return node==it.node;}
		bool operator!=(const iterator_t& it){return node!=it.node;}
	};

	template<bool D>
	void UnLink2(iterator_t<D,0> p1,iterator_t<D,0> p2)
	{
		EW_ASSERT(p1.link==this);
		EW_ASSERT(p2.link==this);
		D? DLink::_do_unlink(p1.node,p2.node) : DLink::_do_unlink(p2.node->next,p1.node->prev); 
	}


	typedef iterator_t<1,0> iterator;
	typedef iterator_t<0,0> reverse_iterator;
	typedef iterator_t<1,1> const_iterator;
	typedef iterator_t<0,1> const_reverse_iterator;

	iterator begin(){return iterator(next,this);}
	iterator end(){return iterator(this,this);}
	reverse_iterator rbegin(){return reverse_iterator(next,this);}
	reverse_iterator rend(){return reverse_iterator(this,this);}	
	const_iterator cbegin(){return const_iterator(next,this);}
	const_iterator cend(){return const_iterator(this,this);}
	const_reverse_iterator crbegin(){return const_reverse_iterator(next,this);}
	const_reverse_iterator crend(){return const_reverse_iterator(this,this);}


};



EW_LEAVE

#endif
