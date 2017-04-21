
#include "ewa_base/figure/fig.h"
#include "ewa_base/figure/figdata.h"
#include "ewa_base/figure/figcoord.h"


EW_ENTER




bool FigFigure::DoGetChildren(arr_1t<DataPtrT<CallableSymbol> >* p)
{
	if (!p) return true;

	p->clear();
	if (m_pItem)
	{
		p->append(m_pItem.get());
	}

	return true;
}




IMPLEMENT_OBJECT_INFO(FigFigure, ObjectSymbolInfo);

EW_LEAVE

