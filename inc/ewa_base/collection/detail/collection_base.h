
#ifndef __H_EW_COLLECTION_BASE__
#define __H_EW_COLLECTION_BASE__


#include "ewa_base/config.h"
#include "ewa_base/basic/exception.h"

//#include "ewa_base/basic/hashing.h"
//#include "ewa_base/memory/allocator.h"
//#include "ewa_base/memory/mempool.h"

#include <memory>
#include <algorithm>

EW_ENTER


class sz_helper
{
public:

	// adjust alignment
	static inline size_t adj(size_t sz,size_t al)
	{
		return (sz+al-1)&~(al-1);
	}

	// adjust alignment
	static inline void* adj(void* sz,size_t al)
	{
		return (void*)(((intptr_t)sz+al-1)&~(al-1));
	}

	// adjust size to 2^n
	static inline size_t n2p(size_t sz)
	{
		size_t mx=1+(size_t(-1)>>1);
		if(sz>mx)
		{
			Exception::XInvalidArgument();
		}

		size_t k=1;
		if(sz>=(k<<16)) k=k<<16;
		if(sz>=(k<< 8)) k=k<< 8;
		if(sz>=(k<< 4)) k=k<< 4;
		while(k<sz) k=k<<1;
		return k;
	}

	static inline size_t gen(size_t sz)
	{

		if(sz<4) return sz;
		size_t sz2=sz+(sz>>1);
		if(sz2>sz) return sz2;

		size_t mk=(1<<16)-1;
		sz2=(sz+mk)&~mk;
		if(sz2>sz) return sz2;
		return sz;
	}

};


class arr_xt_dims
{
public:

	typedef size_t size_type;
	static const size_type MAX_DIM=6;

	inline operator const size_type*() const {return dim;}
	inline operator size_type*() {return dim;}

	inline void resize(size_t n)
	{
		dim[0]=n;
		dim[1]=dim[2]=dim[3]=dim[4]=dim[5]=1;
	}


	inline void resize(size_type k0,size_type k1,size_type k2=1,size_type k3=1,size_type k4=1,size_type k5=1)
	{
		dim[0]=k0;
		dim[1]=k1;
		dim[2]=k2;
		dim[3]=k3;
		dim[4]=k4;
		dim[5]=k5;
	}

	inline arr_xt_dims(){resize(0);}
	inline arr_xt_dims(size_type k0,size_type k1=1,size_type k2=1,size_type k3=1,size_type k4=1,size_type k5=1)
	{
		resize(k0,k1,k2,k3,k4,k5);
	}

	inline size_t size() const{return dim[0]*dim[1]*dim[2]*dim[3]*dim[4]*dim[5];}
	inline size_t checked_size(size_t m) const
	{
		size_t n=size();
		if (n == 0) return 0;
		if(n>m||dim[0]!=n/dim[1]/dim[2]/dim[3]/dim[4]/dim[5])
		{
			Exception::XBadAlloc();
		}
		return n;
	}

	inline size_t operator()(size_type k0) const
	{
		return k0;
	}
	inline size_t operator()(size_type k0,size_type k1) const
	{
		return k0+dim[0]*k1;
	}
	inline size_t operator()(size_type k0,size_type k1,size_type k2) const
	{
		return k0+dim[0]*(k1+dim[1]*k2);
	}
	inline size_t operator()(size_type k0,size_type k1,size_type k2,size_type k3) const
	{
		return k0+dim[0]*(k1+dim[1]*(k2+dim[2]*k3));
	}
	inline size_t operator()(size_type k0,size_type k1,size_type k2,size_type k3,size_type k4) const
	{
		return k0+dim[0]*(k1+dim[1]*(k2+dim[2]*(k3+dim[3]*k4)));
	}
	inline size_t operator()(size_type k0,size_type k1,size_type k2,size_type k3,size_type k4,size_type k5) const
	{
		return k0+dim[0]*(k1+dim[1]*(k2+dim[2]*(k3+dim[3]*(k4+dim[4]*k5))));
	}

	const size_t* ptr() const {return dim;}

private:
	size_t dim[MAX_DIM];
};


class container_base
{
public:

};

template<typename A,bool V>
class alloc_base : public container_base
{
public:
	typedef A allocator_type;

	alloc_base() {}
	alloc_base(const alloc_base&) {}
	alloc_base(const allocator_type&) {}

	allocator_type& get_allocator() const
	{
		return *(allocator_type*)(NULL);
	}

	void _swap_allocator(alloc_base&){}
};

template<typename A>
class alloc_base<A,false> : public container_base
{
public:
	typedef A allocator_type;

	alloc_base() {}
	alloc_base(const alloc_base& o):_al(o._al) {}
	alloc_base(const allocator_type& al):_al(al) {}

