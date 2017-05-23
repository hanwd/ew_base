
#ifndef __H_EW_DOMDATA_DFIGURE__
#define __H_EW_DOMDATA_DFIGURE__

#include "ewa_base/domdata/dobject.h"
#include "ewa_base/domdata/dcoord.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE DFigure : public DObjectBox
{
public:

	DataPtrT<DCoord> m_pItem;

	bool DoGetChildren(arr_1t<DataPtrT<DObject> >* p);

	DECLARE_OBJECT_INFO(DFigure, DObjectInfo);
};


EW_LEAVE
#endif
