
#include "ewa_base/figure/fig.h"


EW_ENTER


DLight::DLight()
{
	light_index = -1;
}


DLineStyle::DLineStyle()
{
	ntype = LINE_SOLID;
	nsize = 1.0;
	color.set(0, 0, 0);
}

DLineStyle::DLineStyle(double w)
{
	ntype = LINE_SOLID;
	nsize = w;
	color.set(0, 0, 0);
}

void DLineStyle::set(int t, float w, const DColor &c)
{
	ntype = t;
	nsize = w;
	color = c;
}
void DLineStyle::set(int t, float w)
{
	ntype = t;
	nsize = w;
}

DColor::DColor(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_)
{
	set(r_, g_, b_, a_);
}

DColor::DColor()
{
	set(0.0f, 0.0f, 0.0f, 1.0f);
}



DColor::operator const uint8_t *() const
{
	return &r;
}

void DColor::set(uint8_t r_, uint8_t g_, uint8_t b_)
{
	r = r_;
	g = g_;
	b = b_;
}

void DColor::set(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_)
{
	r = r_;
	g = g_;
	b = b_;
	a = a_;
}

DFontStyle::DFontStyle()
{
	_init();
}
DFontStyle::DFontStyle(float size)
{
	_init();
	nsize = size;
}

void DFontStyle::_init()
{
	nsize = 16.0;
	color.set(0, 0, 0);
}


FigData2D::FigData2D()
{
	m_nDataType = TYPE_PLOT;

	size_t n = 2000;
	m_aTdata.resize(n);
	m_aValue.resize(n);
	for (size_t i = 0; i < m_aTdata.size(); i++)
	{
		m_aTdata[i] = double(i)-300;
		m_aValue[i] = 30.0*(sin(double(i)*0.3))*(0.5 + double(i) / 150.0);
	}

}

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

IMPLEMENT_OBJECT_INFO(FigItem, ObjectSymbolInfo);
IMPLEMENT_OBJECT_INFO(FigTableCell, ObjectSymbolInfo);
IMPLEMENT_OBJECT_INFO(FigTableRow, ObjectSymbolInfo);
IMPLEMENT_OBJECT_INFO(FigTable, ObjectSymbolInfo);
IMPLEMENT_OBJECT_INFO(FigArray, ObjectSymbolInfo);
IMPLEMENT_OBJECT_INFO(FigText, ObjectSymbolInfo);
IMPLEMENT_OBJECT_INFO(FigData, ObjectSymbolInfo);
IMPLEMENT_OBJECT_INFO(FigData2D, ObjectSymbolInfo);
IMPLEMENT_OBJECT_INFO(FigCoord, ObjectSymbolInfo);
IMPLEMENT_OBJECT_INFO(FigCoord2D, ObjectSymbolInfo);
IMPLEMENT_OBJECT_INFO(FigFigure, ObjectSymbolInfo);


EW_LEAVE

