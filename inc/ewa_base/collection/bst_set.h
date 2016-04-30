#ifndef __H_EW_COLLECTION_BST_SET__
#define __H_EW_COLLECTION_BST_SET__


#include "ewa_base/collection/detail/bst_container.h"
#include <functional>

EW_ENTER



template<
	typename T,
	typename C=std::less<T>,
	typename A=def_allocator,
	template<typename,typename,typename> class P=rbt_trait // P can be rbt_trait or avl_trait
>
class bst_set : public bst_container<P<T,void,C>,A>
{
protected:

	typedef bst_container<P<T,void,C>,A> basetype;

	typedef typename basetype::impl_type impl_type;
	using basetype::impl;

public:
	typedef typename impl_type::key_compare key_compare;
	typedef typename impl_type::allocator_type allocator_type;

	inline bst_set() {}
	inline bst_set(const bst_set& o):basetype(o) {}
	inline explicit bst_set(const key_compare& kc,const A& al=A()):basetype(kc,al) {}

	inline bst_set& operator=(const bst_set& o)
	{
		impl=o.impl;
		return *this;
	}

#ifdef EW_C11
	inline bst_set(bst_set&& o)
	{
		this->swap(o);
	}
	inline bst_set& operator=(bst_set&& o)
	{
		this->swap(o);
		return *this;
	}
#endif

};


template<
	typename T,
	typename C=std::less<T>,
	typename A=def_allocator,
	template<typename,typename,typename> class P=rbt_trait // P can be rbt_trait or avl_trait
>
class bst_multiset
	: public bst_multi_container<P<T,void,C>,A>
{
protected:

	typedef bst_multi_container<P<T,void,C>,A> basetype;

	typedef typename basetype::impl_type impl_type;
	using basetype::impl;

public:
	typedef typename impl_type::key_compare key_compare;
	typedef typename impl_type::allocator_type allocator_type;

	inline bst_multiset() {}
	inline bst_multiset(const bst_multiset& o):basetype(o) {}
	inline bst_multiset(const key_compare& kc,const A& al=A()):basetype(kc,al) {}

	inline bst_multiset& operator=(const bst_multiset& o)
	{
		impl=o.impl;
		return *this;
	}

#ifdef EW_C11
	inline bst_multiset(bst_multiset&& o)
	{
		this->swap(o);
	}
	inline bst_multiset& operator=(bst_multiset&& o)
	{
		this->swap(o);
		return *this;
	}
#endif

};

EW_LEAVE

#endif
