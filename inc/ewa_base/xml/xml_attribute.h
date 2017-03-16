#ifndef __H_EW_XML_ATTRIBUTE__
#define __H_EW_XML_ATTRIBUTE__

#include "ewa_base/xml/xml_base.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE XmlAttribute : public XmlBase
{
public:
	friend class XmlParser;
	friend class XmlNode;

	XmlAttribute() {}
	XmlAttribute(const String& n,const String& v=String());

	XmlAttribute* GetNext()
	{
		return m_pNextSibling.get();
	}
	const XmlAttribute* GetNext() const
	{
		return m_pNextSibling.get();
	}

	void Serialize(SerializerHelper sh);

protected:
	LitePtrT<XmlAttribute> m_pNextSibling;

};


EW_LEAVE
#endif
