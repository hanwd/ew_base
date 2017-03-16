#ifndef __H_EW_XML_NODE__
#define __H_EW_XML_NODE__

#include "ewa_base/xml/xml_attribute.h"
#include "ewa_base/collection/intrusive_list.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE XmlDocument;

class DLLIMPEXP_EWA_BASE XmlNode : public XmlBase
{
public:
	friend class XmlParser;
	friend class XmlDocument;

	enum
	{
		XMLNODE_ELEMENT,
		XMLNODE_DATA,
		XMLNODE_CDATA,
		XMLNODE_ROOT,
		XMLNODE_COMMENT,	// dropped
		XMLNODE_PI,			// dropped
		XMLNODE_DOCTYPE,	// dropped
	};


	XmlNode(int t=XMLNODE_ELEMENT):m_nNodeType(t) {}

	XmlNode(const XmlNode& o);
	XmlNode(const String& tag,const String& val="");

	~XmlNode();

	int GetType() const
	{
		return m_nNodeType;
	}
	void SetType(int t)
	{
		m_nNodeType=t;
	}

	XmlNode* GetFirstChild()
	{
		return listNodes.head();
	}
	XmlNode* GetNext()
	{
		return m_pNextSibling.get();
	}
	const XmlNode* GetFirstChild() const
	{
		return listNodes.head();
	}
	const XmlNode* GetNext() const
	{
		return m_pNextSibling.get();
	}

	XmlNode* GetParent()
	{
		return m_pParent.get();
	}
	const XmlNode* GetParent() const
	{
		return m_pParent.get();
	}
	void SetParent(XmlNode* p)
	{
		m_pParent.reset(p);
	}

	XmlAttribute* GetFirstAttribute()
	{
		return listAttrs.head();
	}
	const XmlAttribute* GetFirstAttribute() const
	{
		return listAttrs.head();
	}

	void InsertChild(XmlNode* p)
	{
		p->SetParent(this);
		listNodes.insert(p);
	}
	void AppendChild(XmlNode* p)
	{
		p->SetParent(this);
		listNodes.append(p);
	}
	bool RemoveChild(XmlNode* p)
	{
		return listNodes.remove(p);
	}
	bool DetachChild(XmlNode* p)
	{
		return listNodes.detach(p);
	}
	void DeleteChildren()
	{
		listNodes.clear();
	}

	void InsertAttribute(XmlAttribute* attr)
	{
		listAttrs.insert(attr);
	}
	void AppendAttribute(XmlAttribute* attr)
	{
		listAttrs.append(attr);
	}
	bool RemoveAttribute(XmlAttribute* attr)
	{
		return listAttrs.remove(attr);
	}
	bool DetachAttribute(XmlAttribute* attr)
	{
		return listAttrs.detach(attr);
	}
	void DeleteAttributes()
	{
		listAttrs.clear();
	}

	void Clear()
	{
		DeleteChildren();
		DeleteAttributes();
	}

	void Serialize(SerializerHelper sh);

	void swap(XmlNode& o);

protected:

	template<typename T>
	class policy_node_next
	{
	public:
		static T* GetNext(T* pnode)
		{
			return pnode->m_pNextSibling.get();
		}

		static void SetNext(T* pnode, T* next)
		{
			pnode->m_pNextSibling.reset(next);
		}

		static void DeleteNode(T* pnode)
		{
			delete pnode;
		}
	};

	LitePtrT<XmlNode> m_pNextSibling;
	LitePtrT<XmlNode> m_pParent;

	intrusive_list<XmlNode,policy_node_next<XmlNode> > listNodes;
	intrusive_list<XmlAttribute,policy_node_next<XmlAttribute> > listAttrs;

	int32_t m_nNodeType;

	void xml_ensure_children_parent();

	static XmlNode* xml_copy_recursive(const XmlNode* node);

};


EW_LEAVE
#endif
