#ifndef __FIGURE_FIGTABLE_H__
#define __FIGURE_FIGTABLE_H__

#include "ewa_base/figure/figitem.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE FigArray : public FigGroupT<FigItem, FigItem>
{
public:

	DECLARE_OBJECT_INFO(FigArray, ObjectSymbolInfo);
};


class DLLIMPEXP_EWA_BASE FigTableCell : public FigGroupT<FigItem, FigItem>
{
public:

	typedef FigGroupT<FigItem, FigItem> basetype;

	FigTableCell(const String& name = "") :basetype(name){ }


	DECLARE_OBJECT_INFO(FigTableCell, ObjectSymbolInfo);

};


class DLLIMPEXP_EWA_BASE FigTableRow : public FigGroupT<FigTableCell, FigItem>
{
public:

	typedef FigGroupT<FigTableCell, FigItem> basetype;
	FigTableRow(const String& name = "") :basetype(name){ }

	DECLARE_OBJECT_INFO(FigTableRow, ObjectSymbolInfo);

};

class DLLIMPEXP_EWA_BASE FigTable : public FigGroupT<FigTableRow, FigItem>
{
public:

	typedef FigGroupT<FigTableRow, FigItem> basetype;
	FigTable(const String& name = "") :basetype(name){ }

	DECLARE_OBJECT_INFO(FigTable, ObjectSymbolInfo);

};

EW_LEAVE

#endif
