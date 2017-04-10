#ifndef __H_EW_UI_DATA_NODE__
#define __H_EW_UI_DATA_NODE__

#include "ewc_base/data/data_array.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE DataNode : public Object
{
public:

	enum
	{
		FLAG_TOUCHED	=1<<0,
		FLAG_IS_GROUP	=1<<1,
	};

	DataNode(DataNode* p = NULL, const String& n = "");
	~DataNode();

	virtual void UpdateGroup();

	DataNode* parent;
	String name;
	DataNodeArray subnodes;
	BitFlags flags;
	int depth;
};

EW_LEAVE
#endif

