
#ifndef __H_EW_COLLECTION_BST_TREE__
#define __H_EW_COLLECTION_BST_TREE__


#include "ewa_base/collection/detail/bst_tree_node.h"
#include "ewa_base/collection/detail/bst_tree_iterator.h"
#include "ewa_base/collection/detail/rbt_tree_policy.h"
#include "ewa_base/collection/detail/avl_tree_policy.h"

EW_ENTER


template<typename P,typename A>
class bst_tree : public containerK<typename P::key_compare,typename A::template rebind<typename P::node_type>::other >
{
public:
	typedef containerK<typename P::key_compare,typename A::template rebind<typename P::node_type>::other > basetype;

	typedef typename P::node_type node_type;
	typedef typename P::key_type key_type;
	typedef typename P::mapped_type mapped_type;
	typedef typename P::value_proxy value_proxy;
	typedef typename P::const_key_reference const_key_reference;

	typedef typename P::key_compare key_compare;
	typedef typename basetype::size_type size_type;
	typedef typename basetype::allocator_type allocator_type;


	typedef P bst_policy;

protected:
	node_type* m_pRoot;
	size_t m_nSize;

public:

	typedef bst_iterator<P,true,false> iterator;
	typedef bst_iterator<P,false,false> reverse_iterator;
	typedef bst_iterator<P,true,true> const_iterator;
	typedef bst_iterator<P,false,true> const_reverse_iterator;

	iterator begin()
	{
		return gen_iterator<iterator>(P::nd_min(m_pRoot));
	}
	iterator end()
	{
		return gen_iterator<iterator>(NULL);
	}
	reverse_iterator rbegin()
	{
		return gen_iterator<reverse_iterator>(P::nd_max(m_pRoot));
	}
	reverse_iterator rend()
	{
		return gen_iterator<reverse_iterator>(NULL);
	}


	bst_tree()
	{
		m_pRoot=NULL;
		m_nSize=0;
	}
	bst_tree(const key_compare& kc,const allocator_type& al):basetype(kc,al)
	{
		m_pRoot=NULL;
		m_nSize=0;
	}

	bst_tree(const bst_tree& o):basetype(o)
	{
		m_pRoot=bst_copy_recursive((node_type*)o.m_pRoot);
		m_nSize=o.m_nSize;
	}

	bst_tree& operator=(const bst_tree& o);

	~bst_tree()
	{
		clear();
	}

	void swap(bst_tree& o);
	void clear();

	inline size_type size() const
	{
		return m_nSize;
	}

	inline bool empty() const
	{
		return m_nSize==0;
	}


#ifdef EW_C11

	template<typename G>
	typename G::ret_type handle_key(key_type&& k)
	{
		G g;return handle_real<G,key_type,false>(g,std::forward<key_type>(k));
	}

	template<typename G>
	typename G::ret_type handle_value(value_proxy&& k)
	{
		G g;return handle_real<G,value_proxy,false>(g,std::forward<value_proxy>(k));
	}

	template<typename G>
	typename G::ret_type handle_multi_key(key_type&& k)
	{
		G g;return handle_real<G,key_type,true>(g,std::forward<key_type>(k));
	}

	template<typename G>
	typename G::ret_type handle_multi_value(value_proxy&& k)
	{
		G g;return handle_real<G,value_proxy,true>(g,std::fowrward<value_proxy>(k));
	}

	template<typename G,typename K,bool M>
	typename G::ret_type handle_real(G& g,K&& v);

#endif



	template<typename G>
	typename G::ret_type handle_key(const key_type& k)
	{
		G g;return handle_real<G,key_type,false>(g,k);
	}

	template<typename G>
	typename G::ret_type handle_value(const value_proxy& k)
	{
		G g;return handle_real<G,value_proxy,false>(g,k);
	}

	template<typename G>
	typename G::ret_type handle_multi_key(const key_type& k)
	{
		G g;return handle_real<G,key_type,true>(g,k);
	}

	template<typename G>
	typename G::ret_type handle_multi_value(const value_proxy& k)
	{
		G g;return handle_real<G,value_proxy,true>(g,k);
	}

