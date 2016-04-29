#ifndef __H_EW_COLLECTION_BST_MAP__
#define __H_EW_COLLECTION_BST_MAP__

#include "ewa_base/collection/detail/bst_container.h"

EW_ENTER

template<
	typename K,
	typename V,
	typename C=std::less<K>,
	typename A=def_allocator,
	template<typename,typename,typename> class P=rbt_trait // P can be rbt_trait or avl_trait
>
class bst_map :
	public bst_container<P<K,V,C>,A>
{
protected:

	typedef bst_container<P<K,V,C>,A> basetype;
	typedef typename basetype::impl_type impl_type;
	using basetype::impl;

public:

	typedef typename impl_type::key_type key_type;
	typedef typename impl_type::mapped_type mapped_type;
	typedef typename impl_type::key_compare key_compare;
	typedef typename impl_type::allocator_type allocator_type;

	inline bst_map() {}
	inline bst_map(const key_compare& kc,const A& al=A()):basetype(kc,al) {}

	inline bst_map(const bst_map& o):basetype(o) {}
	inline bst_map& operator=(const bst_map& o)
	{
		impl=o.impl;
		return *this;
	}

#ifdef EW_C11
	inline bst_map(bst_map&& o)
	{
		this->swap(o);
	}
	inline bst_map& operator=(bst_map&& o)
	{
		this->swap(o);
		return *this;
	}
#endif

	inline mapped_type& operator[](const key_type& k)
	{
		typename impl_type::node_type* n = impl.template handle_key<typename impl_type::fp_insert_node>(k);
		return n->value.second;
	}

	inline const mapped_type& operator[](const key_type& k) const
	{
		typename impl_type::node_type* n=impl.template handle_key<impl_type::fp_return_node>(k);
		if(!n) Exception::XNotFound();
		return n->value.second;
	}

};


template<
	typename K,
	typename V,
	typename C=std::less<K>,
	typename A=def_allocator,
	template<typename,typename,typename> class P=rbt_trait // P can be rbt_trait or avl_trait
>
class bst_multimap 
	: public bst_multi_container<P<K,V,C>,A>
{
protected:

	typedef bst_multi_container<P<K,V,C>,A> basetype;
	typedef typename basetype::impl_type impl_type;
	using basetype::impl;

public:

	typedef typename impl_type::key_type key_type;
	typedef typename impl_type::mapped_type mapped_type;
	typedef typename impl_type::key_compare key_compare;
	typedef typename impl_type::allocator_type allocator_type;

	inline bst_multimap() {}
	inline bst_multimap(const key_compare& kc,const A& al=A()):basetype(kc,al) {}

	inline bst_multimap(const bst_multimap& o):basetype(o) {}
	inline bst_multimap& operator=(const bst_multimap& o)
	{
		impl=o.impl;
		return *this;
	}

#ifdef EW_C11
	inline bst_multimap(bst_multimap&& o)
	{
		this->swap(o);
	}
	inline bst_multimap& operator=(bst_multimap&& o)
	{
		this->swap(o);
		return *this;
	}
#endif

};

EW_LEAVE

#endif
