#ifndef __H_EW_COLLECTION_ARR_1t__
#define __H_EW_COLLECTION_ARR_1t__


#include "ewa_base/collection/detail/collection_base.h"
#include "ewa_base/collection/detail/arr_container.h"


EW_ENTER


template<typename T,typename A=def_allocator>
class arr_1t : public containerB< arr_container<T,typename A::template rebind<T>::other,tl::is_pod<T>::value> >
{
protected:

	typedef containerB< arr_container<T,typename A::template rebind<T>::other,tl::is_pod<T>::value> > basetype;
	using basetype::impl;

public:

	typedef typename basetype::size_type size_type;
	typedef typename basetype::value_type value_type;
	typedef typename basetype::pointer pointer;
	typedef typename basetype::const_pointer const_pointer;
	typedef typename basetype::reference reference;
	typedef typename basetype::const_reference const_reference;
	typedef typename basetype::iterator iterator;
	typedef typename basetype::const_iterator const_iterator;
	typedef typename basetype::reverse_iterator reverse_iterator;
	typedef typename basetype::const_reverse_iterator const_reverse_iterator;

	using basetype::swap;

	inline arr_1t(){}
	inline arr_1t(const arr_1t& o):basetype(o){}
	inline explicit arr_1t(const A& al):basetype(al){}

	inline arr_1t& operator=(const arr_1t& o){impl=o.impl;return *this;}

#if defined(EW_C11)
	inline arr_1t(arr_1t&& p){swap(p);}
	inline arr_1t& operator=(arr_1t&& p){swap(p);return *this;}
#endif

	inline size_type capacity() const{return impl.capacity();}

	void reserve(size_type capacity_){impl.reserve(capacity_);}

	void resize(size_type newsize_,const T& value_=T()){impl.resize(newsize_,value_);}

	void assign(size_type count_,const T& val_)
	{
		impl.assign(count_,val_);
	}

	template<typename It>
	void assign(It first_,size_type count_){impl.assign(first_,count_);}

	template<typename It>
	void assign(It first_,It last_){impl.assign(first_,last_);}

	iterator insert(const_iterator where_,const T& val_)
	{
		return impl.insert(where_,val_);
	}

	template<typename It>
	inline iterator insert(const_iterator where_,It first_,size_type count_)
	{
		return impl.insert(where_,first_,count_);
	}

	template<typename It>
	inline iterator insert(const_iterator where_,It first_,It last_)
	{
		return impl.insert(where_,first_,last_);		
	}

	inline iterator append(const T& val_)
	{
		return impl.append(val_);
	}

	template<typename It>
	inline iterator append(It first_,size_type count_)
	{
		return impl.append(first_,count_);
	}

	template<typename It>
	inline iterator append(It first_,It last_)
	{
		return impl.append(first_,last_);
	}

	inline iterator erase(iterator position_)
	{
		return impl.erase(position_);
	}

	inline iterator erase(iterator first_,iterator last_)
	{
		return impl.erase(first_,last_);
	}

	void shrink_to_fit()
	{
		impl.shrink_to_fit();
	}

	inline reference front(){return impl.front();}
	inline reference back(){return impl.back();}

	inline const_reference front() const{return impl.front();}
	inline const_reference back() const{return impl.back();}

	inline void push_back(const T& val_){impl.append(val_);}

#if defined(EW_C11)
	inline void push_back(T&& val_){impl.append(std::forward<T>(val_));}
	inline iterator append(T&& val_){return impl.append(std::forward<T>(val_));}
#endif

	inline void pop_back(){impl.pop_back();}

	inline void pop_back(T& val){val=impl.back();impl.pop_back();}

	inline reference at (size_type n)
	{
		return impl.at(n);
	}

	inline const_reference at (size_type n) const
	{
		return impl.at(n);
	}

	inline pointer data()
	{
		return impl.data();
	}

	inline const_pointer data() const
	{
		return impl.data();
	}

	inline T& operator[](size_type n){return impl[n];}
	inline const T& operator[](size_type n) const {return impl[n];}

};

template<typename T,typename A1,typename A2>
bool operator==(const arr_1t<T,A1>& lhs,const arr_1t<T,A2>& rhs)
{
	return container_equal(lhs,rhs);
}
template<typename T,typename A1,typename A2>
bool operator!=(const arr_1t<T,A1>& lhs,const arr_1t<T,A2>& rhs)
{
	return !container_equal(lhs,rhs);
}

template<typename T,typename A> class hash_t<arr_1t<T,A> >
{
public:
	inline uint32_t operator()(const arr_1t<T,A>& o)
	{
		return hash_array<T>::hash(o.data(),o.size());
	}
};

EW_LEAVE
#endif
