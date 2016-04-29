#ifndef __H_EW_COLLECTION_INDEXER_CONTAINER__
#define __H_EW_COLLECTION_INDEXER_CONTAINER__

#include "ewa_base/collection/detail/indexer_base.h"
#include "ewa_base/memory/mempool.h"
#include "ewa_base/memory/allocator.h"

EW_ENTER


template<typename P,typename A=def_allocator>
class indexer_container : public container_base
{
protected:
	typedef indexer_base<P,A> impl_type;
	mutable impl_type impl;

public:
	typedef typename impl_type::key_type key_type;
	typedef typename impl_type::value_proxy value_proxy;
	typedef typename impl_type::index_type index_type;
	typedef typename impl_type::size_type size_type;
	typedef typename impl_type::value_array value_array;
	typedef typename impl_type::value_type value_type;

	inline indexer_container() {}
	inline indexer_container(const indexer_container& o):impl(o.impl) {}

	typedef typename impl_type::value_array::iterator iterator;
	typedef typename impl_type::value_array::const_iterator const_iterator;
	typedef typename impl_type::value_array::reverse_iterator reverse_iterator;
	typedef typename impl_type::value_array::const_reverse_iterator const_reverse_iterator;

	inline iterator begin(){return impl.get_values().begin();}
	inline iterator end(){return impl.get_values().end();}
	inline reverse_iterator rbegin(){return impl.get_values().rbegin();}
	inline reverse_iterator rend(){return impl.get_values().rend();}

	inline const_iterator begin() const {return impl.get_values().begin();}
	inline const_iterator end() const {return impl.get_values().end();}
	inline const_reverse_iterator rbegin() const {return impl.get_values().rbegin();}
	inline const_reverse_iterator rend() const {return impl.get_values().rend();}

	inline const_iterator cbegin() const {return impl.get_values().begin();}
	inline const_iterator cend() const {return impl.get_values().end();}
	inline const_reverse_iterator crbegin() const {return impl.get_values().crbegin();}
	inline const_reverse_iterator crend() const {return impl.get_values().crend();}

	inline void swap(indexer_container& o){impl.swap(o.impl);}

	inline void swap_array(typename impl_type::proxy_array& o)
	{
		impl.get_proxys().swap(o);
		impl.rehash(0);
	}

	inline const_iterator find(const key_type& v) const
	{
		index_type n = impl.find1(v);
		if (n < 0) return end();
		return begin() + n;
	}

	inline iterator find(const key_type& v)
	{
		index_type n = impl.find1(v);
		if (n < 0) return end();
		return begin() + n;
	}

	inline index_type find1(const key_type& v) const
	{
		return impl.find1(v);
	}

	inline index_type find2(const key_type& v)
	{
		return impl.find2(v);
	}

	inline index_type insert(const value_proxy& v)
	{
		return impl.insert(v);
	}

#ifdef EW_C11
	inline index_type find2(key_type&& v)
	{
		return impl.find2(std::forward<key_type>(v));
	}

	inline index_type insert(value_proxy&& v)
	{
		return impl.insert(std::forward<value_proxy>(v));
	}

	inline indexer_container(indexer_container&& o)
	{
		impl.swap(o.impl);
	}

#endif


	template <class InputIterator>
	void insert (InputIterator first, InputIterator last)
	{
		while(first!=last) insert(*first++);
	}

	inline size_type erase(const key_type& v)
	{
		return impl.erase(v);
	}

	inline void clear()
	{
		impl.clear();
	}

	inline void rehash(size_t n)
	{
		impl.rehash(n);
	}

	inline void reserve(size_t n)
	{
		impl.reserve(n);
	}

	inline float load_factor() const
	{
		return impl.load_factor();
	}

	inline float max_load_factor() const
	{
		return impl.max_load_factor();
	}

	inline void max_load_factor(float z)
	{
		impl.max_load_factor(z);
	}

	inline value_type& get(index_type n)
	{
		return impl.get_by_id(n);
	}

	inline const value_type& get(index_type n) const
	{
		return impl.get_by_id(n);
	}

	inline bool empty() const
	{
		return impl.empty();
	}

	inline size_t size() const
	{
		return impl.size();
	}

};




EW_LEAVE
#endif

