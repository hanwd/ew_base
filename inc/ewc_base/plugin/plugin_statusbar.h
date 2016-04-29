#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN_STATUSBAR__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN_STATUSBAR__


#include "ewc_base/plugin/plugin_common.h"


EW_ENTER

	
class DLLIMPEXP_EWC_BASE PluginStatusBar : public PluginCommon2
{
public:
	typedef PluginCommon2 basetype;

	PluginStatusBar(WndManager& w);

	virtual bool OnAttach();

};


EW_LEAVE
#endif
