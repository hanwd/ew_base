#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN_OPTION__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN_OPTION__


#include "ewc_base/plugin/plugin_common.h"


EW_ENTER

class DLLIMPEXP_EWC_BASE PluginOption : public PluginCommon
{
public:

	PluginOption(WndManager& w);

	virtual bool OnCmdEvent(ICmdParam& cmd,int phase);
	virtual bool OnAttach();


};

EW_LEAVE

#endif
