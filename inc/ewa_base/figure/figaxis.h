
#ifndef __FIGURE_FIGAXIS_H__
#define __FIGURE_FIGAXIS_H__

#include "ewa_base/figure/figitem.h"


EW_ENTER


class DLLIMPEXP_EWA_BASE AxisUnit : public FigItem
{
public:

	typedef FigItem basetype;

	enum
	{
		FLAG_MIN = basetype::FLAG_MAX,
		FLAG_SHOW_MESH_MAIN = FLAG_MIN << 0,
		FLAG_SHOW_MESH_USER = FLAG_MIN << 1,
		FLAG_MAX = FLAG_MIN << 2,
	};

	AxisUnit(const String& name="");

	class Tick
	{
	public:
		enum
		{
			TICK_MAIN=1,
		};
		BitFlags flags;

		Tick();
		Tick(double v);

		void set(double v);

		double m_nValue;
		String m_sLabel;

		int m_nType;
	};

	DFontStyle font;
	arr_1t<Tick> ticks;

	DLineStyle LineMain;
	DLineStyle LineUser;
	DLineStyle LineTick;
	DFontStyle FontText;
};

class DLLIMPEXP_EWA_BASE FigAxis : public FigItem
{
public:
	typedef FigItem basetype;

	FigAxis(const String& name="");

	double rmin;
	double rmax;
};



class DLLIMPEXP_EWA_BASE AxisUnitD : public AxisUnit
{
public:
	typedef AxisUnit basetype;

	AxisUnitD(const String& name="",int d=0);
	int m_nDirection;

	enum
	{
		FLAG_MIN = basetype::FLAG_MAX,
		FLAG_LABEL_DIR_MAX = FLAG_MIN << 0,
		FLAG_MAX = FLAG_MIN << 1,
	};

	DECLARE_OBJECT_INFO(AxisUnitD, ObjectSymbolInfo);
};

class DLLIMPEXP_EWA_BASE AxisUnitP : public AxisUnit
{
public:

	AxisUnitP(const String& name = "") :AxisUnit(name){}

	DECLARE_OBJECT_INFO(AxisUnitP, ObjectSymbolInfo);
};

class DLLIMPEXP_EWA_BASE AxisUnitR : public AxisUnit
{
public:

	AxisUnitR(const String& name = "");

	double m_nRadiusTextAngle; //deg

	DECLARE_OBJECT_INFO(AxisUnitR, ObjectSymbolInfo);
};


EW_LEAVE

#endif
