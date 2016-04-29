#ifndef __H_EW_MATH_TINY_BOX__
#define __H_EW_MATH_TINY_BOX__

#include "ewa_base/math/tiny_vec.h"

EW_ENTER

template<typename T,int N>
class tiny_box
{
public:
	tiny_vec<T,N> lo,hi;

	operator T*(){return &lo[0];};
	operator const T*() const {return &lo[0];}

	void load_min()
	{
		lo.seta(+std::numeric_limits<double>::max());
		hi.seta(-std::numeric_limits<double>::max());
	}

	void load_max()
	{
		lo.seta(-std::numeric_limits<double>::max());
		hi.seta(+std::numeric_limits<double>::max());
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

};

EW_LEAVE

#endif
