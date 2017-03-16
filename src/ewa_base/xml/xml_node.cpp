#include "ewa_base/xml/xml_node.h"
#include "ewa_base/logging/logger.h"
#include "ewa_base/serialization/serializer.h"

#include "xml_parser.h"

EW_ENTER



XmlNode::XmlNode(const XmlNode& o)
	:XmlBase(o)
	,m_nNodeType(o.m_nNodeType)
{

}

XmlNode::XmlNode(const String& tag,const String& val)
	:XmlBase(tag,val)
	,m_nNodeType(XMLNODE_ELEMENT)
{

}

XmlNode::~XmlNode()
{
	DeleteChildren();
	DeleteAttributes();
}

void XmlNode::xml_ensure_children_parent()
{
	for(XmlNode* pnode=GetFirstChild(); pnode!=NULL; pnode=pnode->GetNext())
	{
		pnode->SetParent(this);
	}
}

void XmlNode::swap(XmlNode& o)
{
	m_sName.swap(o.m_sName);
	m_sValue.swap(o.m_sValue);
	std::swap(m_nNodeType,o.m_nNodeType);
	listNodes.swap(o.listNodes);
	listAttrs.swap(o.listAttrs);
	xml_ensure_children_parent();
	o.xml_ensure_children_parent();
}

void XmlNode::Serialize(SerializerHelper sh)
{
	Serializer& ar(sh.ref(0));

	ar & m_sName & m_sValue & m_nNodeType;

	int32_t nChilds=listNodes.size();
	ar & nChilds;
	if(ar.is_reader())
	{
		DeleteChildren();
		for(int32_t i=0; i<nChilds; i++)
		{
			AutoPtrT<XmlNode> pnode(new XmlNode);
			AppendChild(pnode.release());
		}

	}
	else
	{
		for(XmlNode* pnode=GetFirstChild(); pnode!=NULL; pnode=pnode->GetNext())
		{
			pnode->Serialize(ar);
		}
	}

	int32_t nAttrs=listAttrs.size();
	ar & nAttrs;
	if(ar.is_reader())
	{
		DeleteAttributes();
		for(int32_t i=0; i<nAttrs; i++)
		{
			AutoPtrT<XmlAttribute> pattr(new XmlAttribute);
			pattr->Serialize(ar);
			AppendAttribute(pattr.release());
		}
	}
	else
	{
		for(XmlAttribute* pattr=GetFirstAttribute(); pattr!=NULL; pattr=pattr->GetNext())
		{
			pattr->Serialize(ar);
		}
	}


}



XmlNode* XmlNode::xml_copy_recursive(const XmlNode* node)
{
	if(!node) return NULL;

	AutoPtrT<XmlNode> nnew(new XmlNode(*node));
	for(const XmlNode* p=node->GetFirstChild();p;p=p->GetNext())
	{
		nnew->listNodes.append(xml_copy_recursive(p));
	}
	for(const XmlAttribute* p=node->GetFirstAttribute();p;p=p->GetNext())
	{
		nnew->listAttrs.append(new XmlAttribute(*p));
	}
	nnew->xml_ensure_children_parent();
	return nnew.release();
 
}

EW_LEAVE
