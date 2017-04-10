#ifndef __H_EW_UI_DATA_ARRAY__
#define __H_EW_UI_DATA_ARRAY__

#include "ewc_base/wnd/impl_wx/iwnd_dataview.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE DataNode;

class DataNodeArray : public arr_1t<DataNode*>
{
public:
	void clear_and_destroy();
};


EW_LEAVE
#endif