	allocator_type& get_allocator() const
	{
		return _al;
	}

	void _swap_allocator(alloc_base& o)
	{
		if(_al==o._al) return;
	}

protected:
	mutable allocator_type _al;
};



template<typename K,bool V,typename A>
class keycomp_base : public alloc_base<A,tl::is_empty_type<A>::value>
{
public:
	typedef K key_compare;
	typedef alloc_base<A,tl::is_empty_type<A>::value> basetype;
	typedef typename basetype::allocator_type allocator_type;

	keycomp_base() {}
	keycomp_base(const keycomp_base& o):basetype(o) {}
	keycomp_base(const key_compare&,const allocator_type& al):basetype(al) {}


	key_compare& key_comp() const
	{
		return *(key_compare*)(NULL);
	}
};

template<typename K,typename A>
class keycomp_base<K,false,A> : public alloc_base<A,tl::is_empty_type<A>::value>
{
public:
	typedef K key_compare;
	typedef alloc_base<A,tl::is_empty_type<A>::value> basetype;
	typedef typename basetype::allocator_type allocator_type;

	keycomp_base() {}
	keycomp_base(const keycomp_base& o):basetype(o),_kc(o._kc) {}
	keycomp_base(const key_compare& kc,const allocator_type& al):basetype(al),_kc(kc) {}

	key_compare& key_comp() const
	{
		return _kc;
	}

protected:
	mutable key_compare _kc;
};

template<typename B>
class containerB : public container_base
{
protected:
	typedef B impl_type;
public:

	typedef typename impl_type::allocator_type allocator_type;

	typedef typename allocator_type::value_type value_type;
	typedef typename allocator_type::size_type size_type;
	typedef typename allocator_type::difference_type difference_type;

	typedef typename allocator_type::pointer pointer;
	typedef typename allocator_type::const_pointer const_pointer;

	typedef typename allocator_type::reference reference;
	typedef typename allocator_type::const_reference const_reference;

	typedef typename impl_type::iterator iterator;
	typedef typename impl_type::reverse_iterator reverse_iterator;
	typedef typename impl_type::const_iterator const_iterator;
	typedef typename impl_type::const_reverse_iterator const_reverse_iterator;

	inline containerB(){}

	template<typename T1,typename T2>
	containerB(const T1& kc,const T2& al):impl(kc,al) {}

	containerB(const containerB& o):impl(o.impl) {}

	containerB(const allocator_type& a):impl(a){}

	iterator begin()
	{
		return impl.begin();
	}

	iterator end()
	{
		return impl.end();
	}

	reverse_iterator rbegin()
	{
		return impl.rbegin();
	}

	reverse_iterator rend()
	{
		return impl.rend();
	}

	const_iterator begin() const
	{
		return impl.begin();
	}

	const_iterator end() const
	{
		return impl.end();
	}

	const_reverse_iterator rbegin() const
	{
		return impl.rbegin();
	}

	const_reverse_iterator rend() const
	{
		return impl.rend();
	}

	const_iterator cbegin() const
	{
		return impl.begin();
	}

	const_iterator cend() const
	{
		return impl.end();
	}

	const_reverse_iterator crbegin() const
	{
		return impl.rbegin();
	}

	const_reverse_iterator crend() const
	{
		return impl.rend();
	}

	bool empty() const
	{
		return impl.empty();
	}

	size_type size() const
	{
		return impl.size();
	}

	size_type max_size() const
	{
		return impl.get_allocator().max_size();
	}

	void clear()
	{
		impl.clear();
	}

	void swap(containerB& o)
	{
		impl.swap(o.impl);
	}

	allocator_type& get_allocator()
	{
		return impl.get_allocator();
	}

protected:
	mutable impl_type impl;
};

template<typename A>
class containerA : public alloc_base<A,tl::is_empty_type<A>::value>
{
public:
	typedef alloc_base<A,tl::is_empty_type<A>::value> basetype;
	typedef A allocator_type;

	typedef typename allocator_type::value_type value_type;
	typedef typename allocator_type::size_type size_type;
	typedef typename allocator_type::difference_type difference_type;

	typedef typename allocator_type::pointer pointer;
	typedef typename allocator_type::const_pointer const_pointer;

	typedef typename allocator_type::reference reference;
	typedef typename allocator_type::const_reference const_reference;

	containerA() {}
	containerA(const containerA& o):basetype(o) {}
	containerA(const allocator_type& al):basetype(al) {}

};

template<typename K,typename A>
class containerK : public keycomp_base<K,tl::is_empty_type<K>::value,A>
{
public:

	typedef keycomp_base<K,tl::is_empty_type<K>::value,A> basetype;

	typedef A allocator_type;

	typedef typename allocator_type::value_type value_type;
	typedef typename allocator_type::size_type size_type;
	typedef typename allocator_type::difference_type difference_type;