	template<typename G,typename K,bool M>
	typename G::ret_type handle_real(G& g,const K& v);

	template<typename R>
	class fp_base
	{
	public:
		typedef R ret_type;
		static ret_type handle_empty(bst_tree& t,const_key_reference)
		{
			return R();
		}
		static ret_type handle_node1(bst_tree& t,const_key_reference,node_type*)
		{
			return R();
		}
		static ret_type handle_node2(bst_tree& t,const_key_reference,node_type*)
		{
			return R();
		}
		static ret_type handle_equal(bst_tree& t,const_key_reference,node_type*)
		{
			return R();
		}
		static ret_type handle_multi(bst_tree& t,const_key_reference,node_type*)
		{
			return R();
		}
	};


	class fp_base_iterator
	{
	public:
		typedef iterator ret_type;
		static ret_type handle_empty(bst_tree& t,const_key_reference)
		{
			return t.gen_iterator<iterator>(NULL);
		}
		static ret_type handle_node1(bst_tree& t,const_key_reference,node_type*)
		{
			return t.gen_iterator<iterator>(NULL);
		}
		static ret_type handle_node2(bst_tree& t,const_key_reference,node_type*)
		{
			return t.gen_iterator<iterator>(NULL);
		}
		static ret_type handle_equal(bst_tree& t,const_key_reference,node_type*)
		{
			return t.gen_iterator<iterator>(NULL);
		}
		static ret_type handle_multi(bst_tree& t,const_key_reference,node_type*)
		{
			return t.gen_iterator<iterator>(NULL);
		}
	};

	class fp_return_iterator : public fp_base_iterator
	{
		public:
		typedef iterator ret_type;
		static ret_type handle_equal(bst_tree& t,const_key_reference,node_type* n)
		{
			return t.gen_iterator<ret_type>(n);
		}
		static ret_type handle_multi(bst_tree& t,const_key_reference,node_type* n)
		{
			return t.gen_iterator<ret_type>(n);
		}
	};

	class fp_return_node : public fp_base<node_type*>
	{
		public:
		typedef node_type* ret_type;
		static ret_type handle_equal(bst_tree& t,const_key_reference,node_type* n)
		{
			return n;
		}
		static ret_type handle_multi(bst_tree& t,const_key_reference,node_type* n)
		{
			return n;
		}
	};

	class fp_lower_bound;
	class fp_upper_bound;
	class fp_equal_range;
	class fp_insert_node;
	class fp_insert_pair;
	class fp_erase;

	static bool check_node(node_type* n);

	void do_erase_swap_node0(node_type* n,node_type* x);
	void do_erase_swap_node1(node_type* n,node_type* x);
	void do_erase_swap_node2(node_type* n,node_type* x);
	void do_erase_swap_node(node_type* n,node_type* x);

	void do_erase(node_type* n);
	iterator erase(const_iterator position);
	iterator erase(const_iterator p1,const_iterator p2);

	template<typename ITERATOR>
	ITERATOR gen_iterator(node_type* p)
	{
		return ITERATOR(p,&m_pRoot);
	}

	bool bst_validate();

protected:
	// new_root should be old_root->child1 or old_root->child2;
	void rotate(node_type* old_root,node_type* new_root);
	void rotate_right(node_type* node);
	void rotate_left(node_type* node);

#ifdef EW_C11
	template<typename X>
	node_type* bst_construct(X&& o);

	template<typename X>
	node_type* bst_construct(X& o);
#endif

	template<typename X>
	node_type* bst_construct(const X& o);

	node_type* bst_copy_recursive(node_type* r);

	void bst_destroy(node_type* n);
	void bst_destroy_recursive(node_type* n);

	size_t depth() const
	{
		return depth_real(m_pRoot);
	}

	size_t depth_real(node_type* n)
	{
		if(n==NULL) return 0;
		return 1+std::max(depth_real(n->child1),depth_real(n->child2));
	}

	static node_type* bst_sibling(node_type* n);

	static void set_child1(node_type* node,node_type* child)
	{
		node->child1=child;
		if(child) child->parent=node;
	}

