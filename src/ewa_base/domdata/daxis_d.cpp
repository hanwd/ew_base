
#include "ewa_base/domdata/daxis_d.h"

EW_ENTER

DAxis::DAxis(const String& name) :basetype(name)
{
	m_nMode=MODE_3D_DIR;
}

void DAxis::SetMode(int mode)
{
	m_nMode=mode;
	m_aItems.clear();

	if(mode==MODE_3D_DIR||mode==MODE_SPHERE||mode==MODE_NONE)
	{

	}
	else if(mode==MODE_3D_BOX)
	{
		m_aItems.append(new DAxisUnitD("Axis X", 0));
		m_aItems.append(new DAxisUnitD("Axis Y", 0));
		m_aItems.append(new DAxisUnitD("Axis Z", 0));
		m_aItems[1]->font.flags.del(DFontStyle::STYLE_VERTICAL);
	}
	else if(mode==MODE_2D_POLAR)
	{
		m_aItems.append(new DAxisUnitR("Radius"));
		m_aItems.append(new DAxisUnitP("Angle"));
	}
	else if(mode==MODE_CYLINDER)
	{
		m_aItems.append(new DAxisUnitR("Radius"));
		m_aItems.append(new DAxisUnitP("Angle"));
		m_aItems.append(new DAxisUnitD("Axis Z", 0));
	}
	else
	{
		mode=MODE_2D_BOX;
		m_aItems.append(new DAxisUnitD("Axis X", 0));
		m_aItems.append(new DAxisUnitD("Axis Y", 1));
		m_aItems[1]->font.flags.add(DFontStyle::STYLE_VERTICAL);
	}

}




IMPLEMENT_OBJECT_INFO(DAxis, DObjectInfo);




EW_LEAVE
