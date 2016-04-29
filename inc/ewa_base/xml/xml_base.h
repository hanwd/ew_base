#ifndef __H_EW_XML_BASE__
#define __H_EW_XML_BASE__


#include "ewa_base/basic/object.h"
#include "ewa_base/basic/pointer.h"
#include "ewa_base/memory/mempool.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE XmlAttribute;
class DLLIMPEXP_EWA_BASE XmlNode;
class DLLIMPEXP_EWA_BASE XmlDocument;
class DLLIMPEXP_EWA_BASE XmlParser;

class DLLIMPEXP_EWA_BASE XmlBase : public mp_obj
{
public:

	XmlBase(){}
	XmlBase(const String& name_,const String& value_="")
		:m_sName(name_),
		m_sValue(value_)
	{}

	const String& GetName() const
	{
		return m_sName;
	}
	const String& GetValue() const
	{
		return m_sValue;
	}

	void SetName(const String& s)
	{
		m_sName=s;
	}
	void SetValue(const String& s)
	{
		m_sValue=s;
	}

protected:
	String m_sName;
	String m_sValue;
};

EW_LEAVE
#endif
