#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN_MENU__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN_MENU__

#include "ewc_base/plugin/plugin_common.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE PluginMenu : public PluginCommon
{
public:
	typedef PluginCommon basetype;

	PluginMenu(WndManager& w);

	bool OnAttach();


};


EW_LEAVE

#endif
