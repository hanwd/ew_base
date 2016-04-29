#include "ewc_base/data/data_node.h"

EW_ENTER


DataNode::DataNode(wxDataViewItem p,const String& n):name(n),parent(p)
{
	depth=0;
}

DataNode::~DataNode()
{
	subnodes.clear_and_destroy();
}

unsigned int DataNode::GetChildren(wxDataViewItemArray &children )
{
	EW_ASSERT(flags.get(DataNode::FLAG_IS_GROUP));

	flags.add(DataNode::FLAG_TOUCHED);
	children=subnodes;
	return children.size();
}

EW_LEAVE
