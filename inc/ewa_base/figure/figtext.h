#ifndef __FIGURE_FIGTEXT_H__
#define __FIGURE_FIGTEXT_H__

#include "ewa_base/figure/figitem.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE FigText : public FigItem
{
public:

	FigText(const String& name = "") :FigItem(name){  }

	String m_sText;

	DECLARE_OBJECT_INFO(FigText, ObjectSymbolInfo);

};


EW_LEAVE

#endif