	static void set_child2(node_type* node,node_type* child)
	{
		node->child2=child;
		if(child) child->parent=node;
	}
};

template<typename P,typename A>
bst_tree<P,A>& bst_tree<P,A>::operator=(const bst_tree& o)
{
	if(this==&o) return *this;
	bst_tree(o).swap(*this);
	return *this;
}

#ifdef EW_C11
template<typename P,typename A>
template<typename X>
typename bst_tree<P,A>::node_type* bst_tree<P,A>::bst_construct(X&& o)
{
	node_type* p=get_allocator().allocate(1);
	try
	{
		this->get_allocator().construct(p,kv_trait<typename P::key_type,typename P::mapped_type>::pair(std::forward<X>(o)));
	}
	catch(...)
	{
		this->get_allocator().deallocate(p,1);
		throw;
	}
	return p;
}

template<typename P,typename A>
template<typename X>
typename bst_tree<P,A>::node_type* bst_tree<P,A>::bst_construct(X& o)
{
	node_type* p=get_allocator().allocate(1);
	try
	{
		this->get_allocator().construct(p,kv_trait<typename P::key_type,typename P::mapped_type>::pair(o));
	}
	catch(...)
	{
		this->get_allocator().deallocate(p,1);
		throw;
	}
	return p;
}
#endif

template<typename P,typename A>
template<typename X>
typename bst_tree<P,A>::node_type* bst_tree<P,A>::bst_construct(const X& o)
{
	node_type* p=this->get_allocator().allocate(1);
	try
	{
		this->get_allocator().construct(p,kv_trait<typename P::key_type,typename P::mapped_type>::pair(o));
	}
	catch(...)
	{
		this->get_allocator().deallocate(p,1);
		throw;
	}
	return p;
}

template<typename P,typename A>
void bst_tree<P,A>::swap(bst_tree& o)
{
	std::swap(m_pRoot,o.m_pRoot);
	std::swap(m_nSize,o.m_nSize);
	this->_swap_allocator(o);
}

template<typename P,typename A>
void bst_tree<P,A>::clear()
{
	bst_destroy_recursive(m_pRoot);
	m_pRoot=NULL;
	m_nSize=0;
}

template<typename P,typename A>
inline void bst_tree<P,A>::bst_destroy(node_type* n)
{
	this->get_allocator().destroy(n);
	this->get_allocator().deallocate(n,1);
}

template<typename P,typename A>
void bst_tree<P,A>::bst_destroy_recursive(node_type* n)
{
	if(!n) return;
	bst_destroy_recursive(n->child1);
	bst_destroy_recursive(n->child2);
	bst_destroy(n);
}

template<typename P,typename A>
template<typename G,typename K,bool M>
inline typename G::ret_type bst_tree<P,A>::handle_real(G& g,const K& v)
{
	if(m_pRoot==NULL)
	{
		return g.handle_empty(*this,v);
	}

	node_type* node=m_pRoot;
	for(;;)
	{
		if(this->key_comp()(P::key(v),P::key(node)))
		{
			if(node->child1==NULL)
			{
				return g.handle_node1(*this,v,node);
			}
			else
			{
				node=node->child1;
			}
		}
		else if(this->key_comp()(P::key(node),P::key(v)))
		{
			if(node->child2==NULL)
			{
				return g.handle_node2(*this,v,node);
			}
			else
			{
				node=node->child2;
			}
		}
		else
		{
			if(M)
			{
				return g.handle_multi(*this,v,node);
			}
			else
			{
				return g.handle_equal(*this,v,node);
			}
		}
	}
}

#ifdef EW_C11

