#ifndef __H_EW_BASIC_BITFLAGS__
#define __H_EW_BASIC_BITFLAGS__

#include "ewa_base/config.h"


EW_ENTER


class DLLIMPEXP_EWA_BASE BitFlags
{
public:

	inline BitFlags(int32_t flag=0):m_nFlags(flag){}
	inline bool get(int32_t flag) const{return (m_nFlags&flag)!=0;}
	inline void add(int32_t flag){m_nFlags|=flag;}
	inline void del(int32_t flag){m_nFlags&=~flag;}
	inline void clr(int32_t flag=0){m_nFlags=flag;}
	inline void inv(int32_t flag){m_nFlags^=flag;}
	inline int32_t val() const{return m_nFlags;}

	void set(int32_t flag,bool v);	// set flag   if(v) add(flag) else del(flag)

private:
	int32_t m_nFlags;
};

inline bool operator==(const BitFlags lhs,const BitFlags rhs)
{
	return lhs.val()==rhs.val();
}

inline bool operator!=(const BitFlags lhs,const BitFlags rhs)
{
	return lhs.val()!=rhs.val();
}


EW_LEAVE

namespace tl
{
	template<>
	struct is_pod<ew::BitFlags> : public value_type<true>{};
};

#endif
