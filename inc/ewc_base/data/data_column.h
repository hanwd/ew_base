#ifndef __H_EW_UI_DATA_COLUMN__
#define __H_EW_UI_DATA_COLUMN__

#include "ewc_base/evt/evt_array.h"
#include "ewc_base/data/data_node.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE DataNode;
class DLLIMPEXP_EWC_BASE DataColumn : public IValueColumn
{
public:

	DataColumn(const String& n,int t=COLUMNTYPE_STRING,int w=80);
    virtual void GetValue(wxVariant &variant,DataNode* node) const=0;
};


class DLLIMPEXP_EWC_BASE DataColumnName : public DataColumn
{
public:
	DataColumnName():DataColumn("name"){}
    virtual void GetValue(wxVariant &variant,DataNode* node) const
	{
		WxImpl<String>::set(variant,node->name);
	}
};



EW_LEAVE
#endif
