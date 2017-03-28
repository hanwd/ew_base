#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN_RIBBON__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN_RIBBON__

#include "ewc_base/plugin/plugin_common.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE PluginRibbon : public PluginCommon
{
public:
	typedef PluginCommon basetype;

	PluginRibbon(WndManager& w);

	bool OnAttach();


};


EW_LEAVE

#endif
