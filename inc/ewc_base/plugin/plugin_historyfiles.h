#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN_HISTORYFILE__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN_HISTORYFILE__

#include "ewc_base/plugin/plugin_common.h"
#include "ewc_base/plugin/plugin_manager.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE PluginHistoryFiles : public PluginCommon
{
public:
	typedef PluginCommon basetype;

	PluginHistoryFiles(WndManager& w);

	bool OnAttach();
	void OnDetach();

	bool OnCmdEvent(ICmdParam&,int);

	void remove(const String& s);
	void insert(const String& s);

	bool OnCfgEvent(int lv);

	arr_1t<String> aHistoryFiles;
	int32_t nHistorySize;
};


EW_LEAVE

#endif
