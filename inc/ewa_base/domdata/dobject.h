
#ifndef __H_EW_DOMDATA_DOBJECT__
#define __H_EW_DOMDATA_DOBJECT__


#include "ewa_base/basic/object.h"
#include "ewa_base/math/math_def.h"
#include "ewa_base/scripting/variant.h"
#include "ewa_base/scripting/callable_data.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE DState;
class DLLIMPEXP_EWA_BASE DContext;
class DLLIMPEXP_EWA_BASE DChildrenState;
class DLLIMPEXP_EWA_BASE TableSerializer;


class DLLIMPEXP_EWA_BASE DColor
{
public:

	DColor(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255u);
	DColor();

	operator const uint8_t*() const;

	void set(uint8_t r_, uint8_t g_, uint8_t b_);
	void set(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_);

	bool set_by_string(const String& c);
	String cast_to_string() const;

	uint8_t r, g, b, a;
};


class DLLIMPEXP_EWA_BASE DFontStyle
{
public:

	DFontStyle();
	DFontStyle(float size);

	String sname;
	float nsize;
	BitFlags flags;
	DColor color;

	bool operator==(const DFontStyle& rhs) const;
	bool operator!=(const DFontStyle& rhs) const;

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
	vec4f v4ambient;
	vec4f v4diffuse;
	vec4f v4specular;
	vec4f v4position;
	int light_index;
};


class DLLIMPEXP_EWA_BASE DExprItem
{
public:
	String name,value,desc;

	DExprItem(const String& n,const String& v,const String& d=""):name(n),value(v),desc(d){}
	DExprItem(){}

	void SerializeVariant(Variant& v,int dir);
	void Serialize(SerializerHelper sh);

	bool operator==(const DExprItem& rhs) const;
	bool operator!=(const DExprItem& rhs) const;
};



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



/* 2017/06/28 Dai.Weifeng
 * DWhen 设置计算体系的起止时间步和间隔时间步
 * 默认构造参数：t_begin = 0, t_end = 0, t_step = 1
 * 文档对象设定时，事实上由各个对象自身给出DWhen参数的“用户建议设置”
 * t_begin == -1, t_end == -1, t_step == -1均认为是“默认设置”
 * 默认设置如何转换为前后处理对象和引擎级别对象的真实DWhen参数，则由记录器的前后处理对象的函数autowhen来修正
 * bool test(int) const函数用于测试某时间步是否处于需要处理，true为需要处理，false为不需要
 * 由于计算提前终止导致真实的截至时间步和用户设定的截至时间步不同时由引擎级别对象或前后处理对象进行处理
 */
class DLLIMPEXP_EWA_BASE DWhen
{
public:

	int t_begin,t_end,t_step;
	bool test(int t_cur) const;

	DWhen();
};


class DLLIMPEXP_EWA_BASE DMatrixBox
{
public:

	DMatrixBox();
	DMatrixBox(const mat4d& m);


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

	bool operator==(const SymbolItem& rhs) const;
	bool operator!=(const SymbolItem& rhs) const;
};


template<>
class hash_t<SymbolItem>
{
public:
	uint32_t operator()(const SymbolItem& item);
};

DEFINE_OBJECT_NAME(SymbolItem,"SymbolItem")



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

class DLLIMPEXP_EWA_BASE DAttribute : public ObjectData
{
public:

	class Item
	{
	public:
		String type;
		String value;
	};

	indexer_map<String, Item> values;

	static String MakeName(const String& name, const String& sub);

	void SetValue(const String& name, int value);
	void SetValue(const String& name, float value);
	void SetValue(const String& name, double value);
	void SetValue(const String& name, const String& value);
	void SetValue(const String& name, const DColor& value);
	void SetValue(const String& name, const DLineStyle& value);
	void SetValue(const String& name, const DFontStyle& value);

	void SetValue(const String& name, const vec3d& value);
	void SetValue(const String& name, const box3d& value);

};


class DLLIMPEXP_EWA_BASE DObjectInfo : public ObjectInfo
{
public:
	DObjectInfo(const String& s = "");
};





class DLLIMPEXP_EWA_BASE DObject : public CallableData
{
public:
	DObject(const String& s = "");

	enum
	{
		FLAG_MIN		=1<<0,
		FLAG_UPDATED	=FLAG_MIN<<0,
		FLAG_ERROR		=FLAG_MIN<<1,
		FLAG_MAX		=FLAG_MIN<<2,
	};

	String m_sId;
	DataPtrT<DAttribute> m_pAttribute;
	BitFlags flags;

	virtual bool DoUpdateValue(DState&){return true;}
	virtual bool DoCheckParam(DState&){return true;}

	virtual bool DoTransferData(TableSerializer& ar);

	virtual void DoRender(DContext&){}
	virtual bool DoGetChildren(DChildrenState&){ return false; }

	virtual DObject* DecorateWithM4(const mat4d&){ return this; }

	DECLARE_OBJECT_INFO(DObject,DObjectInfo);
};


class DLLIMPEXP_EWA_BASE AtrributeUpdator;

class DLLIMPEXP_EWA_BASE DAttributeManager : public DObject
{
public:

	DAttributeManager();
	indexer_map<String, DataPtrT<DAttribute> > m_mapAttributes;


	static DataPtrT<DAttribute> MakeWhiteBackgroundStyle();
	static DataPtrT<DAttribute> MakeBlackBackgroundStyle();

	DECLARE_OBJECT_INFO(DAttributeManager, DObjectInfo);
};




class DLLIMPEXP_EWA_BASE AtrributeUpdator
{
public:

	void SetManager(DAttributeManager* pmgr);
	void SetObject(DObject* pobj);


	void Update(const String& name, String& value);
	void Update(const String& name, int& value);
	void Update(const String& name, double& value);
	void Update(const String& name, float& value);
	void Update(const String& name, DColor& value);
	void Update(const String& name, DLineStyle& value);
	void Update2(const String& name, DLineStyle& value);

	void Update(const String& name, DFontStyle& value);
	void Update(const String& name, vec3d& value);
	void Update(const String& name, box3d& value);

	void ResetIndex();

protected:

	int nIndex;

	void DoAppend(DAttribute* p);
	DataPtrT<DAttributeManager> m_pAttributeManager;

	void DoUpdateInternal(const String& name);
	arr_1t<indexer_map<String, DAttribute::Item>*> aLinks;


	template<typename T>
	void DoUpdateType(const String& name, T& value);


};


class DLLIMPEXP_EWA_BASE DChildrenState
{
public:

	DChildrenState()
	{
		m_parray = &m_internal;
	}

	typedef arr_1t<DataPtrT<DObject> > grp_type;
	
	void assign(grp_type::iterator t1, grp_type::iterator t2)
	{
		m_parray->assign(t1, t2);
	}

	void append(DObject* p)
	{
		m_parray->append(p);
	}

	template<typename T>
	void append(const DataPtrT<T>& p)
	{
		m_parray->append((T*)p.get());
	}

	size_t size() const
	{
		return m_parray->size();
	}

	DataPtrT<DObject>& operator[](size_t i)
	{
		return (*m_parray)[i];
	}

	void clear()
	{
		m_internal.clear();
		m_parray = &m_internal;
	}

	void set_array(grp_type& p)
	{
		m_parray = &p;
	}

	void set_item(DObject* p)
	{
		clear();
		m_internal.append(p);
	}

	DataPtrT<DObject> pdoc;

private:
	grp_type* m_parray;
	grp_type m_internal;

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

	virtual bool DoGetChildren(DChildrenState& cs)
	{
		cs.set_array((DChildrenState::grp_type&)m_aItems.proxy());
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
