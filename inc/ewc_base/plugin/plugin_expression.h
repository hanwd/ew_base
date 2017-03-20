#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN_EXPRESSION__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN_EXPRESSION__

#include "ewc_base/plugin/plugin_common.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE PluginExpression : public PluginCommon
{
public:
	PluginExpression(WndManager& w);
	virtual bool OnAttach();

};

EW_LEAVE

#endif
