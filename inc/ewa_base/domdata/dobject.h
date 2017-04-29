
#ifndef __H_EW_DOMDATA_DOBJECT__
#define __H_EW_DOMDATA_DOBJECT__


#include "ewa_base/basic.h"
#include "ewa_base/scripting.h"
#include "ewa_base/math/math_def.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE DState;
class DLLIMPEXP_EWA_BASE DContext;
class DLLIMPEXP_EWA_BASE TableSerializer;


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


class DLLIMPEXP_EWA_BASE DExprItem
{
public:
	String name,value,desc;

	DExprItem(const String& n,const String& v,const String& d=""):name(n),value(v),desc(d){}
	DExprItem(){}

	void SerializeVariant(Variant& v,int dir);

	void Serialize(SerializerHelper sh);
};

inline bool operator==(const DExprItem& lhs,const DExprItem& rhs)
{
	return lhs.name==rhs.name && lhs.value==rhs.value && lhs.desc==rhs.desc;
}
inline bool operator!=(const DExprItem& lhs,const DExprItem& rhs)
{
	return !(lhs==rhs);
}



template<>
class hash_t<DExprItem>
{
public:
	inline uint32_t operator()(const DExprItem& val)
	{
		hash_t<String> h2;
		return h2(val.name)^h2(val.value)^h2(val.desc);
	}
};


DEFINE_OBJECT_NAME(DExprItem,"expritem");



class DLLIMPEXP_EWA_BASE DWhen
{
public:

	int t_begin,t_end,t_step;

	DWhen()
	{
		t_begin=t_end=0;
		t_step=-1;
	}
};


class DLLIMPEXP_EWA_BASE DMatrixBox
{
public:

	DMatrixBox()
	{
		m4.LoadIdentity();
		b3.load_min();
	}

	DMatrixBox(const mat4d& m):m4(m)
	{
		b3.load_min();
	}


	mat4d m4;
	box3d b3;
};



class DLLIMPEXP_EWA_BASE SymbolItem
{
public:
	SymbolItem(const String& n = "");

	String name;
	arr_1t<Variant> prop;

	void SerializeVariant(Variant& v, int dir);

	void Serialize(SerializerHelper sh);
};


template<>
class hash_t<SymbolItem>
{
public:
	uint32_t operator()(const SymbolItem& item);
};

DEFINE_OBJECT_NAME(SymbolItem,"SymbolItem")

inline bool operator==(const SymbolItem& lhs,const SymbolItem& rhs)
{
	return lhs.name==rhs.name && lhs.prop==rhs.prop;
}
inline bool operator!=(const SymbolItem& lhs,const SymbolItem& rhs)
{
	return !(lhs==rhs);
}

template<>
class DLLIMPEXP_EWA_BASE CallableWrapT<arr_1t<SymbolItem> > : public CallableDataBaseT<arr_1t<SymbolItem> >
{
public:
	typedef arr_1t<SymbolItem> type;
	typedef CallableDataBaseT<type> basetype;

	CallableWrapT():basetype(){}
	CallableWrapT(const type& v):basetype(v){}
	virtual CallableData* DoClone(ObjectCloneState&){return new CallableWrapT(value);}
};



class DLLIMPEXP_EWA_BASE DObjectInfo : public ObjectInfo
{
public:

	DObjectInfo(const String& s = "") :ObjectInfo(s)
	{

	}

};


class DLLIMPEXP_EWA_BASE DObject : public CallableData
{
public:
	DObject(const String& s=""):m_sId(s){}

	enum
	{
		FLAG_MIN		=1<<0,
		FLAG_UPDATED	=FLAG_MIN<<0,
		FLAG_ERROR		=FLAG_MIN<<1,
		FLAG_MAX		=FLAG_MIN<<2,
	};

	String m_sId;
	BitFlags flags;

	virtual bool DoUpdateValue(DState&){return true;}
	virtual bool DoCheckParam(DState&){return true;}
	virtual bool DoTransferData(TableSerializer&){ return true; }
	virtual void DoRender(DContext&){}
	virtual bool DoGetChildren(arr_1t<DataPtrT<DObject> >*){ return false; }

	virtual DObject* DecorateWithM4(const mat4d&){ return this; }

	DECLARE_OBJECT_INFO(DObject,DObjectInfo);
};

class DLLIMPEXP_EWA_BASE DObjectBox : public DObject
{
public:
	typedef DObject basetype;
	DObjectBox(const String& name = "") :DObject(name){}

	DECLARE_OBJECT_INFO(DObjectBox, DObjectInfo);
};

template<typename T, typename B>
class FigGroupT : public B
{
public:
	FigGroupT(const String& name = "") :B(name){}

	ObjectGroupT<T> m_aItems;

	virtual bool DoGetChildren(arr_1t<DataPtrT<DObject> >* p)
	{
		if (p)
		{
			p->assign(m_aItems.begin(), m_aItems.end());
		}
		return true;
	}
};


template<typename T>
class NamedReferenceT : public DataPtrT<T>
{
public:
	String name;
};


EW_LEAVE
#endif
