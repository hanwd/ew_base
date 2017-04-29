#ifndef __H_EW_DOMDATA_DUNIT__
#define __H_EW_DOMDATA_DUNIT__

#include "ewa_base/domdata/dobject.h"


EW_ENTER


class KUnit
{
public:
	KUnit();

	double ul, ut, uf;

	KUnit& operator/=(const KUnit& ku)
	{
		ul /= ku.ul;
		ut /= ku.ut;
		uf /= ku.uf;
		return *this;
	}
};

class DUnit : public DObject
{
public:

	DUnit();

	String ul, ut, uf;

	KUnit ku;

	BitFlags flags;

	virtual bool DoUpdateValue(DState& dp);
	virtual bool DoTransferData(TableSerializer& ar);

	DECLARE_OBJECT_INFO(DUnit, DObjectInfo)

};



EW_LEAVE

#endif
