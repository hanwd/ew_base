
#ifndef __FIGURE_FIGDATA_H__
#define __FIGURE_FIGDATA_H__

#include "ewa_base/figure/figitem.h"

EW_ENTER



class DLLIMPEXP_EWA_BASE FigDataAxis
{
public:

	void reset()
	{
		bbox.load_min();
		radius.load_min();
		values.load_min();
	}

	void test_point(double x, double y)
	{
		bbox.add_x(x);
		bbox.add_y(y);
	}

	void test_point(double x, double y, double z)
	{
		bbox.add_x(x);
		bbox.add_y(y);
		bbox.add_z(z);
	}

	void test_point(const vec3d& p)
	{
		bbox.add(p);
	}
	void test_radius(double r)
	{
		radius.add_x(r);
	}

	void test_value(double v)
	{
		values.add_x(v);
	}

	box3d bbox;
	tiny_box<double, 1> radius;
	tiny_box<double, 1> values;

};

class DLLIMPEXP_EWA_BASE FigData : public FigItem
{
public:
	DECLARE_OBJECT_INFO(FigData, ObjectSymbolInfo);
};

class DLLIMPEXP_EWA_BASE FigDataManager : public FigGroupT<FigData,FigItem>
{
public:
	typedef FigGroupT<FigData, FigItem> basetype;

	FigDataManager(const String& name = "datamgr") :basetype(name){}

	DECLARE_OBJECT_INFO(FigData, ObjectSymbolInfo);
};

class DLLIMPEXP_EWA_BASE FigData2D : public FigData
{
public:

	enum
	{
		TYPE_PLOT,
		TYPE_POLAR,
	};

	FigData2D();

	int m_nDataType;
	arr_1t<double> m_aTdata;
	arr_1t<double> m_aValue;

	DLineStyle LineType;

	DECLARE_OBJECT_INFO(FigData2D, ObjectSymbolInfo);
};

class DLLIMPEXP_EWA_BASE FigData3D : public FigData
{
public:

	DECLARE_OBJECT_INFO(FigData3D, ObjectSymbolInfo);
};

EW_LEAVE

#endif
