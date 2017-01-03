#ifndef __H_EW_BASIC_BITFLAGS__
#define __H_EW_BASIC_BITFLAGS__

#include "ewa_base/config.h"


EW_ENTER


template<typename T>
class DLLIMPEXP_EWA_BASE BitFlags_t
{
public:
	typedef T type;

	inline BitFlags_t(type flag = 0) :m_nFlags(flag){}
	inline bool get(type flag) const{ return (m_nFlags&flag) != 0; }
	inline void add(type flag){ m_nFlags |= flag; }
	inline void del(type flag){ m_nFlags &= ~flag; }
	inline void clr(type flag = 0){ m_nFlags = flag; }
	inline void inv(type flag){ m_nFlags ^= flag; }
	inline int32_t val() const{return m_nFlags;}

	void set(type flag, bool v)
	{
		if (v) add(flag); else del(flag);
	}

private:
	type m_nFlags;
};

template<typename T>
inline bool operator==(const BitFlags_t<T> lhs, const BitFlags_t<T> rhs)
{
	return lhs.val()==rhs.val();
}

template<typename T>
inline bool operator!=(const BitFlags_t<T> lhs, const BitFlags_t<T> rhs)
{
	return lhs.val()!=rhs.val();
}

typedef BitFlags_t<int32_t> BitFlags;
typedef BitFlags_t<int64_t> BitFlags64;

EW_LEAVE

namespace tl
{
	template<typename T>
	struct is_pod<ew::BitFlags_t<T> > : public value_type<true>{};
};

#endif
