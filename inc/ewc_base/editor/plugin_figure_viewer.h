#ifndef __H_UI_EDITOR_PLUGIN_FIGURE_VIEWER__
#define __H_UI_EDITOR_PLUGIN_FIGURE_VIEWER__

#include "ewc_base/plugin/plugin_plugineditor.h"

EW_ENTER



class DLLIMPEXP_EWC_BASE PluginFigureViewer : public PluginEditorEx
{
public:
	typedef PluginEditorEx basetype;
	PluginFigureViewer(WndManager& w);
	DataPtrT<MvcModel> CreateSampleModel();
	virtual bool OnAttach();

	bool OnCfgEvent(int lv);

};


EW_LEAVE
#endif

