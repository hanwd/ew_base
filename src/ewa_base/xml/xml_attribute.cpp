#include "ewa_base/xml/xml_attribute.h"
#include "ewa_base/serialization/serializer.h"

EW_ENTER

XmlAttribute::XmlAttribute(const String& n,const String& v)
	:XmlBase(n,v)
{

}

void XmlAttribute::Serialize(Serializer& ar)
{
	ar & m_sName & m_sValue;
}

EW_LEAVE
