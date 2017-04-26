
#include "ewa_base/figure/fig.h"
#include "ewa_base/figure/figdata.h"
#include "ewa_base/figure/figcoord.h"

EW_ENTER

FigCoord::FigCoord()
{
	m_pDataManager.reset(new FigDataManager);

}

FigCoord2D::FigCoord2D()
{
	m_pAxis.reset(new FigAxisD);

	m_aItems.append(m_pAxis.get());
	m_aItems.append(m_pDataManager.get());

}

FigCoord3D::FigCoord3D()
{
	//m_pAxis.reset(new FigAxisD);
	m_aItems.append(m_pDataManager.get());

}

IMPLEMENT_OBJECT_INFO(FigCoord, ObjectSymbolInfo);
IMPLEMENT_OBJECT_INFO(FigCoord2D, ObjectSymbolInfo);
IMPLEMENT_OBJECT_INFO(FigCoord3D, ObjectSymbolInfo);

EW_LEAVE
