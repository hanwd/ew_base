#ifndef __H_EW_SCRIPTING_DETAIL_IDX_1T__
#define __H_EW_SCRIPTING_DETAIL_IDX_1T__

#include "ewa_base/config.h"
#include "ewa_base/math/tiny_cpx.h"

//#include "ewa_base/basic.h"
//#include "ewa_base/collection.h"


EW_ENTER

class DLLIMPEXP_EWA_BASE ColonData;
class DLLIMPEXP_EWA_BASE Variant;

class DLLIMPEXP_EWA_BASE idx_1t
{
public:

	typedef size_t size_type;

	enum
	{
		IDX_NONE,
		IDX_ICOLON,
		IDX_DCOLON,
		IDX_INTPTR,
		IDX_DBLPTR,
		IDX_CPXPTR,
		IDX_VARPTR,
	};

	idx_1t(){type=IDX_NONE;}

	template<typename T>
	int update(const T&,intptr_t,intptr_t){return -1;}

	// array range [n1,n2)
	int update(Variant& d,intptr_t n1,intptr_t n2);
	int update(int64_t d,intptr_t n1,intptr_t n2);
	int update(double d,intptr_t n1,intptr_t n2);
	int update(const dcomplex& d,intptr_t n1,intptr_t n2);
	int update(const String& d,intptr_t n1,intptr_t n2);
	int update(const ColonData& d,intptr_t n1,intptr_t n2);
	int update(const arr_xt<int64_t>& d,intptr_t n1,intptr_t n2);
	int update(const arr_xt<double>& d,intptr_t n1,intptr_t n2);
	int update(const arr_xt<Variant>& d,intptr_t n1,intptr_t n2);
	int update(const arr_xt<dcomplex>& d,intptr_t n1,intptr_t n2);
	
	size_type operator()(size_type i);

	int type;
	size_type size;
	intptr_t imin;
	intptr_t imax;

private:
	union
	{
		double dval[2];
		int64_t ival[2];
		const void *xptr;
	}cdat;

	void _update_colon(double s1,double ds);

	template<typename T>
	int _update_array(int t,const arr_xt<T>& d,intptr_t n1,intptr_t n2);

};

EW_LEAVE

#endif
