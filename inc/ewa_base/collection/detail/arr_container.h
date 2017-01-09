#ifndef __H_EW_COLLECTION_POD_1t__
#define __H_EW_COLLECTION_POD_1t__


#include "ewa_base/collection/detail/collection_base.h"
#include "ewa_base/basic/hashing.h"

EW_ENTER

#pragma push_macro("new")
#undef new

template<typename T,typename A,bool pod>
class arr_container : public containerS<A>
{
public:

	typedef containerS<A> basetype;

	typedef T value_type;
	typedef T* pointer;
	typedef T& reference;

	typedef size_t size_type;

	typedef typename basetype::iterator iterator;
	typedef typename basetype::reverse_iterator reverse_iterator;
	typedef typename basetype::const_iterator const_iterator;
	typedef typename basetype::const_reverse_iterator const_reverse_iterator;

	arr_container();
	arr_container(const A& a):basetype(a){}
	arr_container(const arr_container& o);

	arr_container& operator=(const arr_container& o);

	~arr_container();

	inline void swap(arr_container& o)
	{
		std::swap(m_base,o.m_base);
		std::swap(m_end1,o.m_end1);
		std::swap(m_end2,o.m_end2);
		this->_swap_allocator(o);
	}

	void clear();

	size_type size() const
	{
		return m_end1-m_base;
	}

	size_type capacity() const
	{
		return m_end2-m_base;
	}

	bool empty() const
	{
		return m_base==m_end1;
	}

	void reserve(size_type capacity_);

	void resize(size_type count_,const T& val_=T());

	void assign(size_type count_,const T& val_);

	template<typename It>
	void assign(It first_,size_type count_);

	template<typename It>
	void assign(It first_,It last_)
	{
		assign(first_,std::distance(first_,last_));
	}

	iterator insert(const_iterator where_,const T& val_)
	{
		return insert(where_,&val_,1);
	}

	template<typename It>
	iterator insert(const_iterator where_,It first_,size_type count_);

	template<typename It>
	iterator insert(const_iterator where_,It first_,It last_)
	{
		return insert(where_,first_,std::distance(first_,last_));
	}

	iterator append(const T& val_);

#ifdef EW_C11
	iterator append(T&& val_);
#endif

	template<typename It>
	iterator append(It first_,size_type count_);


	template<typename It>
	iterator append(It first_,It last_)
	{
		return append(first_,std::distance(first_,last_));
	}


	iterator erase(iterator first_,size_t count_);

	iterator erase(iterator first_)
	{
		return erase(first_,1);
	}

	template<typename It>
	iterator erase(It first_,It last_)
	{
		return erase(first_,std::distance(first_,last_));
	}

	void shrink_to_fit();

	reference front();
	reference back();

	void pop_back();

	iterator begin(){ return iterator(m_base); }
	iterator end(){ return iterator(m_end1); }
	reverse_iterator rbegin(){ return reverse_iterator(m_end1 - 1); }
	reverse_iterator rend(){ return reverse_iterator(m_base - 1); }

	inline reference at (size_type n)
	{
		if(n>= size_type(m_end1-m_base)) Exception::XInvalidIndex();
		return m_base[n];
	}

	pointer data(){return m_base;}

	T& operator[](size_type n)
	{
		EW_ASSERT_INDEX_VALID(n, m_end1 - m_base);
		return m_base[n];
	}

	bool enlarge_size_by(size_t sz)
	{
		if(m_end1+sz>m_end2) return false;
		m_end1+=sz;
		return true;
	}

protected:

	template<typename It>
	iterator _insert_gen(const_iterator where_,It first_,size_type count_);

	template<typename It>
	iterator _append_gen(It first_,size_type count_);

	void _clear_and_rawset(T* p1,T* p2,T* p3);


	T* _alloc_copy(size_t capacity_,const T& val_,size_t count_);

	template<typename IT>
	T* _alloc_copy(size_t capacity_,IT first_,size_t count_);

	T* _alloc_copy(size_t capacity_,T* first_,T* last_)
	{
		return _alloc_copy(capacity_,first_,last_-first_);
	}

	void _dealloc_destroy(T* p1,T* p2,T* p3)
	{
		if(p1)
		{
			xmem<T>::destroy(p1,p2);
			this->get_allocator().deallocate(p1,p3-p1);
		}
	}

	void _check_not_empty()
	{
		if(m_base==m_end1)
		{
			Exception::XError("arr_container is empty()",false);
		}
	}

	template<typename P>
	inline bool _inside(P x)
	{
		const T* p=this->get_allocator().address(*x);
		return p>=m_base&&p<m_end2;
	}

