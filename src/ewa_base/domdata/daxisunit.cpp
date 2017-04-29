
#include "ewa_base/domdata/daxisunit.h"

EW_ENTER
	
DAxisUnit::Tick::Tick()
{
	m_nType=0;
}

DAxisUnit::Tick::Tick(double v)
{
	m_nType=TICK_MAIN;
	set(v);
}

void DAxisUnit::Tick::set(double v)
{
	m_nValue=v;
	m_sLabel=String::Format("%.6g",v);
}

DAxisUnit::DAxisUnit(const String& name):basetype(name)
{
	flags.add(FLAG_SHOW_MESH_MAIN);

	m_sId=name;
	LineUser.nsize=0.5;
	LineMain.ntype=DLineStyle::LINE_DASH3;
	LineUser.ntype=DLineStyle::LINE_DASH4;
}

DAxisUnitD::DAxisUnitD(const String& name,int d):basetype(name)
{
	m_nDirection=d;
	FontText.color.set(127, 127, 127);
}

AxisUnitR::AxisUnitR(const String& name) :DAxisUnit(name)
{
	m_nRadiusTextAngle = 0.5*(30.0 + 45.0);
}

FigAxis::FigAxis(const String& name)
:basetype(name)
{
	rmin=-30.0;
}

IMPLEMENT_OBJECT_INFO(DAxisUnitD, DObjectInfo);
IMPLEMENT_OBJECT_INFO(AxisUnitP, DObjectInfo);
IMPLEMENT_OBJECT_INFO(AxisUnitR, DObjectInfo);

EW_LEAVE

