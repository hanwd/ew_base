
#ifndef __H_EW_DOMDATA_DFIGDATA__
#define __H_EW_DOMDATA_DFIGDATA__

#include "ewa_base/domdata/dobject.h"

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

class DLLIMPEXP_EWA_BASE FigData : public DObject
{
public:
	DECLARE_OBJECT_INFO(FigData, DObjectInfo);
};

class DLLIMPEXP_EWA_BASE DFigDataManager : public FigGroupT<DObject, DObject>
{
public:
	typedef FigGroupT<DObject, DObject> basetype;

	DFigDataManager(const String& name = "datamgr") :basetype(name){}

	DECLARE_OBJECT_INFO(DFigDataManager, DObjectInfo);
};


class DLLIMPEXP_EWA_BASE DFigData2D : public FigData
{
public:

	enum
	{
		TYPE_PLOT,
		TYPE_POLAR,
	};

	DFigData2D();

	int m_nDataType;
	arr_1t<double> m_aTdata;
	arr_1t<double> m_aValue;


	DECLARE_OBJECT_INFO(DFigData2D, DObjectInfo);
};

class DLLIMPEXP_EWA_BASE DFigData3D : public FigData
{
public:

	DECLARE_OBJECT_INFO(DFigData3D, DObjectInfo);
};


class DLLIMPEXP_EWA_BASE DFigData3DImage : public DFigData3D
{
public:

	arr_xt<double> m_aXvalues;
	arr_xt<double> m_aYvalues;
	arr_xt<double> m_aZvalues;

	void Serialize(SerializerHelper sh)
	{
		Serializer& ar(sh.ref(0));
		ar & m_aXvalues & m_aYvalues & m_aZvalues;
	}

	DECLARE_OBJECT_INFO(DFigData3DImage, DObjectInfo);
};


class DLLIMPEXP_EWA_BASE DFigDataModel : public DFigData3D
{
public:

	DFigDataModel();

	DataPtrT<DObject> m_pRealObject;

	bool DoGetChildren(DChildrenState& cs)
	{
		if (!m_pRealObject) return false;
		return m_pRealObject->DoGetChildren(cs);
	}

	DECLARE_OBJECT_INFO(DFigDataModel, DObjectInfo);
};

EW_LEAVE

#endif
