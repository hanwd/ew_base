
#include "ewa_base/figure/figaxis.h"

EW_ENTER
	
AxisUnit::Tick::Tick()
{
	m_nType=0;
}

AxisUnit::Tick::Tick(double v)
{
	m_nType=TICK_MAIN;
	set(v);
}

void AxisUnit::Tick::set(double v)
{
	m_nValue=v;
	m_sLabel=String::Format("%.6g",v);
}

AxisUnit::AxisUnit(const String& name):basetype(name)
{
	flags.add(FLAG_SHOW_MESH_MAIN);

	m_sId=name;
	LineUser.nsize=0.5;
	LineMain.ntype=DLineStyle::LINE_DASH3;
	LineUser.ntype=DLineStyle::LINE_DASH4;
}

AxisUnitD::AxisUnitD(const String& name,int d):basetype(name)
{
	m_nDirection=d;
}

AxisUnitR::AxisUnitR(const String& name) :AxisUnit(name)
{
	m_nRadiusTextAngle = 0.5*(30.0 + 45.0);
}

FigAxis::FigAxis(const String& name)
:basetype(name)
{
	rmin=-30.0;
}

IMPLEMENT_OBJECT_INFO(AxisUnitD, ObjectSymbolInfo);
IMPLEMENT_OBJECT_INFO(AxisUnitP, ObjectSymbolInfo);
IMPLEMENT_OBJECT_INFO(AxisUnitR, ObjectSymbolInfo);

EW_LEAVE

