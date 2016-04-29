#ifndef __H_EW_UI_DATA_NODE__
#define __H_EW_UI_DATA_NODE__

#include "ewc_base/data/data_array.h"

EW_ENTER


class DataNode : public Object
{
public:

	enum
	{
		FLAG_TOUCHED	=1<<0,
		FLAG_IS_GROUP	=1<<1,
	};

	DataNode(wxDataViewItem p=wxDataViewItem(),const String& n="");
	~DataNode();

	virtual unsigned int GetChildren(wxDataViewItemArray &children );

	String name;
	BitFlags flags;
	int depth;
	wxDataViewItem parent;

	DataNodeArray subnodes;

};

EW_LEAVE
#endif