template<typename P,typename A>
template<typename G,typename K,bool M>
inline typename G::ret_type bst_tree<P,A>::handle_real(G& g,K&& v)
{
	if(m_pRoot==NULL)
	{
		return g.handle_empty(*this,std::forward<K>(v));
	}

	node_type* node=m_pRoot;
	for(;;)
	{
		if(this->key_comp()(P::key(v),P::key(node)))
		{
			if(node->child1==NULL)
			{
				return g.handle_node1(*this,std::forward<K>(v),node);
			}
			else
			{
				node=node->child1;
			}
		}
		else if(this->key_comp()(P::key(node),P::key(v)))
		{
			if(node->child2==NULL)
			{
				return g.handle_node2(*this,std::forward<K>(v),node);
			}
			else
			{
				node=node->child2;
			}
		}
		else
		{
			if(M)
			{
				return g.handle_multi(*this,std::forward<K>(v),node);
			}
			else
			{
				return g.handle_equal(*this,std::forward<K>(v),node);
			}
		}
	}
}

#endif

template<typename P,typename A>
typename bst_tree<P,A>::node_type* bst_tree<P,A>::bst_sibling(node_type* n)
{
	EW_ASSERT(n!=NULL&&n->parent!=NULL);

	node_type* p=n->parent;
	if(n==p->child1)
	{
		return p->child2;
	}
	else
	{
		return p->child1;
	}
}

template<typename P,typename A>
class bst_tree<P,A>::fp_erase : public fp_base<size_type>
{
public:
	typedef size_type ret_type;
	typedef const key_type& const_key_reference;

	static ret_type handle_equal(bst_tree& t,const_key_reference,node_type* n)
	{
		t.do_erase(n);
		return 1;
	}
	static ret_type handle_multi(bst_tree& t,const_key_reference v,node_type* n)
	{
		ret_type num=1;
		while(1)
		{
			node_type* x=P::nd_inc(n);
			if(x==NULL||t.key_comp()(v,P::key(x)))
			{
				break;
			}
			++num;
			t.do_erase(x);
		}

		while(1)
		{
			node_type* x=P::nd_dec(n);
			if(x==NULL||t.key_comp()(P::key(x),v))
			{
				break;
			}
			++num;
			t.do_erase(x);
		}

		t.do_erase(n);
		return num;
	}

};

template<typename P,typename A>
class bst_tree<P,A>::fp_lower_bound : public fp_base_iterator
{
public:
	typedef iterator ret_type;
	static ret_type handle_node1(bst_tree& t,const_key_reference,node_type* n)
	{
		return t.gen_iterator<iterator>(n);
	}
	static ret_type handle_node2(bst_tree& t,const_key_reference,node_type* n)
	{
		return ++t.gen_iterator<iterator>(n);
	}
	static ret_type handle_equal(bst_tree& t,const_key_reference,node_type* n)
	{
		return t.gen_iterator<iterator>(n);
	}

	static ret_type handle_multi(bst_tree& t,const_key_reference v,node_type* n)
	{
		if(!n->child1) return t.gen_iterator<iterator>(n);
		for(;;)
		{
			if(t.key_comp()(P::key(n),v))
			{
				if(n->child2)
				{
					n=n->child2;
				}
				else
				{
					return ++t.gen_iterator<iterator>(n);
				}
			}
			else
			{
				if(!n->child1)
				{
					return t.gen_iterator<iterator>(n);
				}
				else
				{
					n=n->child1;
				}
			}
		}
	}


};

template<typename P,typename A>
class bst_tree<P,A>::fp_upper_bound : public fp_base_iterator
{
public:
	typedef typename fp_base_iterator::ret_type ret_type;
	static ret_type handle_node1(bst_tree& t,const_key_reference,node_type* n)
	{
		return t.gen_iterator<iterator>(n);
	}
	static ret_type handle_node2(bst_tree& t,const_key_reference,node_type* n)
	{
		return ++t.gen_iterator<iterator>(n);
	}
	static ret_type handle_equal(bst_tree& t,const_key_reference,node_type* n)
	{
		return ++t.gen_iterator<iterator>(n);
	}
	static ret_type handle_multi(bst_tree& t,const_key_reference v,node_type* n)
	{
		if(!n->child2) return ++t.gen_iterator<iterator>(n);
		for(;;)
		{
			if(t.key_comp()(v,P::key(n)))
			{
				if(n->child1)
				{
					n=n->child1;
				}
				else
				{
					return t.gen_iterator<iterator>(n);
				}
			}
			else
			{
				if(!n->child2)
				{
					return ++t.gen_iterator<iterator>(n);
				}
				else
				{
					n=n->child2;
				}
			}
		}
	}
};

