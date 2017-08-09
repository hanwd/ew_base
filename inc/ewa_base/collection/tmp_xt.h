#ifndef __H_EW_COLLECTION_TMP_XT__
#define __H_EW_COLLECTION_TMP_XT__


#include "ewa_base/collection/arr_xt.h"

EW_ENTER

class tmp_xt_base
{
public:
	typedef intptr_t size_type;
	static const int MAX_DIM=6;
};

class tmp_xt_dims : public tmp_xt_base
{
public:

	intptr_t sz[MAX_DIM];
	intptr_t lo[MAX_DIM];
	intptr_t hi[MAX_DIM];

};


class tmp_xt_data : public tmp_xt_base
{
public:

	intptr_t ds[MAX_DIM];
	void* pt;

};


template<typename T>
class tmp_xt
{
public:
	typedef intptr_t size_type;

	inline const T& operator()(size_type k0) const
	{
		return base[k0*dims.sz[0]];
	}

	inline const T& operator()(size_type k0,size_type k1) const
	{
		return base[k0*dims.sz[0]+k1*dims.sz[1]];
	}

	inline const T& operator()(size_type k0,size_type k1,size_type k2) const
	{
		return base[k0*dims.sz[0]+k1*dims.sz[1]+k2*dims.sz[2]];
	}

	inline T& operator()(size_type k0)
	{
		return base[k0*dims.sz[0]];
	}

	inline T& operator()(size_type k0,size_type k1)
	{
		return base[k0*dims.sz[0]+k1*dims.sz[1]];
	}

	inline T& operator()(size_type k0,size_type k1,size_type k2)
	{
		return base[k0*dims.sz[0]+k1*dims.sz[1]+k2*dims.sz[2]];
	}	
	
	void set_dim(int d,int lo,int hi,int o)
	{
		if(d<0||d>=tmp_xt_dims::MAX_DIM)
		{
			Exception::XError("invalid dim");
		}
		dims.lo[d]=lo;
		dims.hi[d]=hi;
		dims.sz[d]=data.ds[o];
	}

	void update()
	{
		base=(T*)data.pt;
		base-=dims.lo[0]*dims.sz[0];
		base-=dims.lo[1]*dims.sz[1];
		base-=dims.lo[2]*dims.sz[2];
		base-=dims.lo[3]*dims.sz[3];
		base-=dims.lo[4]*dims.sz[4];
		base-=dims.lo[5]*dims.sz[5];
	}

	tmp_xt() :base(NULL)
	{

	}

	tmp_xt(arr_xt<T>& a)
	{
		reset(a);
	}

	void reset(arr_xt<T>& a)
	{
		data.pt=a.data();
		for(size_t i=0;i<6;i++)
		{
			data.ds[i]=i==0?1:data.ds[i-1]*a.size(i-1);
			set_dim(i,0,a.size(i),i);
		}
		update();
	}

	int lo(size_t d) const
	{
		return dims.lo[d];
	}

	int hi(size_t d) const
	{
		return dims.hi[d];
	}

protected:

	tmp_xt_dims dims;
	tmp_xt_data data;
	T* base;

};


EW_LEAVE

#endif
