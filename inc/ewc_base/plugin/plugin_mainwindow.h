#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN_MAINWINDOW__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN_MAINWINDOW__

#include "ewc_base/plugin/plugin_common.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE PluginMainWindow : public PluginCommon
{
public:
	typedef PluginCommon basetype;

	PluginMainWindow(WndManager& w);

	bool OnAttach();
};

EW_LEAVE
#endif
