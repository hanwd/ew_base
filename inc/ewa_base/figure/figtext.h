#ifndef __FIGURE_FIGTEXT_H__
#define __FIGURE_FIGTEXT_H__

#include "ewa_base/figure/figitem.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE FigText : public FigItem
{
public:

	FigText(const String& name = "") :FigItem(name)
	{ 
		m_tFont.color.set(255, 255, 255);
	}

	String m_sText;

	vec3d m_v3Pos;
	vec3d m_v3Pxl;

	vec3d m_v3Shf;

	DFontStyle m_tFont;

	DECLARE_OBJECT_INFO(FigText, ObjectSymbolInfo);

};


EW_LEAVE

#endif
