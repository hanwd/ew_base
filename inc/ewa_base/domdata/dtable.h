#ifndef __H_EW_DOMDATA_DTABLE__
#define __H_EW_DOMDATA_DTABLE__

#include "ewa_base/domdata/dobject.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE FigArray : public FigGroupT<DObject, DObject>
{
public:

	DECLARE_OBJECT_INFO(FigArray, DObjectInfo);
};


class DLLIMPEXP_EWA_BASE DTableCell : public FigGroupT<DObject, DObject>
{
public:

	typedef FigGroupT<DObject, DObject> basetype;

	DTableCell(const String& name = "") :basetype(name){ }


	DECLARE_OBJECT_INFO(DTableCell, DObjectInfo);

};


class DLLIMPEXP_EWA_BASE DTableRow : public FigGroupT<DTableCell, DObject>
{
public:

	typedef FigGroupT<DTableCell, DObject> basetype;
	DTableRow(const String& name = "") :basetype(name){ }

	DECLARE_OBJECT_INFO(DTableRow, DObjectInfo);

};

class DLLIMPEXP_EWA_BASE DTable : public FigGroupT<DTableRow, DObject>
{
public:

	typedef FigGroupT<DTableRow, DObject> basetype;
	DTable(const String& name = "") :basetype(name){ }

	DECLARE_OBJECT_INFO(DTable, DObjectInfo);

};

EW_LEAVE

#endif
