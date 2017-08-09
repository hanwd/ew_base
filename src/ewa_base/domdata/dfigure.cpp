
#include "ewa_base/domdata/dfigure.h"

EW_ENTER

bool DFigure::DoGetChildren(DChildrenState& dpm)
{

	if (!m_pItem)
	{
		return false;
	}

	dpm.set_item(m_pItem.get());
	return true;
}


IMPLEMENT_OBJECT_INFO(DFigure, DObjectInfo);

EW_LEAVE

