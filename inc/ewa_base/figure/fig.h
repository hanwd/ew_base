
#ifndef __FIG_FIG_H__
#define __FIG_FIG_H__


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
		STYLE_VERTICAL	= 1 << 0,
		STYLE_ITALIC	= 1 << 1,
		STYLE_UNDERLINE = 1 << 2,
		STYLE_STRIDE	= 1 << 3,
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
	vec4f v4deffuse;
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


template<typename T,typename B>
class FigGroupT : public B
{
public:
	FigGroupT(const String& name="") :B(name){}

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

class DLLIMPEXP_EWA_BASE FigTableCell : public FigGroupT<FigItem, FigItem>
{
public:

	typedef FigGroupT<FigItem, FigItem> basetype;

	FigTableCell(const String& name="") :basetype(name){ }


	DECLARE_OBJECT_INFO(FigTableCell, ObjectSymbolInfo);

};


class DLLIMPEXP_EWA_BASE FigTableRow : public FigGroupT<FigTableCell, FigItem>
{
public:

	typedef FigGroupT<FigTableCell, FigItem> basetype;
	FigTableRow(const String& name="") :basetype(name){ }

	DECLARE_OBJECT_INFO(FigTableRow, ObjectSymbolInfo);

};

class DLLIMPEXP_EWA_BASE FigTable : public FigGroupT<FigTableRow, FigItem>
{
public:

	typedef FigGroupT<FigTableRow, FigItem> basetype;
	FigTable(const String& name = "") :basetype(name){ }

	DECLARE_OBJECT_INFO(FigTable, ObjectSymbolInfo);

};

class DLLIMPEXP_EWA_BASE FigText : public FigItem
{
public:

	FigText(const String& name = "") :FigItem(name){  }

	String m_sText;

	DECLARE_OBJECT_INFO(FigText, ObjectSymbolInfo);

};



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
	tiny_box<double,1> radius;
	tiny_box<double,1> values;

};

class DLLIMPEXP_EWA_BASE FigData : public FigItem
{
public:
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

	DECLARE_OBJECT_INFO(FigData2D, ObjectSymbolInfo);
};



class DLLIMPEXP_EWA_BASE FigCoord : public FigGroupT<FigData, FigItem>
{
public:

	DECLARE_OBJECT_INFO(FigCoord, ObjectSymbolInfo);
};

class DLLIMPEXP_EWA_BASE FigCoord2D : public FigCoord
{
public:

	DECLARE_OBJECT_INFO(FigCoord2D, ObjectSymbolInfo);
};

class DLLIMPEXP_EWA_BASE FigArray : public FigGroupT<FigItem, FigItem>
{
public:

	DECLARE_OBJECT_INFO(FigArray, ObjectSymbolInfo);
};

class DLLIMPEXP_EWA_BASE FigFigure : public FigItem
{
public:

	DataPtrT<FigItem> m_pItem;


	bool DoGetChildren(arr_1t<DataPtrT<CallableSymbol> >* p);

	DECLARE_OBJECT_INFO(FigFigure, ObjectSymbolInfo);
};


EW_LEAVE
#endif
