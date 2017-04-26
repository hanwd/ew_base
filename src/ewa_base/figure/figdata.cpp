
#include "ewa_base/figure/fig.h"
#include "ewa_base/figure/figdata.h"
#include "ewa_base/figure/figcoord.h"

EW_ENTER


FigData2D::FigData2D()
{
	m_nDataType = TYPE_PLOT;

	static int g_ndata=0;

	g_ndata++;

	size_t n = 2000;
	m_aTdata.resize(n);
	m_aValue.resize(n);
	for (size_t i = 0; i < m_aTdata.size(); i++)
	{
		m_aTdata[i] = double(i) - 300;
		m_aValue[i] = 50.0*(sin(double(g_ndata)+double(i)*0.3))*(0.5 + double(i) / 150.0);
	}	


}

IMPLEMENT_OBJECT_INFO(FigData, ObjectSymbolInfo);
IMPLEMENT_OBJECT_INFO(FigData2D, ObjectSymbolInfo);
IMPLEMENT_OBJECT_INFO(FigData3D, ObjectSymbolInfo);

IMPLEMENT_OBJECT_INFO(FigDataManager, ObjectSymbolInfo);

EW_LEAVE
