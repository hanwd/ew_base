
#include "ewa_base/domdata/daxis_d.h"

EW_ENTER

DAxisD::DAxisD(const String& name) :basetype(name)
{
	DAxisUnitD* px = new DAxisUnitD("X", 0);
	m_aItems.append(px);

	DAxisUnitD* py = new DAxisUnitD("Y", 1);
	py->font.flags.add(DFontStyle::STYLE_VERTICAL);
	m_aItems.append(py);
}


IMPLEMENT_OBJECT_INFO(DAxisD, DObjectInfo);

EW_LEAVE
