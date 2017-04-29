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

void SymbolItem::Serialize(SerializerHelper sh)
{
	Serializer& ar(sh.ref(0));
	ar & name & prop;
}


uint32_t hash_t<SymbolItem>::operator()(const SymbolItem& item)
{
	return hash_t<String>()(item.name);
}


IMPLEMENT_OBJECT_INFO(DObject, DObjectInfo);
IMPLEMENT_OBJECT_INFO(DObjectBox, DObjectInfo);

EW_LEAVE
