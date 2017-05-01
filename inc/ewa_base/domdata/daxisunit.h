
#ifndef __H_EW_DOMDATA_DAXISUNIT__
#define __H_EW_DOMDATA_DAXISUNIT__

#include "ewa_base/domdata/dobject.h"


EW_ENTER


class DLLIMPEXP_EWA_BASE DAxisUnit : public DObject
{
public:

	typedef DObject basetype;

	enum
	{
		FLAG_MIN = basetype::FLAG_MAX,
		FLAG_SHOW_MESH_MAIN = FLAG_MIN << 0,
		FLAG_SHOW_MESH_USER = FLAG_MIN << 1,
		FLAG_MAX = FLAG_MIN << 2,
	};

	DAxisUnit(const String& name="");

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


class DLLIMPEXP_EWA_BASE DAxisUnitD : public DAxisUnit
{
public:
	typedef DAxisUnit basetype;

	DAxisUnitD(const String& name="",int d=0);
	int m_nDirection;

	enum
	{
		FLAG_MIN = basetype::FLAG_MAX,
		FLAG_LABEL_DIR_MAX = FLAG_MIN << 0,
		FLAG_MAX = FLAG_MIN << 1,
	};

	DECLARE_OBJECT_INFO(DAxisUnitD, DObjectInfo);
};

class DLLIMPEXP_EWA_BASE DAxisUnitP : public DAxisUnit
{
public:

	DAxisUnitP(const String& name = "") :DAxisUnit(name){}

	DECLARE_OBJECT_INFO(DAxisUnitP, DObjectInfo);
};

class DLLIMPEXP_EWA_BASE DAxisUnitR : public DAxisUnit
{
public:

	DAxisUnitR(const String& name = "");

	double m_nRadiusTextAngle; //deg

	DECLARE_OBJECT_INFO(DAxisUnitR, DObjectInfo);
};



EW_LEAVE

#endif