	typedef typename allocator_type::pointer pointer;
	typedef typename allocator_type::const_pointer const_pointer;

	typedef typename allocator_type::reference reference;
	typedef typename allocator_type::const_reference const_reference;

	typedef typename basetype::key_compare key_compare;

	containerK() {}
	containerK(const containerK& o):basetype(o) {}
	containerK(const key_compare& kc,const allocator_type& al):basetype(kc,al) {}

};



template<typename T,bool D>
class IteratorDirection
{
public:
	static inline T* add(T* p,intptr_t n)
	{
		return p+n;
	}
	static inline T* sub(T* p,intptr_t n)
	{
		return p-n;
	}
	static inline intptr_t sub(const T* p1,const T* p2)
	{
		return p1-p2;
	}
};

template<typename T>
class IteratorDirection<T,false>
{
public:
	static inline T* add(T* p,intptr_t n)
	{
		return p-n;
	}
	static inline T* sub(T* p,intptr_t n)
	{
		return p+n;
	}
	static inline intptr_t sub(const T* p1,const T* p2)
	{
		return p2-p1;
	}
};

// D iterator direction
// C const_iterator?
template<typename T,bool D,bool C>
class rac_iterator;


template<typename T,bool D>
class rac_iterator<T,D,true>	: public std::iterator<std::random_access_iterator_tag,T>
{
public:
	typedef T* pointer;

	explicit rac_iterator(const pointer x=pointer()) :m_ptr(x) {}

	rac_iterator& operator++()
	{
		m_ptr=IteratorDirection<T,D>::add(m_ptr,1);
		return *this;
	}
	rac_iterator operator++(int)
	{
		rac_iterator tmp(*this);
		m_ptr=IteratorDirection<T,D>::add(m_ptr,1);
		return tmp;
	}
	rac_iterator& operator--()
	{
		m_ptr=IteratorDirection<T,D>::sub(m_ptr,1);
		return *this;
	}
	rac_iterator operator--(int)
	{
		rac_iterator tmp(*this);
		m_ptr=IteratorDirection<T,D>::sub(m_ptr,1);
		return tmp;
	}
	rac_iterator& operator-= (intptr_t d)
	{
		m_ptr=IteratorDirection<T,D>::sub(m_ptr,d);
		return *this;
	}
	rac_iterator& operator+= (intptr_t d)
	{
		m_ptr=IteratorDirection<T,D>::add(m_ptr,d);
		return *this;
	}
	bool operator==(const rac_iterator& rhs)
	{
		return m_ptr==rhs.m_ptr;
	}
	bool operator!=(const rac_iterator& rhs)
	{
		return m_ptr!=rhs.m_ptr;
	}
	bool operator<(const rac_iterator& rhs)
	{
		return IteratorDirection<T,D>::sub(m_ptr,rhs.m_ptr)<0;
	}
	bool operator<=(const rac_iterator& rhs)
	{
		return IteratorDirection<T,D>::sub(m_ptr,rhs.m_ptr)<=0;
	}
	bool operator>(const rac_iterator& rhs)
	{
		return IteratorDirection<T,D>::sub(m_ptr,rhs.m_ptr)>0;
	}
	bool operator>=(const rac_iterator& rhs)
	{
		return IteratorDirection<T,D>::sub(m_ptr,rhs.m_ptr)>=0;
	}

	const T& operator*()
	{
		return *m_ptr;
	}
	T* get()
	{
		return m_ptr;
	}
	const T* operator->()
	{
		return m_ptr;
	}

protected:
	pointer m_ptr;
};

template<typename T,bool D>
class rac_iterator<T,D,false> : public rac_iterator<T,D,true>
{
protected:
	using rac_iterator<T,D,true>::m_ptr;
public:
	typedef T* pointer;

	explicit rac_iterator(pointer x = pointer()) :rac_iterator<T, D, true>(x) {}

	rac_iterator& operator++()
	{
		m_ptr=IteratorDirection<T,D>::add(m_ptr,1);
		return *this;
	}
	rac_iterator operator++(int)
	{
		rac_iterator tmp(*this);
		m_ptr=IteratorDirection<T,D>::add(m_ptr,1);
		return tmp;
	}
	rac_iterator& operator--()
	{
		m_ptr=IteratorDirection<T,D>::sub(m_ptr,1);
		return *this;
	}
	rac_iterator operator--(int)
	{
		rac_iterator tmp(*this);
		m_ptr=IteratorDirection<T,D>::sub(m_ptr,1);
		return tmp;
	}

	rac_iterator& operator-= (intptr_t d)
	{
		m_ptr = IteratorDirection<T, D>::sub(m_ptr, d);
		return *this;
	}
	rac_iterator& operator+= (intptr_t d)
	{
		m_ptr = IteratorDirection<T, D>::add(m_ptr, d);
		return *this;
	}

