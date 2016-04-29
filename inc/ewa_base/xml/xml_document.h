#ifndef __H_EW_XML_DOCUMENT__
#define __H_EW_XML_DOCUMENT__

#include "ewa_base/xml/xml_node.h"
#include "ewa_base/collection/arr_1t.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE XmlDocument : public XmlNode
{
public:
	friend class XmlParser;

	XmlDocument();

	// deep copy
	XmlDocument(const XmlDocument& o);
	const XmlDocument& operator=(const XmlDocument& o);

	~XmlDocument();

	bool LoadXml(const String& s);
	bool SaveXml(const String& s);
	bool LoadStr(const char* pstr_,size_t size_);
	bool LoadStr(const char* pstr_);

};


EW_LEAVE
#endif
