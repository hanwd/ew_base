#ifndef __H_EW_DOMDATA_DAXIS_D__
#define __H_EW_DOMDATA_DAXIS_D__

#include "ewa_base/domdata/daxisunit.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE DAxisD : public FigGroupT<DAxisUnit, FigAxis>
{
public:

	typedef FigGroupT<DAxisUnit, FigAxis> basetype;

	DAxisD(const String& name="axis_d");

	DECLARE_OBJECT_INFO(DAxisD, DObjectInfo);
};


EW_LEAVE

#endif