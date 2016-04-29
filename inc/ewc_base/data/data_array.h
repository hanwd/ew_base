#ifndef __H_EW_UI_DATA_ARRAY__
#define __H_EW_UI_DATA_ARRAY__

#include "ewc_base/wnd/impl_wx/iwnd_dataview.h"

EW_ENTER


class DataNode;

class DataNodeArray : public wxDataViewItemArray
{
public:
	typedef DataNode* node_type;
	node_type& operator[](size_t i);
	void push_back(DataNode* d);
	void clear_and_destroy();
};


EW_LEAVE
#endif