	void _append_gen(const T& val_);

#ifdef EW_C11
	void _append_gen(T&& val_);
#endif

	inline size_t _gen_size(size_t n0,size_t n1)
	{
		if(this->get_allocator().max_size()-n0<=n1)
		{
			Exception::XBadAlloc();
		}
		return sz_helper::gen(n0+n1);
	}

	void _resize_gen(size_type count_,const T& val_);

	pointer m_base;
	pointer m_end1;
	pointer m_end2;

};


template<typename T,typename A,bool pod>
inline void arr_container<T,A,pod>::_clear_and_rawset(T* p1,T* p2,T* p3)
{
	if(m_base)
	{
		xmem<T>::destroy(m_base,m_end1);
		this->get_allocator().deallocate(m_base,m_end2-m_end1);
	}

	m_base=p1;
	m_end1=p2;
	m_end2=p3;
}

template<typename T,typename A,bool pod>
T* arr_container<T,A,pod>::_alloc_copy(size_t capacity_,const T& val_,size_t count_)
{
	if(capacity_==0) return NULL;

	T* _tmp=this->get_allocator().allocate(capacity_);

	if(pod)
	{
		for(size_t i=0;i<count_;i++) _tmp[i]=val_;
	}
	else
	{
		try
		{
			xmem<T>::uninitialized_fill_n(_tmp,count_,val_);
		}
		catch(...)
		{
			this->get_allocator().deallocate(_tmp,capacity_);
			throw;
		}
	}
	return _tmp;
}

template<typename T,typename A,bool pod>
template<typename IT>
T* arr_container<T,A,pod>::_alloc_copy(size_t capacity_,IT first_,size_t count_)
{
	if(capacity_==0) return NULL;

	T* _tmp=this->get_allocator().allocate(capacity_);
	if(count_==0) return _tmp;

	try
	{
		xmem<T>::uninitialized_copy_n(first_,count_,_tmp);
	}
	catch(...)
	{
		this->get_allocator().deallocate(_tmp,capacity_);
		throw;
	}

	return _tmp;
}

template<typename T,typename A,bool pod>
inline arr_container<T,A,pod>::arr_container():m_base(NULL),m_end1(NULL),m_end2(NULL)
{

}

template<typename T,typename A,bool pod>
inline arr_container<T,A,pod>::arr_container(const arr_container& o)
{
	size_t n1=o.size();
	m_base=_alloc_copy(n1,o.m_base,o.m_end1);
	m_end1=m_end2=m_base+n1;
}

template<typename T,typename A,bool pod>
inline arr_container<T,A,pod>& arr_container<T,A,pod>::operator=(const arr_container& o)
{
	if(this==&o) return *this;

	size_t n1=o.size();
	T* p1=_alloc_copy(n1,o.m_base,o.m_end1);
	T* p2=p1+n1;

	_clear_and_rawset(p1,p2,p2);

	return *this;
}


template<typename T,typename A,bool pod>
inline arr_container<T,A,pod>::~arr_container()
{
	_clear_and_rawset(NULL,NULL,NULL);
}


template<typename T,typename A,bool pod>
void arr_container<T,A,pod>::shrink_to_fit()
{
	if(m_end1==m_end2) return;

	size_t n0=size();
	T* p1=_alloc_copy(n0,m_base,m_end1);
	T* p2=p1+n0;

	_clear_and_rawset(p1,p2,p2);
}

template<typename T,typename A,bool pod>
inline void arr_container<T,A,pod>::pop_back()
{
	_check_not_empty();
	this->get_allocator().destroy(--m_end1);
}

template<typename T,typename A,bool pod>
inline typename arr_container<T,A,pod>::reference  arr_container<T,A,pod>::back()
{
	_check_not_empty();
	return *(m_end1-1);
}

template<typename T,typename A,bool pod>
inline typename arr_container<T,A,pod>::reference arr_container<T,A,pod>::front()
{
	_check_not_empty();
	return *m_base;
}

template<typename T,typename A,bool pod>
inline void arr_container<T,A,pod>::clear()
{
	if(m_end1!=m_base)
	{
		xmem<T>::destroy(m_base,m_end1);
		m_end1=m_base;
	}
}


template<typename T,typename A,bool pod>
inline void arr_container<T,A,pod>::reserve(size_type capacity_)
{
	if(capacity()>=capacity_)
	{
		return;
	}

	size_t n0=size();
	T* p1=_alloc_copy(capacity_,m_base,n0);
	T* p2=p1+n0;
	T* p3=p1+capacity_;

	_clear_and_rawset(p1,p2,p3);

}

