#ifndef __H_EW_COLLECTION_ARR_XT__
#define __H_EW_COLLECTION_ARR_XT__


#include "ewa_base/collection/arr_1t.h"
#include "ewa_base/math/tiny_cpx.h"

EW_ENTER

template<typename T,typename A=def_allocator>
class arr_xt : public containerB< arr_container<T,typename A::template rebind<T>::other,tl::is_pod<T>::value> >
{
protected:
	typedef containerB< arr_container<T,typename A::template rebind<T>::other,tl::is_pod<T>::value> > basetype;

public:
	static const size_t MAX_DIM=arr_xt_dims::MAX_DIM;

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

	inline arr_xt(){}
	inline arr_xt(const arr_xt& o):basetype(o),dims(o.dims){}
	inline explicit arr_xt(const A& al):basetype(al){}

	inline arr_xt(size_type k0, size_type k1 = 1, size_type k2 = 1, size_type k3 = 1, size_type k4 = 1, size_type k5 = 1)
	{
		resize(k0, k1, k2, k3, k4, k5);
	}

	inline arr_xt& operator=(const arr_xt& o)
	{
		impl=o.impl;
		dims=o.dims;
		return *this;
	}

	inline void swap(arr_xt& o)
	{
		impl.swap(o.impl);
		std::swap(dims,o.dims);
	}


#if defined(EW_C11)

	inline arr_xt(arr_xt&& p){swap(p);}
	inline arr_xt& operator=(arr_xt&& p){swap(p);return *this;}

#endif

	void assign(size_type count_,const T& val_)
	{
		impl.assign(count_,val_);
		dims.resize(count_);
	}

	template<typename It>
	void assign(It first_,size_type count_)
	{
		impl.assign(first_,count_);
		dims.resize(count_);
	}

	template<typename It>
	void assign(It first_,It last_)
	{
		impl.assign(first_,last_);
		dims.resize(impl.size());
	}

	inline iterator append(const T& val_)
	{
		return impl.append(val_);
	}

	inline void push_back(const T& val_)
	{
		impl.append(val_);
	}

	inline void pop_back_and_reshaepe_to_row_vector(size_t n)
	{
		if(impl.size()<n) Exception::XError("not enough elements for pop");
		size_t d=impl.size()-n;
		impl.resize(d);
		dims.resize(1,d);
	}

	void reshape(size_type k0,size_type k1=1,size_type k2=1,size_type k3=1,size_type k4=1,size_type k5=1);
	void reshape(const arr_xt_dims& kn);

	void resize(size_type k0,size_type k1=1,size_type k2=1,size_type k3=1,size_type k4=1,size_type k5=1);
	void resize(const arr_xt_dims& kn,size_t rs=0);


	void clear()
	{
		impl.clear();
		dims.resize(0);
	}

	inline T& operator()(size_type k0)
	{
		return impl[dims(k0)];
	}
	inline T& operator()(size_type k0,size_type k1)
	{
		return impl[dims(k0,k1)];
	}
	inline T& operator()(size_type k0,size_type k1,size_type k2)
	{
		return impl[dims(k0,k1,k2)];
	}
	inline T& operator()(size_type k0,size_type k1,size_type k2,size_type k3)
	{
		return impl[dims(k0,k1,k2,k3)];
	}
	inline T& operator()(size_type k0,size_type k1,size_type k2,size_type k3,size_type k4)
	{
		return impl[dims(k0,k1,k2,k3,k4)];
	}
	inline T& operator()(size_type k0,size_type k1,size_type k2,size_type k3,size_type k4,size_type k5)
	{
		return impl[dims(k0,k1,k2,k3,k4,k5)];
	}


	inline const T& operator()(size_type k0) const
	{
		return impl[dims(k0)];
	}
	inline const T& operator()(size_type k0,size_type k1) const
	{
		return impl[dims(k0,k1)];
	}
	inline const T& operator()(size_type k0,size_type k1,size_type k2) const
	{
		return impl[dims(k0,k1,k2)];
	}
	inline const T& operator()(size_type k0,size_type k1,size_type k2,size_type k3) const
	{
		return impl[dims(k0,k1,k2,k3)];
	}
	inline const T& operator()(size_type k0,size_type k1,size_type k2,size_type k3,size_type k4) const
	{
		return impl[dims(k0,k1,k2,k3,k4)];
	}
	inline const T& operator()(size_type k0,size_type k1,size_type k2,size_type k3,size_type k4,size_type k5) const
	{
		return impl[dims(k0,k1,k2,k3,k4,k5)];
	}