template<typename P,typename A>
class bst_tree<P,A>::fp_equal_range
{
public:
	typedef std::pair<iterator,iterator> ret_type;
	static ret_type handle_empty(bst_tree& t,const_key_reference)
	{
		iterator it=t.gen_iterator<iterator>(NULL);
		return ret_type(it,it);
	}
	static ret_type handle_node1(bst_tree& t,const_key_reference,node_type* n)
	{
		iterator it=t.gen_iterator<iterator>(n);
		return ret_type(it,it);
	}
	static ret_type handle_node2(bst_tree& t,const_key_reference,node_type* n)
	{
		iterator it=++t.gen_iterator<iterator>(n);
		return ret_type(it,it);
	}
	static ret_type handle_equal(bst_tree& t,const_key_reference,node_type* n)
	{
		iterator it=t.gen_iterator<iterator>(n);
		return ret_type(it,++iterator(it));
	}
	static ret_type handle_multi(bst_tree& t,const_key_reference v,node_type* n)
	{
		return ret_type(fp_lower_bound::handle_multi(t,v,n),fp_upper_bound::handle_multi(t,v,n));
	}
};



template<typename P,typename A>
class bst_tree<P,A>::fp_insert_pair
{
public:
	typedef std::pair<iterator,bool> ret_type;

	ret_type adjust_insert(bst_tree& t,node_type* n)
	{
		t.m_nSize++;
		P::adjust_insert(t,n);
		return ret_type(t.gen_iterator<iterator>(n),true);
	}

	ret_type handle_empty(bst_tree& t,const value_proxy& v)
	{
		node_type* n=t.bst_construct(v);
		return adjust_insert(t,n);
	}

	ret_type handle_node1(bst_tree& t,const value_proxy& v,node_type* p)
	{
		node_type* n=t.bst_construct(v);
		set_child1(p,n);
		return adjust_insert(t,n);
	}

	ret_type handle_node2(bst_tree& t,const value_proxy& v,node_type* p)
	{
		node_type* n=t.bst_construct(v);
		set_child2(p,n);
		return adjust_insert(t,n);
	}

	ret_type handle_equal(bst_tree& t,const value_proxy&,node_type* n)
	{
		return ret_type(t.gen_iterator<iterator>(n),false);
	}

	ret_type handle_multi(bst_tree& t,const value_proxy& v,node_type* n)
	{
		for(;;)
		{
			if(t.key_comp()(P::key(v),P::key(n)))
			{
				if(n->child1==NULL)
				{
					return handle_node1(t,v,n);
				}
				else
				{
					n=n->child1;
				}
			}
			else
			{
				if(n->child2==NULL)
				{
					return handle_node2(t,v,n);
				}
				else
				{
					n=n->child2;
				}
			}
		}
	}

#ifdef EW_C11

	ret_type handle_empty(bst_tree& t,value_proxy&& v)
	{
		node_type* n=t.bst_construct(std::forward<value_proxy>(v));
		return adjust_insert(t,n);
	}

	ret_type handle_node1(bst_tree& t,value_proxy&& v,node_type* p)
	{
		node_type* n=t.bst_construct(std::forward<value_proxy>(v));
		set_child1(p,n);
		return adjust_insert(t,n);
	}

	ret_type handle_node2(bst_tree& t,value_proxy&& v,node_type* p)
	{
		node_type* n=t.bst_construct(std::forward<value_proxy>(v));
		set_child2(p,n);
		return adjust_insert(t,n);
	}

	ret_type handle_equal(bst_tree& t,value_proxy&&,node_type* n)
	{
		return ret_type(t.gen_iterator<iterator>(n),false);
	}

	ret_type handle_multi(bst_tree& t,value_proxy&& v,node_type* n)
	{
		for(;;)
		{
			if(t.key_comp()(P::key(v),P::key(n)))
			{
				if(n->child1==NULL)
				{
					return handle_node1(t,std::forward<value_proxy>(v),n);
				}
				else
				{
					n=n->child1;
				}
			}
			else
			{
				if(n->child2==NULL)
				{
					return handle_node2(t,std::forward<value_proxy>(v),n);
				}
				else
				{
					n=n->child2;
				}
			}
		}
	}
#endif

};



