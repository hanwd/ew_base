
#include "ewa_base/domdata/dfigure.h"

EW_ENTER
bool DFigure::DoGetChildren(arr_1t<DataPtrT<DObject> >* p)
{
	if (!p) return true;

	p->clear();
	if (m_pItem)
	{
		p->append(m_pItem.get());
	}

	return true;
}


IMPLEMENT_OBJECT_INFO(DFigure, DObjectInfo);

EW_LEAVE

