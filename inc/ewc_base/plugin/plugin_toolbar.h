#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN_TOOLBAR__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN_TOOLBAR__

#include "ewc_base/plugin/plugin_common.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE PluginToolBar : public PluginCommon
{
public:
	typedef PluginCommon basetype;

	PluginToolBar(WndManager& w);

	bool OnAttach();


};


EW_LEAVE

#endif
