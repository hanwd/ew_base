
#ifndef __FIGURE_FIGITEM_H__
#define __FIGURE_FIGITEM_H__


#include "ewa_base/basic.h"
#include "ewa_base/util/symm.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE DColor
{
public:

	DColor(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255u);
	DColor();

	operator const uint8_t*() const;

	void set(uint8_t r_, uint8_t g_, uint8_t b_);
	void set(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_);

	uint8_t r, g, b, a;
};


class DLLIMPEXP_EWA_BASE DFontStyle
{
public:

	DFontStyle();
	DFontStyle(float size);

	float nsize;
	String sname;
	DColor color;
	BitFlags flags;

	enum
	{
		STYLE_VERTICAL = 1 << 0,
		STYLE_ITALIC = 1 << 1,
		STYLE_UNDERLINE = 1 << 2,
		STYLE_STRIDE = 1 << 3,
	};
private:
	void _init();
};


class DLLIMPEXP_EWA_BASE DLineStyle
{
public:

	enum
	{
		LINE_NONE,
		LINE_SOLID,
		LINE_DOT1,
		LINE_DOT2,
		LINE_DASH1,
		LINE_DASH2,
		LINE_DASH3,
		LINE_DASH4,
	};

	DLineStyle();
	DLineStyle(double w);

	void set(int t, float w, const DColor& c);
	void set(int t, float w = 1.0);

	int ntype;
	float nsize;
	DColor color;
};


class DLLIMPEXP_EWA_BASE DLight
{
public:

	DLight();

	int light_index;
	vec4f v4ambient;
	vec4f v4diffuse;
	vec4f v4specular;
	vec4f v4position;
};


class DLLIMPEXP_EWA_BASE FigItem : public CallableSymbol
{
public:
	typedef CallableSymbol basetype;
	FigItem(const String& name = "") :CallableSymbol(name){}


	DECLARE_OBJECT_INFO(FigItem, ObjectSymbolInfo);
};

class DLLIMPEXP_EWA_BASE FigBBox : public FigItem
{
public:
	typedef CallableSymbol basetype;
	FigBBox(const String& name = "") :FigItem(name){}


	DECLARE_OBJECT_INFO(FigBBox, ObjectSymbolInfo);
};



template<typename T, typename B>
class FigGroupT : public B
{
public:
	FigGroupT(const String& name = "") :B(name){}

	ObjectGroupT<T> m_aItems;

	virtual bool DoGetChildren(arr_1t<DataPtrT<CallableSymbol> >* p)
	{
		if (p)
		{
			p->assign(m_aItems.begin(), m_aItems.end());
		}
		return true;
	}
};


EW_LEAVE
#endif
