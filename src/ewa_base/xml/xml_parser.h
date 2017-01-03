#include "ewa_base/xml/xml_attribute.h"
#include "ewa_base/xml/xml_node.h"
#include "ewa_base/xml/xml_document.h"
#include "ewa_base/basic/bitflags.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/scanner_helper.h"

#include <fstream>

EW_ENTER



class XmlParser : public Object
{
public:

	typedef ScannerHelper<const char*> helper;
	typedef helper::type mychar;
	typedef helper::pointer mychar_ptr;

	XmlParser(XmlDocument& xmldoc_);

	bool LoadStr(const char* pstr_,size_t size_);
	bool LoadXml(const String& f,int t);
	bool SaveXml(const String& f);

	XmlNode* CreateNode(int t=XmlNode::XMLNODE_ELEMENT)
	{
		return new XmlNode(t);
	}

	XmlAttribute* CreateAttr()
	{
		return new XmlAttribute();
	}

	enum
	{
		FLAG_KEEP_COMMENT=1<<0,
		FLAG_KEEP_PI=1<<0,
	};

	BitFlags flags;

protected:

	template<size_t N>
	inline void skip_tag(mychar_ptr& pcur,const char (&tag)[N])
	{
		if(N<2)
		{
			return;
		}

		lookup_table<lkt2uppercase> lku;
		for(size_t i=0; i<N-1; i++)
		{
			if(lku(pcur[i])!=(mychar)tag[i])
			{
				kexpected(tag);
				return;
			}
		}
		pcur+=N-1;
	}

	XmlDocument& xmldoc;
	arr_1t<XmlNode*> nodes;
	StringBuffer<mychar> buffer;
	StringBuffer<mychar> tempbuf; //string_assign buffer
	StringBuffer<mychar> savebuf;

	inline void add_node(XmlNode* pnode)
	{
		pnode->m_pParent.reset(nodes.back());
		nodes.back()->AppendChild(pnode);
	}

	inline void put_node(XmlNode* pnode)
	{
		pnode->m_pParent.reset(nodes.back());
		nodes.back()->AppendChild(pnode);
		nodes.push_back(pnode);
	}

	inline void pop_node()
	{
		nodes.pop_back();
	}

	mychar_ptr pbeg;
	mychar_ptr pend;
	mychar_ptr pcur; //current position
	size_t size;


	void savenode(XmlNode* pnode,int lv=0);
	void tabindent(int lv)
	{
		for(int i=0; i<lv; i++)
		{
			savebuf<<"\t";
		}
	}
	void savestring(const String& v);

	void kerror(const String& msg);
	void kexpected(const String& msg);

	bool parse_document();

	//<tag attributes> content </tag>
	void parse_element_node();

	//A document node is a specialized kind of element node.
	//It has a type p but no attributes. Instead it has an optional URL u.
	//The intent of the URL is to specify a specialized data model for this node and its children.
	//A document node looks like this:
	//<!doctype p "u">c1 . . . cm for m>0
	void parse_doctype_node();

	//A processing instruction (PI) node is always a leaf node.
	//It only has an instruction i associated with it.
	//The instruction is a sequence of zero or more characters, without any restrictions,
	//except that the sequence may not start with the three characters `xml' (upper, lower or mixed case) followed by a space or newline.
	//It looks like this in the XML document:
	//<?i?>
	void parse_instruction_node();


	//<!-- This is a comment -->
	void parse_comment_node();

	void parse_subnodes();
	void parse_data();
	void parse_cdata();

	void parse_attributes();

	void parse_value();
	void parse_string1();
	void parse_string2();

	void string_assign(String& s0,mychar_ptr p1,mychar_ptr p2);

};

EW_LEAVE
