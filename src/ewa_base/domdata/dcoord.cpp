
#include "ewa_base/domdata/dfigure.h"
#include "ewa_base/domdata/dfigdata.h"
#include "ewa_base/domdata/dcoord.h"

EW_ENTER

DCoord::DCoord()
{
	m_pDataManager.reset(new DFigDataManager);

}

DCoord2D::DCoord2D()
{
	m_pAxis.reset(new DAxisD);

	m_aItems.append(m_pAxis.get());
	m_aItems.append(m_pDataManager.get());

}

DCoord3D::DCoord3D()
{
	//m_pAxis.reset(new DAxisD);
	m_aItems.append(m_pDataManager.get());

}

IMPLEMENT_OBJECT_INFO(DCoord, DObjectInfo);
IMPLEMENT_OBJECT_INFO(DCoord2D, DObjectInfo);
IMPLEMENT_OBJECT_INFO(DCoord3D, DObjectInfo);

EW_LEAVE
