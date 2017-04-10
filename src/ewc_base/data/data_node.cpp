#include "ewc_base/data/data_node.h"

EW_ENTER


DataNode::DataNode(DataNode* p,const String& n):parent(p),name(n)
{
	depth=0;
}

DataNode::~DataNode()
{
	subnodes.clear_and_destroy();
}


void DataNode::UpdateGroup()
{
	EW_ASSERT(flags.get(DataNode::FLAG_IS_GROUP));
	flags.add(DataNode::FLAG_TOUCHED);
}

EW_LEAVE
