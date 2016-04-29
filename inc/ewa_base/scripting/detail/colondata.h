#ifndef __H_EW_SCRIPTING_DETAIL_COLONDATA__
#define __H_EW_SCRIPTING_DETAIL_COLONDATA__

#include "ewa_base/config.h"
#include "ewa_base/basic.h"
#include "ewa_base/collection.h"


EW_ENTER


class DLLIMPEXP_EWA_BASE ColonData
{
public:

	enum
	{
		FLAG_BEG=1<<0,
		FLAG_MID=1<<1,
		FLAG_END=1<<2,
	};

	ColonData(){}

	ColonData(double v1,double v2)
	{
		set(v1,v2);
	}

	ColonData(double v1,double vs,double v2)
	{
		set(v1,vs,v2);
	}

	inline void set(double v1,double v2)
	{
		val[0]=v1;val[2]=v2;
		flags.clr(FLAG_BEG|FLAG_END);
	}

	inline void set(double v1,double vs,double v2)
	{
		val[0]=v1;val[1]=vs;val[2]=v2;
		flags.clr(FLAG_BEG|FLAG_MID|FLAG_END);
	}

	inline void reset()
	{
		flags.clr(0);
	}

	inline double operator()(size_t k0) const
	{
		return val[0]+double(k0)*val[1];
	}

	inline operator bool() const
	{
		return flags.get(FLAG_BEG)&&flags.get(FLAG_END);
	}

	inline size_t size() const
	{
		if(!flags.get(FLAG_BEG)||!flags.get(FLAG_END))
		{
			return 0;
		}
		return _to_count();
	}

	inline size_t size(double v1,double v2) const
	{
		if(flags.get(FLAG_MID) && val[1]<0.0)
		{
			if(!flags.get(FLAG_BEG))
			{
				val[0]=v2;
			}

			if(!flags.get(FLAG_END))
			{
				val[2]=v1;
			}
		}
		else
		{
			if(!flags.get(FLAG_BEG))
			{
				val[0]=v1;
			}

			if(!flags.get(FLAG_END))
			{
				val[2]=v2;
			}
		}

		return _to_count();
	}

	inline arr_xt_dims size_ptr(double v1,double v2) const
	{
		return size(v1,v2);
	}
	
	inline arr_xt_dims size_ptr() const
	{
		return size();
	}	

	void set_beg(double v)
	{
		val[0]=v;
		flags.add(FLAG_BEG);
	}

	void set_mid(double v)
	{
		val[1]=v;
		flags.add(FLAG_MID);
	}

	void set_end(double v)
	{
		val[2]=v;
		flags.add(FLAG_END);
	}

	inline uint32_t hashcode() const
	{
		uint32_t u=0;
		hash_t<double> h;
		if(flags.get(FLAG_BEG)) u^=h(val[0]);
		if(flags.get(FLAG_MID)) u^=h(val[1]);
		if(flags.get(FLAG_END)) u^=h(val[2]);
		return u;
	}

	inline bool operator==(const ColonData& rhs) const
	{
		const ColonData& lhs(*this);
		if(lhs.flags.val()!=rhs.flags.val()) return false;
		if(lhs.flags.get(FLAG_BEG)&&lhs.val[0]!=rhs.val[0]) return false;
		if(lhs.flags.get(FLAG_MID)&&lhs.val[1]!=rhs.val[1]) return false;
		if(lhs.flags.get(FLAG_END)&&lhs.val[2]!=rhs.val[2]) return false;
		return true;
	}

	inline bool operator!=(const ColonData& rhs) const
	{
		return !((*this)==rhs);
	}

	inline double get_beg() const{return val[0];}
	inline double get_mid() const{return val[1];}

	BitFlags flags;

protected:

	static inline size_t _adjust_count(double v)
	{
		return 1+(size_t)::floor(v);
	}

	inline size_t _to_count() const
	{
		if(flags.get(FLAG_MID))
		{
			double d=val[2]-val[0];
			if(d*val[1]<=0.0) return 1;
			return _adjust_count(d/val[1]);
		}

		if(val[2]>=val[0])
		{
			val[1]=+1.0;
			return _adjust_count(val[2]-val[0]);
		}
		else
		{
			val[1]=-1.0;
			return _adjust_count(val[0]-val[2]);
		}
	}

	mutable double val[3];

};



DEFINE_OBJECT_NAME(ColonData,"ColonData")

template<>
class hash_t<ColonData>
{
public:
	uint32_t operator()(const ColonData& o)
	{
		return o.hashcode();
	}
};

EW_LEAVE

namespace tl
{
	template<> struct is_pod<ew::ColonData> : public value_type<true>{};
};

#endif
