#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN_SEARCH__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN_SEARCH__

#include "ewc_base/plugin/plugin_common.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE PluginSearch : public PluginCommon
{
public:

	PluginSearch(WndManager& w);

	virtual bool OnCmdEvent(ICmdParam& cmd,int phase);
	virtual bool OnWndEvent(IWndParam& cmd,int phase);

	virtual bool OnAttach();

};

EW_LEAVE

#endif
