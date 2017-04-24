
#ifndef __FIGURE_FIG_H__
#define __FIGURE_FIG_H__

#include "ewa_base/figure/figitem.h"
#include "ewa_base/figure/figaxis_d.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE FigFigure : public FigItem
{
public:

	DataPtrT<FigItem> m_pItem;


	bool DoGetChildren(arr_1t<DataPtrT<CallableSymbol> >* p);

	DECLARE_OBJECT_INFO(FigFigure, ObjectSymbolInfo);
};


EW_LEAVE
#endif