template<typename P,typename A>
class bst_tree<P,A>::fp_insert_node
{
public:
	typedef node_type* ret_type;

	ret_type handle_empty(bst_tree& t,const_key_reference v)
	{
		node_type* n=t.bst_construct(v);
		return adjust_insert(t,n);
	}

	ret_type handle_node1(bst_tree& t,const_key_reference v,node_type* p)
	{
		node_type* n=t.bst_construct(v);
		set_child1(p,n);
		return adjust_insert(t,n);
	}

	ret_type handle_node2(bst_tree& t,const_key_reference v,node_type* p)
	{
		node_type* n=t.bst_construct(v);
		set_child2(p,n);
		return adjust_insert(t,n);
	}

	ret_type adjust_insert(bst_tree& t,node_type* n)
	{
		P::adjust_insert(t,n);
		t.m_nSize++;
		return n;
	}

	ret_type handle_equal(bst_tree&,const_key_reference,node_type* n)
	{
		return n;
	}

	ret_type handle_multi(bst_tree&,const_key_reference,node_type* n)
	{
		return n;
	}

};


template<typename P,typename A>
void bst_tree<P,A>::rotate(node_type* old_root,node_type* new_root)
{
	if(new_root==old_root->child1)
	{
		rotate_right(old_root);
	}
	else
	{
		rotate_left(old_root);
	}
}

template<typename P,typename A>
void bst_tree<P,A>::rotate_right(node_type* node)
{

	EW_ASSERT(node->child1!=NULL);

	node_type* oldr=node->parent;
	node_type* newr=node->child1;
	set_child1(node,newr->child2);
	set_child2(newr,node);

	if(oldr)
	{
		if(node==oldr->child1)
		{
			set_child1(oldr,newr);
		}
		else
		{
			set_child2(oldr,newr);
		}
	}
	else
	{
		m_pRoot=newr;
		newr->parent=NULL;
	}
}

template<typename P,typename A>
void bst_tree<P,A>::rotate_left(node_type* node)
{
	EW_ASSERT(node->child2!=NULL);

	node_type* oldr=node->parent;
	node_type* newr=node->child2;
	set_child2(node,newr->child1);
	set_child1(newr,node);

	if(oldr)
	{
		if(node==oldr->child1)
		{
			set_child1(oldr,newr);
		}
		else
		{
			set_child2(oldr,newr);
		}
	}
	else
	{
		m_pRoot=newr;
		newr->parent=NULL;
	}
}

template<typename P,typename A>
void bst_tree<P,A>::do_erase_swap_node(node_type* n,node_type* x)
{
	std::swap(n->extra,x->extra);
	if(n->child2==x)
	{
		do_erase_swap_node2(n,x);
	}
	else if(n->child1==x)
	{
		do_erase_swap_node1(n,x);
	}
	else
	{
		do_erase_swap_node0(n,x);
	}
}

template<typename P,typename A>
void bst_tree<P,A>::do_erase_swap_node0(node_type* n,node_type* x)
{
	node_type* p=n->parent;
	if(p)
	{
		if(p->child1==n)
		{
			p->child1=x;
		}
		else
		{
			p->child2=x;
		}
	}
	else
	{
		m_pRoot=x;
	}

	p=x->parent;
	if(p->child1==x)
	{
		p->child1=n;
	}
	else
	{
		p->child2=n;
	}

	n->child1->parent=x;
	n->child2->parent=x;
	if(x->child1) x->child1->parent=n;
	if(x->child2) x->child2->parent=n;

	std::swap(n->parent,x->parent);
	std::swap(n->child1,x->child1);
	std::swap(n->child2,x->child2);

	EW_ASSERT(check_node(n));
	EW_ASSERT(check_node(x));
}