template<typename T,typename A,bool pod>
void arr_container<T,A,pod>::_resize_gen(size_type count_,const T& val_)
{
	size_t n0=size();

	T* p1=_alloc_copy(count_,m_base,m_end1);
	T* p2=p1+n0;
	T* p3=p1+count_;

	xmem<T>::uninitialized_fill(p2,p3,val_);

	_clear_and_rawset(p1,p3,p3);
}

template<typename T,typename A,bool pod>
inline void arr_container<T,A,pod>::resize(size_type count_,const T& val_)
{
	size_t n0=size();
	if(n0>=count_)
	{
		T* _end2=m_base+count_;
		xmem<T>::destroy(_end2,m_end1);
		m_end1=_end2;
	}
	else if(capacity()>=count_)
	{
		T* _end2=m_base+count_;
		xmem<T>::uninitialized_fill(m_end1,_end2,val_);
		m_end1=_end2;
	}
	else
	{
		_resize_gen(count_,val_);
	}
}



template<typename T,typename A,bool pod>
void arr_container<T,A,pod>::_append_gen(const T& val_)
{
	size_t n0=size();
	size_t sz=_gen_size(n0,1);

	T* p1=_alloc_copy(sz,m_base,m_end1);
	T* p2=p1+n0;
	T* p3=p1+sz;

	try
	{
		this->get_allocator().construct(p2++,val_);
	}
	catch(...)
	{
		_dealloc_destroy(p1,p2,p3);
		throw;
	}

	_clear_and_rawset(p1,p2,p3);
}

#ifdef EW_C11

template<typename T,typename A,bool pod>
void arr_container<T,A,pod>::_append_gen(T&& val_)
{
	size_t n0=size();
	size_t sz=_gen_size(n0,1);

	T* p1=_alloc_copy(sz,m_base,m_end1);
	T* p2=p1+n0;
	T* p3=p1+sz;

	try
	{
		this->get_allocator().construct(p2++,std::forward<T>(val_));
	}
	catch(...)
	{
		_dealloc_destroy(p1,p2,p3);
		throw;
	}

	_clear_and_rawset(p1,p2,p3);
}

template<typename T,typename A,bool pod>
inline typename arr_container<T,A,pod>::iterator  arr_container<T,A,pod>::append(T&& val_)
{
	if(m_end1==m_end2)
	{
		_append_gen(std::forward<T>(val_));
	}
	else
	{
		this->get_allocator().construct(m_end1++,std::forward<T>(val_));
	}

	return iterator(m_end1);
}

#endif

template<typename T,typename A,bool pod>
inline typename arr_container<T,A,pod>::iterator  arr_container<T,A,pod>::append(const T& val_)
{
	if(m_end1==m_end2)
	{
		_append_gen(val_);
	}
	else
	{
		this->get_allocator().construct(m_end1++,val_);
	}

	return iterator(m_end1);
}


template<typename T,typename A,bool pod>
template<typename It>
typename arr_container<T,A,pod>::iterator arr_container<T,A,pod>::_append_gen(It first_,size_type count_)
{
	size_t n0=size();
	size_t sz=_gen_size(n0,count_);

	T* p1=_alloc_copy(sz,m_base,m_end1);
	T* p2=p1+n0;
	T* p3=p1+sz;

	try
	{
		xmem<T>::uninitialized_copy_n(first_,count_,p2);
	}
	catch(...)
	{
		_dealloc_destroy(p1,p2,p3);
		throw;
	}

	_clear_and_rawset(p1,p2+count_,p3);

	return iterator(m_end1);

}

template<typename T,typename A,bool pod>
template<typename It>
typename arr_container<T,A,pod>::iterator arr_container<T,A,pod>::append(It first_,size_type count_)
{
	if(count_==0) return iterator(m_end1);

	if(size_type(m_end2-m_end1)<count_)
	{
		_append_gen(first_,count_);
	}
	else
	{
		xmem<T>::uninitialized_copy_n(first_,count_,m_end1);
		m_end1+=count_;
	}
	return iterator(m_end1);
}



template<typename T,typename A,bool pod>
template<typename It>
typename arr_container<T,A,pod>::iterator arr_container<T,A,pod>::_insert_gen(const_iterator where_,It first_,size_type count_)
{

	size_t n0=size();
	size_t sz=_gen_size(n0,count_);

	T* px=(T*)&(*where_);

	T* p1=this->get_allocator().allocate(sz);
	T* p3=p1+sz;

	iterator it(p1);

	T* pp=p1+(px-m_base)+count_;

	try
	{
		it=xmem<T>::uninitialized_copy(m_base,px,it);
		it=xmem<T>::uninitialized_copy_n(first_,count_,it);
		it=xmem<T>::uninitialized_copy(px,m_end1,it);
	}
	catch(...)
	{
		_dealloc_destroy(p1,&(*it),p3);
		throw;
	}

	_clear_and_rawset(p1,&(*it),p3);

	return iterator(pp);

}

