
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
	m_pAxis.reset(new DAxis);
	m_pAxis->SetMode(DAxis::MODE_2D_BOX);

	m_aItems.append(m_pAxis.get());
	m_aItems.append(m_pDataManager.get());

}

DCoord3D::DCoord3D(int m)
{
	mode = m;

	m_pAxis.reset(new DAxis);
	m_pAxis->SetMode(DAxis::MODE_3D_DIR);

	m_aItems.append(m_pAxis.get());
	m_aItems.append(m_pDataManager.get());
}

bool DCoord3D::DoGetChildren(DChildrenState& cs)
{
	if (mode == 0)
	{
		cs.set_array((DChildrenState::grp_type&)m_aItems.proxy());
	}
	else
	{
		return m_pDataManager->DoGetChildren(cs);
	}

	return true;
}


IMPLEMENT_OBJECT_INFO(DCoord, DObjectInfo);
IMPLEMENT_OBJECT_INFO(DCoord2D, DObjectInfo);
IMPLEMENT_OBJECT_INFO(DCoord3D, DObjectInfo);

EW_LEAVE
