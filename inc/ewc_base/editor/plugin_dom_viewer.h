#ifndef __H_UI_EDITOR_PLUGIN_DOM_VIEWER__
#define __H_UI_EDITOR_PLUGIN_DOM_VIEWER__

#include "ewc_base/plugin/plugin_plugineditor.h"

EW_ENTER



class DLLIMPEXP_EWC_BASE PluginDomViewer : public PluginEditorEx
{
public:
	typedef PluginEditorEx basetype;
	PluginDomViewer(WndManager& w);
	DataPtrT<MvcModel> CreateSampleModel();
	virtual bool OnAttach();

	bool OnCfgEvent(int lv);

};


EW_LEAVE
#endif

