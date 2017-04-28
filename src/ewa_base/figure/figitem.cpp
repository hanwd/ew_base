#include "ewa_base/figure/figitem.h"


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
	set(0, 0, 0, 255u);
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


IMPLEMENT_OBJECT_INFO(FigItem, ObjectSymbolInfo);

EW_LEAVE
