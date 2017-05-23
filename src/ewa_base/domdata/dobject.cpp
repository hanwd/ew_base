#include "ewa_base/domdata/dobject.h"


EW_ENTER


DLight::DLight()
{
	light_index = -1;
}


DLineStyle::DLineStyle()
{
	ntype = LINE_SOLID;
	nsize = 1.0;

}

DLineStyle::DLineStyle(double w)
{
	ntype = LINE_SOLID;
	nsize = w;

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

bool DColor::set_by_string(const String& c)
{
	if (c.c_str()[0] != '#') return false;
	size_t n = c.size();

	uint32_t _r(0), _g(0), _b(0), _a = 255;

	if (n == 7)
	{
		if (sscanf(c.c_str() + 1, "%2X%2X%2X", &_r, &_g, &_b) != 3)
		{
			return false;
		}
	}
	else if (n == 9)
	{
		if (sscanf(c.c_str() + 1, "%2X%2X%2X%2X", &_r, &_g, &_b,&_a) != 4)
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	r = _r;
	g = _g;
	b = _b;
	a = _a;
	return true;
}

String DColor::cast_to_string() const
{
	return String::Format("#%02X%02X%02X%02X",r,g,b,a);
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
	color.set(124, 124, 124);
}

bool DFontStyle::operator==(const DFontStyle& rhs) const
{
	return nsize == rhs.nsize && sname == rhs.sname && color == rhs.color && flags == rhs.flags;
}

bool DFontStyle::operator!=(const DFontStyle& rhs) const
{
	return !operator==(rhs);
}

bool DExprItem::operator == (const DExprItem& rhs) const
{
	return name == rhs.name && value == rhs.value && desc == rhs.desc;
}

bool DExprItem::operator!=(const DExprItem& rhs) const
{
	return !(*this == rhs);
}


void DExprItem::SerializeVariant(Variant& v, int dir)
{

	if (dir == +1)
	{
		arr_xt<Variant>& value(v.ref<arr_xt<Variant> >());

		value.resize(3);
		value(0).reset(name);
		value(1).reset(value);
		value(2).reset(desc);

	}
	else if (dir == -1)
	{
		const arr_xt<Variant>& value(v.get<arr_xt<Variant> >());
		DExprItem item;
		if (value.size() < 2)
		{
			Exception::Exception("invalid value");
		}

		item.name = variant_cast<String>(value[0]);
		item.value = variant_cast<String>(value[1]);
		if (value.size()>2)
		{
			item.desc = variant_cast<String>(value[2]);
		}
		*this = item;

	}
	else
	{

	}
	

}

void DExprItem::Serialize(SerializerHelper sh)
{
	Serializer& ar(sh.ref(0));
	ar & name & value & desc;
}




SymbolItem::SymbolItem(const String& n) 
:name(n)
{

}


void SymbolItem::SerializeVariant(Variant& v, int dir)
{

	if (dir == +1)
	{
		if (prop.empty())
		{
			v.reset(name);
		}
		else
		{
			arr_xt<Variant>& value(v.ref<arr_xt<Variant> >());
			value.resize(prop.size()+1);
			value(0).reset(name);
			std::copy(prop.begin(), prop.end(), value.begin() + 1);
		}
	}
	else if (dir == -1)
	{
		if (v.get(name))
		{
			prop.clear();
		}
		else
		{
			const arr_xt<Variant>& value(v.get<arr_xt<Variant> >());
			SymbolItem item;
			if (value.size() < 1)
			{
				Exception::XError("invalid value");
			}

			item.name = variant_cast<String>(value[0]);
			item.prop.assign(value.begin() + 1, value.end());

*this = item;

		}

	}



}


bool SymbolItem::operator==(const SymbolItem& rhs)  const
{
	return name == rhs.name && prop == rhs.prop;
}

bool SymbolItem::operator!=(const SymbolItem& rhs) const
{
	return !(*this == rhs);
}

void SymbolItem::Serialize(SerializerHelper sh)
{
	Serializer& ar(sh.ref(0));
	ar & name & prop;
}


uint32_t hash_t<SymbolItem>::operator()(const SymbolItem& item)
{
	return hash_t<String>()(item.name);
}



DWhen::DWhen()
{
	t_begin = t_end = 0;
	t_step = -1;
}


DMatrixBox::DMatrixBox()
{
	m4.LoadIdentity();
	b3.load_min();
}

DMatrixBox::DMatrixBox(const mat4d& m) :m4(m)
{
	b3.load_min();
}


DObjectInfo::DObjectInfo(const String& s) :ObjectInfo(s)
{

}

DObject::DObject(const String& s)
	: m_sId(s)
{
}







void DAttribute::SetValue(const String& name, int value)
{
	values[name].type = "int";
	values[name].value.Printf("%d", value);
}

void DAttribute::SetValue(const String& name, float value)
{
	SetValue(name, double(value));
}

void DAttribute::SetValue(const String& name, double value)
{
	values[name].type = "double";
	values[name].value.Printf("%g", value);
}

void DAttribute::SetValue(const String& name, const String& value)
{
	values[name].type = "string";
	values[name].value = value;
}


String DAttribute::MakeName(const String& name, const String& sub)
{
	return sub + "." + name;

}


void DAttribute::SetValue(const String& name, const DColor& value)
{
	SetValue(name, value.cast_to_string());
}

void DAttribute::SetValue(const String& name, const DLineStyle& value)
{
	SetValue(MakeName(name, "line_nsize"), value.nsize);
	SetValue(MakeName(name, "line_ntype"), value.ntype);
	SetValue(MakeName(name, "line_color"), value.color);
}

void DAttribute::SetValue(const String& name, const DFontStyle& value)
{
	SetValue(MakeName(name, "font_nsize"), value.nsize);
	SetValue(MakeName(name, "font_sname"), value.sname);
	SetValue(MakeName(name, "font_color"), value.color);
	SetValue(MakeName(name, "font_flags"), *(int*)&value.flags);
}



void DAttribute::SetValue(const String& name, const vec3d& value)
{
	SetValue(DAttribute::MakeName(name, "vec3d_x"), value[0]);
	SetValue(DAttribute::MakeName(name, "vec3d_y"), value[1]);
	SetValue(DAttribute::MakeName(name, "vec3d_z"), value[2]);
}

void DAttribute::SetValue(const String& name, const box3d& value)
{
	SetValue(DAttribute::MakeName(name, "box_lo"), value.lo);
	SetValue(DAttribute::MakeName(name, "box_hi"), value.hi);
}



bool AttributeStringToValue(const String& t, double& v)
{
	return t.ToNumber(&v);
}


bool AttributeStringToValue(const String& t, int& v)
{
	return t.ToNumber(&v);
}

bool AttributeStringToValue(const String& t, String& v)
{
	v = t;
	return true;
}

template<typename T>
void AtrributeUpdator::DoUpdateType(const String& name, T& value)
{
	String name2(name);
	
	char* p =(char*) name2.c_str();

	for (int n = name2.length();n>0;n--)
	{
		
		if (p[n] == '.')
		{
			p[n] = 0;
		}

		if (p[n]) continue;

		for (auto it = aLinks.begin(); it != aLinks.end(); ++it)
		{
			auto & values(**it);
			auto vt = values.find(name2);
			if (vt != values.end())
			{
				if (AttributeStringToValue((*vt).second.value,value))
				{
					return;
				}
			}
		}
		

	}




}

void AtrributeUpdator::Update(const String& name, String& value)
{
	DoUpdateType(name, value);
}

void AtrributeUpdator::Update(const String& name, int& value)
{
	DoUpdateType(name, value);

}

void AtrributeUpdator::Update(const String& name, double& value)
{
	DoUpdateType(name, value);
}


void AtrributeUpdator::Update(const String& name, float& value)
{
	double dvalue(value);
	Update(name, dvalue);
	value = dvalue;
}


void AtrributeUpdator::Update(const String& name, DColor& value)
{
	String svalue;
	Update(name, svalue);
	if (!svalue.empty())
	{
		value.set_by_string(svalue);
	}
}

void AtrributeUpdator::Update(const String& name, DLineStyle& value)
{
	Update(DAttribute::MakeName(name, "line_nsize"), value.nsize);
	Update(DAttribute::MakeName(name, "line_ntype"), value.ntype);
	Update(DAttribute::MakeName(name, "line_color"), value.color);
}

void AtrributeUpdator::Update2(const String& name, DLineStyle& value)
{
	Update(name, value);
	nIndex++;

	int& color = *(int*)&value.color;
	color = 0x12348121*(nIndex + 1341) + 134141 + ((0x134121*nIndex)<<8);
	value.color.a = 255;
	
}

void AtrributeUpdator::Update(const String& name, DFontStyle& value)
{
	Update(DAttribute::MakeName(name, "font_nsize"), value.nsize);
	Update(DAttribute::MakeName(name, "font_sname"), value.sname);
	Update(DAttribute::MakeName(name, "font_color"), value.color);
	Update(DAttribute::MakeName(name, "font_flags"), *(int*)&value.flags);
}

void AtrributeUpdator::Update(const String& name, vec3d& value)
{
	Update(DAttribute::MakeName(name, "vec3d_x"), value[0]);
	Update(DAttribute::MakeName(name, "vec3d_y"), value[1]);
	Update(DAttribute::MakeName(name, "vec3d_z"), value[2]);
}

void AtrributeUpdator::Update(const String& name, box3d& value)
{
	Update(DAttribute::MakeName(name, "box_lo"), value.lo);
	Update(DAttribute::MakeName(name, "box_hi"), value.hi);
}

void AtrributeUpdator::ResetIndex()
{
	nIndex = 0;
}

void AtrributeUpdator::SetManager(DAttributeManager* pmgr)
{
	m_pAttributeManager.reset(pmgr);
}

void AtrributeUpdator::DoUpdateInternal(const String& name)
{
	auto it = m_pAttributeManager->m_mapAttributes.find(name);
	if (it == m_pAttributeManager->m_mapAttributes.end()) return;
	DoAppend((*it).second.get());
}


void AtrributeUpdator::DoAppend(DAttribute* p)
{
	if (!p) return;
	aLinks.push_back(&p->values);
}

void AtrributeUpdator::SetObject(DObject* pobj)
{
	aLinks.clear();

	if (!m_pAttributeManager)
	{
		return;
	}

	if (pobj)
	{
		DoAppend(pobj->m_pAttribute.get());

		if (!pobj->m_sId.empty())
		{
			DoUpdateInternal(pobj->m_sId);
		}

		DoUpdateInternal(pobj->GetObjectName());
	}

	DoUpdateInternal("");

}


DataPtrT<DAttribute> DAttributeManager::MakeWhiteBackgroundStyle()
{

	DataPtrT<DAttribute> p(new DAttribute);

	DFontStyle font_text;
	font_text.color.set(0, 0, 0);
	p->SetValue("text", font_text);

	DFontStyle font_label;
	font_label.color.set(0, 0, 0);
	p->SetValue("text.label", font_text);

	DLineStyle line_main;
	line_main.color.set(0, 0, 0);
	line_main.set(DLineStyle::LINE_DASH3);
	p->SetValue("line.axis.main", line_main);

	DLineStyle line_user;
	line_main.color.set(0, 0, 0);
	line_main.set(DLineStyle::LINE_DASH4);
	p->SetValue("line.axis.user", line_user);

	DLineStyle line_tick;
	line_tick.color.set(0, 0, 0);
	p->SetValue("line.axis.tick", line_tick);

	DLineStyle line_bbox;
	line_bbox.color.set(0, 0, 0);
	p->SetValue("line.bbox", line_bbox);


	DLineStyle line_data;
	line_data.color.set(255, 0, 0);
	p->SetValue("line.data", line_data);


	DLineStyle line_dir;
	p->SetValue(DAttribute::MakeName("line.axis_dir", "line_nsize"), line_dir.nsize);
	p->SetValue(DAttribute::MakeName("line.axis_dir", "line_ntype"), line_dir.ntype);

	p->SetValue("line_color.line.axis_dir.x", DColor(255, 0, 0));
	p->SetValue("line_color.line.axis_dir.y", DColor(0, 255, 0));
	p->SetValue("line_color.line.axis_dir.z", DColor(0, 0, 255));

	p->SetValue("color.background", DColor(220, 220, 220));

	return p;

}

DataPtrT<DAttribute> DAttributeManager::MakeBlackBackgroundStyle()
{

	DataPtrT<DAttribute> p(new DAttribute);

	DFontStyle font_text;
	font_text.color.set(200, 200, 200);
	p->SetValue("text", font_text);

	DFontStyle font_label;
	font_label.color.set(200, 200, 200);
	p->SetValue("text.label", font_text);

	DLineStyle line_main;
	line_main.color.set(200, 200, 200);
	line_main.set(DLineStyle::LINE_DASH3);
	p->SetValue("line.axis.main", line_main);

	DLineStyle line_user;
	line_main.color.set(200, 200, 200);
	line_main.set(DLineStyle::LINE_DASH4);
	p->SetValue("line.axis.user", line_user);

	DLineStyle line_tick;
	line_tick.color.set(200, 200, 200);
	p->SetValue("line.axis.tick", line_tick);

	DLineStyle line_bbox;
	line_bbox.color.set(200, 200, 200);
	p->SetValue("line.bbox", line_bbox);


	DLineStyle line_data;
	line_data.color.set(255, 0, 0);
	p->SetValue("line.data", line_data);


	DLineStyle line_dir;
	p->SetValue(DAttribute::MakeName("line.axis_dir", "line_nsize"), line_dir.nsize);
	p->SetValue(DAttribute::MakeName("line.axis_dir", "line_ntype"), line_dir.ntype);

	p->SetValue("line_color.line.axis_dir.x", DColor(255, 0, 0));
	p->SetValue("line_color.line.axis_dir.y", DColor(0, 255, 0));
	p->SetValue("line_color.line.axis_dir.z", DColor(0, 0, 255));

	p->SetValue("color.background", DColor(0, 0, 0));

	return p;

}

DAttributeManager::DAttributeManager()
{
	static int style = 0;
	m_mapAttributes[""] = style++%2 ? MakeBlackBackgroundStyle() : MakeWhiteBackgroundStyle();
}


IMPLEMENT_OBJECT_INFO(DObject, DObjectInfo);
IMPLEMENT_OBJECT_INFO(DAttributeManager, DObjectInfo);

IMPLEMENT_OBJECT_INFO(DObjectBox, DObjectInfo);

EW_LEAVE
