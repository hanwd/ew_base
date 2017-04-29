
#ifndef __H_EW_DOMDATA_DCOORD__
#define __H_EW_DOMDATA_DCOORD__

#include "ewa_base/domdata/dobject.h"
#include "ewa_base/domdata/daxis_d.h"
#include "ewa_base/domdata/dfigdata.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE DCoord : public FigGroupT<DObject, DObjectBox>
{
public:

	typedef FigGroupT<DObject, DObjectBox> basetype;

	DCoord();

	DataPtrT<DFigDataManager> m_pDataManager;
	DataPtrT<FigAxis> m_pAxis;

	DECLARE_OBJECT_INFO(DCoord, DObjectInfo);
};

class DLLIMPEXP_EWA_BASE DCoord2D : public DCoord
{
public:

	DCoord2D();

	DECLARE_OBJECT_INFO(DCoord2D, DObjectInfo);
};

class DLLIMPEXP_EWA_BASE DCoord3D : public DCoord
{
public:

	DCoord3D();

	DECLARE_OBJECT_INFO(DCoord3D, DObjectInfo);
};


EW_LEAVE
#endif