	inline size_type size() const
	{
		return impl.size();
	}
	inline size_type size(int n) const
	{
		return dims[n];
	}

	inline const arr_xt_dims& size_ptr() const
	{
		return dims;
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

protected:	
	using basetype::impl;
	arr_xt_dims dims;
};

template<typename T,typename A>
void arr_xt<T,A>::resize(const arr_xt_dims& kn,size_t rs)
{

	size_t _newsize=kn.checked_size(this->get_allocator().max_size());
	if(_newsize<rs) _newsize=rs;

	arr_xt tmp;
	tmp.impl.resize(_newsize);
	tmp.dims=kn;

	arr_xt& _Eold(*this);
	arr_xt& _Enew(tmp);

	if(_newsize>0)
	{
		for(size_type a0=0; a0<std::min(_Eold.dims[0],_Enew.dims[0]); a0++)
			for(size_type a1=0; a1<std::min(_Eold.dims[1],_Enew.dims[1]); a1++)
				for(size_type a2=0; a2<std::min(_Eold.dims[2],_Enew.dims[2]); a2++)
					for(size_type a3=0; a3<std::min(_Eold.dims[3],_Enew.dims[3]); a3++)
						for(size_type a4=0; a4<std::min(_Eold.dims[4],_Enew.dims[4]); a4++)
							for(size_type a5=0; a5<std::min(_Eold.dims[5],_Enew.dims[5]); a5++)
							{
								tmp(a0,a1,a2,a3,a4,a5)=(*this)(a0,a1,a2,a3,a4,a5);
							}
	}

	swap(tmp);
}

template<typename T,typename A>
void arr_xt<T,A>::resize(size_type k0,size_type k1,size_type k2,size_type k3,size_type k4,size_type k5)
{
	resize(arr_xt_dims(k0,k1,k2,k3,k4,k5));
}

template<typename T,typename A>
void arr_xt<T,A>::reshape(size_type k0,size_type k1,size_type k2,size_type k3,size_type k4,size_type k5)
{
	arr_xt_dims _dms(k0,k1,k2,k3,k4,k5);
	reshape(_dms);
}

template<typename T,typename A>
void arr_xt<T,A>::reshape(const arr_xt_dims& _dms)
{
	size_t n = _dms.checked_size(this->get_allocator().max_size());
	if(n!=impl.size())
	{
		impl.resize(_dms.size());
	}
	dims=_dms;
}

template<typename T,typename A1,typename A2>
bool operator==(const arr_xt<T,A1>& lhs,const arr_xt<T,A2>& rhs)
{
	typedef size_t size_type;
	if(lhs.size()!=rhs.size()) return false;

	for(size_type i=0; i<lhs.MAX_DIM; i++)
	{
		if(lhs.size(i)!=rhs.size(i)) return false;
	}

	for(size_type i=0; i<lhs.size(); i++)
	{
		if(lhs[i]!=rhs[i]) return false;
	}

	return true;
}

template<typename T,typename A1,typename A2>
bool operator!=(const arr_xt<T,A1>& lhs,const arr_xt<T,A2>& rhs)
{
	return !(lhs==rhs);
}


template<typename T,typename A> class hash_t<arr_xt<T,A> >
{
public:
	typedef arr_xt<T,A> type;
	uint32_t operator()(const arr_xt<T,A>& o)
	{
		return hash_array<T>::hash(o.data(),o.size())^hash_array<typename type::size_type>::hash(o.size_ptr(),type::MAX_DIM);
	}
};



template<typename T>
struct opx_scalar_arr : public tl::value_type<false>
{
	typedef T type;
	typedef arr_xt<T> promoted;
};
template<typename T,typename A>
struct opx_scalar_arr<arr_xt<T,A> > : public tl::value_type<true>
{
	typedef T type;
};

template<typename X,typename Y>
struct arr_promote : public opx_helper_promote<X,Y,cpx_promote,opx_scalar_arr>{};

EW_LEAVE

#endif
