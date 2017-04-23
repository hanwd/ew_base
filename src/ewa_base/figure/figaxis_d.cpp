
#include "ewa_base/figure/figaxis_d.h"

EW_ENTER

FigAxisD::FigAxisD(const String& name) :basetype(name)
{
	AxisUnitD* px = new AxisUnitD("X", 0);
	m_aItems.append(px);

	AxisUnitD* py = new AxisUnitD("Y", 1);
	py->font.flags.add(DFontStyle::STYLE_VERTICAL);
	m_aItems.append(py);
}


IMPLEMENT_OBJECT_INFO(FigAxisD, ObjectSymbolInfo);

EW_LEAVE
