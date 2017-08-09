#ifndef __H_EW_MATH_TINY_BOX__
#define __H_EW_MATH_TINY_BOX__

#include "ewa_base/math/tiny_vec.h"
#include <limits>

EW_ENTER

template<typename T,int N>
class tiny_box
{
public:
	tiny_vec<T, N> lo, hi;

	operator T*(){ return &lo[0]; };
	operator const T*() const { return &lo[0]; }

	tiny_box(){}

	tiny_box(const tiny_vec<T, N>& v)
		: lo(v),hi(v){}

	tiny_box(const tiny_vec<T, N>& l, const tiny_vec<T, N>& h)
		: lo(l), hi(h) {}

	void load_min()
	{
		lo.seta(+std::numeric_limits<T>::max());
		hi.seta(-std::numeric_limits<T>::max());
	}

	void load_max()
	{
		lo.seta(-std::numeric_limits<T>::max());
		hi.seta(+std::numeric_limits<T>::max());
	}

	bool is_valid() const
	{
		if(N>0 && lo[0]>hi[0]) return false;
		if(N>1 && lo[1]>hi[1]) return false;
		if(N>2 && lo[2]>hi[2]) return false;
		if(N>3 && lo[3]>hi[3]) return false;
		if(N>4)
		{
			for(size_t i=4;i<N;i++)
			{
				if(lo[i]>hi[i]) return false;
			}
		}
		return true;
	}

	bool test_x(T v) const 
	{
		return v>=lo[0] && v<=hi[0];
	}

	bool test_y(T v) const
	{
		return v>=lo[1] && v<=hi[1];
	}

	bool test_z(T v) const
	{
		return v>=lo[2] && v<=hi[2];
	}

	bool test(const tiny_vec<T, N>& v) const
	{
		for(int i=0;i<N;i++)
		{
			if(v[i]>=lo[i] && v[i]<=hi[i]) continue;
			return false;
		}
		return true;
	}

	bool test(const tiny_box<T, N>& v) const
	{
		return test(v.lo) && test(v.hi);
	}

	template<int X>
	bool test_n(const tiny_vec<T, N>& v) const
	{
		return v[X]>=lo[X] && v[X]<=hi[X];
	}

	void add_x(T v)
	{
		if(N>0 && lo[0]>v) lo[0]=v;
		if(N>0 && hi[0]<v) hi[0]=v;
	}

	void add_y(T v)
	{
		if(N>1 && lo[1]>v) lo[1]=v;
		if(N>1 && hi[1]<v) hi[1]=v;
	}

	void add_z(T v)
	{
		if(N>2 && lo[2]>v) lo[2]=v;
		if(N>2 && hi[2]<v) hi[2]=v;
	}

	void add(const tiny_vec<T,N>& v)
	{
		if(N>0 && lo[0]>v[0]) lo[0]=v[0];
		if(N>0 && hi[0]<v[0]) hi[0]=v[0];
		if(N>1 && lo[1]>v[1]) lo[1]=v[1];
		if(N>1 && hi[1]<v[1]) hi[1]=v[1];
		if(N>2 && lo[2]>v[2]) lo[2]=v[2];
		if(N>2 && hi[2]<v[2]) hi[2]=v[2];
		if(N>3 && lo[3]>v[3]) lo[3]=v[3];
		if(N>3 && hi[3]<v[3]) hi[3]=v[3];

		if(N>4)
		{
			for(size_t i=4;i<N;i++)
			{
				if(lo[i]>v[i]) lo[i]=v[i];
				if(hi[i]<v[i]) hi[i]=v[i];
			}
		}
	}

	void add(const tiny_box& b)
	{
		add(b.lo);
		add(b.hi);
	}

	template<typename T1>
	void add(const tiny_mat<T1,4,4>& m4, const tiny_box& b)
	{
		typedef tiny_vec<T, N> vec_t;
		const vec_t& blo(b.lo);
		const vec_t& bhi(b.hi);
		add(m4 * blo);
		add(m4 * vec_t(bhi[0], blo[1], blo[2]));
		add(m4 * vec_t(blo[0], bhi[1], blo[2]));
		add(m4 * vec_t(bhi[0], bhi[1], blo[2]));

		add(m4 * vec_t(blo[0], blo[1], bhi[2]));
		add(m4 * vec_t(bhi[0], blo[1], bhi[2]));
		add(m4 * vec_t(blo[0], bhi[1], bhi[2]));
		add(m4 * bhi);
	}

	void intersect(const tiny_box& o)
	{
		if(N>0 && lo[0]<o.lo[0]) lo[0]=o.lo[0];
		if(N>1 && lo[1]<o.lo[1]) lo[1]=o.lo[1];
		if(N>2 && lo[2]<o.lo[2]) lo[2]=o.lo[2];
		if(N>3 && lo[3]<o.lo[3]) lo[3]=o.lo[3];
		if(N>0 && hi[0]>o.hi[0]) hi[0]=o.hi[0];
		if(N>1 && hi[1]>o.hi[1]) hi[1]=o.hi[1];
		if(N>2 && hi[2]>o.hi[2]) hi[2]=o.hi[2];
		if(N>3 && hi[3]>o.hi[3]) hi[3]=o.hi[3];
		if(N>4)
		{
			for(size_t i=4;i<N;i++)
			{
				if(lo[i]<o.lo[i]) lo[i]=o.lo[i];
				if(hi[i]>o.hi[i]) hi[i]=o.hi[i];
			}
		}
	}


	void set_x(T v1,T v2)
	{
		if(N>0) lo[0]=v1;hi[0]=v2;
	}

	void set_y(T v1,T v2)
	{
		if(N>1) lo[1]=v1;hi[1]=v2;
	}

	void set_z(T v1,T v2)
	{
		if(N>2) lo[2]=v1;hi[2]=v2;
	}

	T x_width() const
	{
		if(N>0) return hi[0]-lo[0];
		return 0;
	}

	T y_width() const
	{
		if(N>1) return hi[1]-lo[1];
		return 0;
	}

	T z_width() const
	{
		if(N>2) return hi[2]-lo[2];
		return 0;
	}

	tiny_vec<T, N> width() const
	{
		return hi - lo;
	}

	tiny_vec<T,N> center() const
	{
		return 0.5*(lo+hi);
	}

	void Translate(const tiny_vec<T, 3>& v)
	{
		lo += v;
		hi += v;
	}

	void Scale(T v)
	{
		lo *= v;
		hi *= v;
	}

	bool operator==(const tiny_box& rhs)
	{
		return lo == rhs.lo && hi == rhs.hi;
	}

	bool operator!=(const tiny_box& rhs)
	{
		return lo != rhs.lo || hi != rhs.hi;
	}

};

template<int N>
class tiny_box < String, N >
{
public :
	tiny_storage<String, N> lo, hi;

	void set_x(String v1, String v2)
	{
		if (N>0) lo[0] = v1; hi[0] = v2;
	}

	void set_y(String v1, String v2)
	{
		if (N>1) lo[1] = v1; hi[1] = v2;
	}

	void set_z(String v1, String v2)
	{
		if (N>2) lo[2] = v1; hi[2] = v2;
	}

};



EW_LEAVE

namespace tl
{
	template<typename T,int N>
	struct is_pod<ew::tiny_box<T,N> > : public value_type<true>{};
};

#endif
