#include "ewa_base/domdata/dunit.h"
#include "ewa_base/domdata/dstate.h"
#include "ewa_base/domdata/table_serializer.h"

EW_ENTER

KUnit::KUnit()
{
	ul = 1.0e-3;
	uf = 1.0e9;
	ut = 0.5*ul / 3.0e8;
}

DUnit::DUnit()
{

}


bool DUnit::DoUpdateValue(DState& dp)
{

	if (dp.phase == DPHASE_VAR)
	{
		dp.link(ul, ku.ul);
		dp.link(uf, ku.uf);
		dp.link(ut, ku.ut);
	}

	return true;
}


bool DUnit::DoTransferData(TableSerializer& ar)
{

	ar.link("ul", ul);
	ar.link("ut", ut);
	ar.link("uf", uf);

	ar.link("flags", *(int*)&flags);

	return true;
}



IMPLEMENT_OBJECT_INFO(DUnit, ObjectInfo)


EW_LEAVE

