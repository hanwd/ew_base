#ifndef __H_EW_STRINGBUFFER__
#define __H_EW_STRINGBUFFER__

#include "ewa_base/config.h"

#include "ewa_base/basic/hashing.h"
#include "ewa_base/basic/platform.h"
#include "ewa_base/basic/string_detail.h"
#include "ewa_base/collection/arr_1t.h"
#include "ewa_base/memory/allocator.h"


#ifdef _MSC_VER
#pragma warning(disable:4307)
#endif

EW_ENTER



template<typename T>
class DLLIMPEXP_EWA_BASE StringBuffer : 
	public arr_1t<T,AllocatorN<def_allocator,1> >, 
	public FormatHelper<StringBuffer<T>,typename tl::meta_if<sizeof(T)==1,FormatPolicy2,FormatPolicy1>::type>
{
public:
	typedef arr_1t<T,AllocatorN<def_allocator,1> > basetype;
	typedef typename basetype::size_type size_type;

	using basetype::assign;
	using basetype::resize;
	using basetype::data;
	using basetype::size;

	StringBuffer() {}

	StringBuffer(const T* p1);
	StringBuffer(const T* p1,size_t ln){assign(p1,ln);}
	StringBuffer(const T* p1,const T* p2){assign(p1,p2);}

	StringBuffer(const StringBuffer& o):basetype(o) {}

#if defined(EW_C11)
	StringBuffer(StringBuffer&& p)
	{
		swap(p);
	}
	const StringBuffer& operator=(StringBuffer&& p)
	{
		swap(p);
		return *this;
	}
#endif

	StringBuffer(const String& o){(*this)=o;}

	StringBuffer& operator=(const String& o);
	StringBuffer& operator=(const T* p1);

	StringBuffer& operator+=(const StringBuffer& o)
	{
		basetype::append(o.begin(),o.end());
		return *this;
	}
	StringBuffer& operator+=(const String& o);

	bool load(const String& file,int t=FILE_TYPE_TEXT);
	bool save(const String& file,int t=FILE_TYPE_TEXT);

	T* c_str();
	const T* c_str() const;

	bool enlarge_size_by(size_t sz);

	using basetype::append;

	template<typename X>
	void append(const X* p,size_t n)
	{
		if(sizeof(X)==sizeof(T)) basetype::append((T*)p,n);
		else (*this)+=String(p,n);
	}

};

template<typename T> class hash_t<StringBuffer<T> >
{
public:
	inline uint32_t operator()(const StringBuffer<T>& o)
	{
		return hash_array<T>::hash(o.data(),o.size());
	}
};

template<typename T>
inline const T* FormatPolicy::cast(const StringBuffer<T>& v)
{
	return v.c_str();
}


EW_LEAVE
#endif