template<typename P,typename A>
void bst_tree<P,A>::do_erase_swap_node1(node_type* n,node_type* x)
{
	EW_ASSERT(n->child1==x);
	EW_ASSERT(n->child2!=NULL);
	EW_ASSERT(x->child2==NULL);

	// parent
	node_type* p=n->parent;
	x->parent=p;
	if(p)
	{
		if(n==p->child1) p->child1=x;
		else p->child2=x;
	}
	else
	{
		m_pRoot=x;
	}

	// xchild2
	x->child2=n->child2;
	x->child2->parent=x;
	n->child2=NULL;

	// child1
	n->child1=x->child1;
	if(n->child1) n->child1->parent=n;
	x->child1=n;
	n->parent=x;

	EW_ASSERT(check_node(n));
	EW_ASSERT(check_node(x));
}

template<typename P,typename A>
void bst_tree<P,A>::do_erase_swap_node2(node_type* n,node_type* x)
{
	EW_ASSERT(n->child2==x);
	EW_ASSERT(n->child1!=NULL);
	EW_ASSERT(x->child1==NULL);

	// parent
	node_type* p=n->parent;
	x->parent=p;
	if(p)
	{
		if(n==p->child1) p->child1=x;
		else p->child2=x;
	}
	else
	{
		m_pRoot=x;
	}

	// xchild2
	x->child1=n->child1;
	x->child1->parent=x;
	n->child1=NULL;

	// child1
	n->child2=x->child2;
	if(n->child2) n->child2->parent=n;
	x->child2=n;
	n->parent=x;

	EW_ASSERT(check_node(n));
	EW_ASSERT(check_node(x));

}

template<typename P,typename A>
bool bst_tree<P,A>::bst_validate()
{
	return P::bst_validate(*this);
}

template<typename P,typename A>
void bst_tree<P,A>::do_erase(node_type* n)
{
	if(n->child1!=NULL&&n->child2!=NULL)
	{
		//node_type* x=P::nd_min(n->child2);
		node_type* x=P::nd_max(n->child1);
		do_erase_swap_node(n,x);
	}

	P::delete_one_child(*this,n);
	m_nSize--;
}

template<typename P,typename A>
typename bst_tree<P,A>::iterator bst_tree<P,A>::erase(const_iterator position)
{
	EW_ASSERT(position.root()==m_pRoot);
	node_type* n=position.node();
	if(n==NULL)
	{
		System::LogError("%s failed! INVALID iterator!","bst_tree<...>::erase");
		return end();
	}
	node_type* x=P::nd_inc(n);
	do_erase(n);

	return gen_iterator<iterator>(x);
}

template<typename P,typename A>
typename bst_tree<P,A>::iterator bst_tree<P,A>::erase(const_iterator p1,const_iterator p2)
{
	EW_ASSERT(p1.root()==m_pRoot);
	EW_ASSERT(p2.root()==m_pRoot);

	while(p1!=p2)
	{
		node_type* n=p1.node();
		if(n==NULL)
		{
			System::LogError("%s failed! INVALID iterator!","bst_tree<...>::erase");
			return end();
		}

		++p1;
		do_erase(n);
	}
	return gen_iterator<iterator>(p2.node());
}


template<typename P,typename A>
bool bst_tree<P,A>::check_node(node_type* n)
{
	if(n->child1 && n->child1->parent!=n)
	{
		return false;
	}
	if(n->child2 && n->child2->parent!=n)
	{
		return false;
	}
	if(n->parent && n->parent->child1!=n && n->parent->child2!=n)
	{
		return false;
	}
	return true;
}


template<typename P,typename A>
typename bst_tree<P,A>::node_type* bst_tree<P,A>::bst_copy_recursive(node_type* r)
{
	if(!r) return NULL;

	node_type* p=bst_construct(r->value);

	try
	{
		p->extra=r->extra;

		p->child1=bst_copy_recursive(r->child1);
		if(p->child1) p->child1->parent=p;

		p->child2=bst_copy_recursive(r->child2);
		if(p->child2) p->child2->parent=p;
	}
	catch(std::bad_alloc&)
	{
		bst_destroy_recursive(p);
		throw;
	}

	return p;
}



EW_LEAVE

#endif
