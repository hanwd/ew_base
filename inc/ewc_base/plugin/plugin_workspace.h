#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN_WORKSPACE__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN_WORKSPACE__


#include "ewc_base/plugin/plugin_common.h"


EW_ENTER

class DLLIMPEXP_EWC_BASE PluginWorkspace : public PluginCommon2
{
public:

	typedef PluginCommon2 basetype;
	PluginWorkspace(WndManager& w);

	virtual bool OnAttach();

	bool OnCmdEvent(ICmdParam& cmd,int phase);

};

EW_LEAVE

#endif