template<typename T,typename A,bool pod>
template<typename It>
typename arr_container<T,A,pod>::iterator arr_container<T,A,pod>::insert(const_iterator where_,It first_,size_type count_)
{
	if(where_==end())
	{
		return append(first_,count_);
	}

	if(_inside(first_)||size_type(m_end2-m_end1)<count_)
	{
		return _insert_gen(where_,first_,count_);
	}


	T* p1=(T*)&(*where_);
	T* d1=p1+count_;

	intptr_t dd=m_end1-d1;
	if(dd>=0)
	{
		T* p2=p1+dd;
		m_end1=xmem<T>::uninitialized_copy_n(p2,count_,m_end1);

		for(T* d2=p2+count_;p2>p1;)
		{
			*--d2=*--p2;
		}

		xmem<T>::copy_n(first_,count_,p1);
	}
	else
	{
		T* p2=m_end1;
		It x2=first_;
		std::advance(x2,count_+dd);
		m_end1=xmem<T>::uninitialized_copy_n(x2,-dd,m_end1);
		m_end1=xmem<T>::uninitialized_copy(p1,p2,m_end1);

		xmem<T>::copy_n(first_,count_+dd,p1);
	}

	return iterator(p1 + count_);
}


template<typename T,typename A,bool pod>
void arr_container<T,A,pod>::assign(size_type count_,const T& val_)
{
	if(size()>=count_)
	{
		xmem<T>::fill_n(m_base,count_,val_);

		T* _end2=m_base+count_;
		xmem<T>::destroy(_end2,m_end1);
		m_end1=_end2;


	}
	else if(capacity()>=count_)
	{
		T* _end2=m_base+count_;
		xmem<T>::uninitialized_fill(m_end1,_end2,val_);
		xmem<T>::fill(m_base,m_end1,val_);
		m_end1=_end2;
	}
	else
	{

		T* p1=_alloc_copy(count_,val_,count_);
		T* p2=p1+count_;

		_clear_and_rawset(p1,p2,p2);
	}
}

template<typename T,typename A,bool pod>
template<typename It>
void arr_container<T,A,pod>::assign(It first_,size_type count_)
{
	if(count_==0)
	{
		clear();
		return;
	}

	if(_inside(first_))
	{
		T* p1=_alloc_copy(count_,first_,count_);
		T* p2=p1+count_;
		_clear_and_rawset(p1,p2,p2);
	}
	else if(size()>=count_)
	{
		xmem<T>::copy_n(first_,count_,m_base);
		T* _end2=m_base+count_;
		xmem<T>::destroy(_end2,m_end1);
		m_end1=_end2;

	}
	else if(capacity()>=count_)
	{
		T* p1=m_base;
		T* p2=m_end1;

		T* _end2=p1+count_;

		while(p1<p2)
		{
			*p1++=*first_++;
		}

		xmem<T>::uninitialized_copy_n(first_,_end2-p2,p2);
		m_end1=_end2;
	}
	else
	{
		T* p1=_alloc_copy(count_,first_,count_);
		T* p2=p1+count_;
		_clear_and_rawset(p1,p2,p2);
	}
}


template<typename T,typename A,bool pod>
typename arr_container<T,A,pod>::iterator arr_container<T,A,pod>::erase(iterator first_,size_t count_)
{

	T* p1=&(*first_);
	T* p2=p1+count_;
	T* p3=m_end1;
	EW_ASSERT(p1>=m_base && p1<=m_end1 && p2>=m_base && p2<=m_end1);

	m_end1=xmem<T>::copy(p2,p3,p1);
	xmem<T>::destroy(m_end1,p3);

	return first_;
}


template<typename T,typename A,bool pod>
bool operator==(const arr_container<T,A,pod>& lhs,const arr_container<T,A,pod>& rhs)
{
	if(lhs.size()!=rhs.size())
	{
		return false;
	}
	for(size_t i=0; i<lhs.size(); i++)
	{
		if(lhs[i]!=rhs[i]) return false;
	}
	return true;
}

template<typename T,typename A,bool pod>
bool operator!=(const arr_container<T,A,pod>& lhs,const arr_container<T,A,pod>& rhs)
{
	return !(lhs==rhs);
}

template<typename T,typename A,bool pod> class hash_t<arr_container<T,A,pod> >
{
public:
	uint32_t operator()(const arr_container<T,A,pod>& o)
	{
		return hash_array<T>::hash(o.data(),o.size());
	}
};

#pragma pop_macro("new")

EW_LEAVE

#endif
