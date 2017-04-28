#ifndef __EW_EWA_BASE_UNIT_H__
#define __EW_EWA_BASE_UNIT_H__

#include "ewa_base/util/symm.h"


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

class DUnit : public CallableSymbol
{
public:

	DUnit();

	String ul, ut, uf;

	KUnit ku;

	BitFlags flags;

	virtual bool DoUpdateValue(DState& dp);
	virtual bool DoTransferData(TableSerializer& ar);

	DECLARE_OBJECT_INFO(DUnit, ObjectInfo)

};



EW_LEAVE

#endif
