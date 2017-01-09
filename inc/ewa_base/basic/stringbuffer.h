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
class DLLIMPEXP_EWA_BASE StringBuffer : public arr_1t<T,AllocatorN<def_allocator,1> >
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
	StringBuffer(const T* p1,size_t ln)
	{
		assign(p1,ln);
	}
	StringBuffer(const T* p1,const T* p2)
	{
		assign(p1,p2);
	}
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

	StringBuffer(const String& o)
	{
		(*this)=o;
	}

	StringBuffer& operator=(const String& o);
	StringBuffer& operator=(const T* p1);

	StringBuffer& operator+=(const StringBuffer& o)
	{
		this->append(o.begin(),o.end());
		return *this;
	}
	StringBuffer& operator+=(const String& o);

	bool load(const String& file,int t=FILE_TYPE_TEXT);
	bool save(const String& file,int t=FILE_TYPE_TEXT);

	T* c_str();
	const T* c_str() const;

	StringBuffer& operator<<(bool v);
	StringBuffer& operator<<(char v);
	StringBuffer& operator<<(int32_t v);
	StringBuffer& operator<<(int64_t v);
	StringBuffer& operator<<(uint32_t v);
	StringBuffer& operator<<(uint64_t v);
	StringBuffer& operator<<(float v);
	StringBuffer& operator<<(double v);
	StringBuffer& operator<<(const T* v);
	StringBuffer& operator<<(const void* v);
	StringBuffer& operator<<(const String& v);
	StringBuffer& operator<<(const StringBuffer& v);


	StringBuffer& operator<<(const std::basic_string<T>& v)
	{
		return (*this)<<v.c_str();
	}

	bool enlarge_size_by(size_t sz);

protected:

	template<typename G>
	void _do_format_integer(G v);
};

template<typename T> class hash_t<StringBuffer<T> >
{
public:
	inline uint32_t operator()(const StringBuffer<T>& o)
	{
		return hash_array<T>::hash(o.data(),o.size());
	}
};

inline const char* StringParamCast::g(const StringBuffer<char>& v)
{
	return v.c_str();
}


EW_LEAVE
#endif
