#ifndef __H_EW_MATH_RANDOM__
#define __H_EW_MATH_RANDOM__

#include "ewa_base/basic/object.h"
#include "ewa_base/basic/pointer.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE rand_gen
{
public:

	rand_gen();

	uint32_t random_int(uint32_t v);
	double random_double();

	uint32_t operator()();

	void seed();
	void seed(double v);
	void seed(uint32_t v);
	void seed(uint32_t* p,int n);

	void save_state();
	void load_state();
	void load_state(uint32_t i,uint32_t j=0,uint32_t k=0);

protected:
	DataPtrT<ObjectData> m_pData1,m_pData2;
};

EW_LEAVE
#endif