	T& operator*()
	{
		return *m_ptr;
	}
	T* get()
	{
		return m_ptr;
	}
	T* operator->()
	{
		return m_ptr;
	}
};


template<typename T,bool D,bool C>
rac_iterator<T,D,C> operator+(rac_iterator<T,D,C> it1,intptr_t n)
{
	return rac_iterator<T,D,C>(IteratorDirection<T, D>::add(it1.get(), n));
}

template<typename T,bool C,bool D>
rac_iterator<T,D,C> operator-(rac_iterator<T,D,C> it1,intptr_t n)
{
	return rac_iterator<T,D,C>(IteratorDirection<T, D>::sub(it1.get(), n));
}

template<typename T,bool D,bool C1,bool C2>
intptr_t operator-(rac_iterator<T,D,C1> it1,rac_iterator<T,D,C2> it2)
{
	return IteratorDirection<T,D>::sub(it1.get(),it2.get());
}


template<typename A>
class containerS : public containerA<A>
{
public:
	typedef containerA<A> basetype;
	typedef typename basetype::allocator_type allocator_type;
	typedef typename basetype::value_type value_type;
	typedef rac_iterator<value_type,true,false> iterator;
	typedef rac_iterator<value_type,true,true> const_iterator;

	typedef rac_iterator<value_type,false,false> reverse_iterator;
	typedef rac_iterator<value_type,false,true> const_reverse_iterator;

	typedef typename containerA<A>::size_type size_type;

	size_type max_size() const
	{
		return this->get_allocator().max_size();
	}

	containerS() {}
	containerS(const containerS& o):basetype(o) {}
	containerS(const allocator_type& al):basetype(al) {}

};


template<typename T,bool POD>
class xmem_helper_type;

template<typename T>
class xmem_helper_type<T,true>
{
public:
	static void destroy(T* first_,size_t count_)
	{
		EW_UNUSED(first_);
		EW_UNUSED(count_);
	}
	template<typename It>
	static void destroy(It first_,It last_)
	{
		EW_UNUSED(first_);
		EW_UNUSED(last_);
	}
};

template<typename T>
class xmem_helper_type<T,false>
{
public:
	static void destroy(T* first_,size_t count_)
	{
		for(size_t i=0; i<count_; i++)
		{
			first_[i].~T();
		}
	}

	template<typename It>
	static void destroy(It first_,It last_)
	{
		for(; first_<last_; first_++)
		{
			(*first_).~T();
		}
	}

};


template<typename T>
class xmem_helper : public xmem_helper_type<T,tl::is_pod<T>::value>
{
public:

	template<typename It>
	static void uninitialized_fill(It first_,It last_,const T& val_)
	{
		std::uninitialized_fill(first_,last_,val_);
	}

	template<typename It>
	static void uninitialized_fill_n(It first_,size_t count_,const T& val_)
	{
		std::uninitialized_fill_n(first_,count_,val_);
	}

	template<typename It>
	static void fill(It first_,It last_,const T& val_)
	{
		std::fill(first_,last_,val_);
	}

	template<typename It>
	static void fill_n(It first_,size_t count_,const T& val_)
	{
		std::fill_n(first_,count_,val_);
	}

	template<typename It,typename Ot>
	static Ot uninitialized_copy(It first_,It last_,Ot dest_)
	{
		return std::uninitialized_copy(first_,last_,dest_);
	}

	template<typename It,typename Ot>
	static Ot uninitialized_copy_n(It first_,size_t count_,Ot dest_)
	{
		return std::uninitialized_copy_n(first_,count_,dest_);		//C++11
	}

	template<typename It,typename Ot>
	static Ot copy(It first_,It last_,Ot dest_)
	{
		return std::copy(first_,last_,dest_);
	}

	template<typename It,typename Ot>
	static Ot copy_n(It first_,size_t count_,Ot dest_)
	{
		if(count_==0) return dest_;
		return std::copy_n(first_,count_,dest_);
	}

	template<typename It,typename Ot>
	static Ot copy_backward(It first_,It last_,Ot dest_)
	{
		return std::copy_backward(first_,last_,dest_);
	}
};


template<typename T>
class xmem : public xmem_helper<T>
{
public:

};

template<typename C1,typename C2>
bool container_equal(const C1& lhs,const C2& rhs)
{
	if(lhs.size()!=rhs.size()) return false;
	typename C1::const_iterator it1=lhs.begin();
	typename C1::const_iterator it2=lhs.end();
	typename C2::const_iterator dt1=rhs.begin();
	while(it1!=it2)
	{
		if(*it1++!=*dt1++)
		{
			return false;
		}
	}
	return true;
}

EW_LEAVE


#endif
