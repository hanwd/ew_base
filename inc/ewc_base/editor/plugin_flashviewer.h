#ifndef __H_UI_EDITOR_PLUGIN_FLASHVIEWER__
#define __H_UI_EDITOR_PLUGIN_FLASHVIEWER__

#include "ewc_base/plugin/plugin_plugineditor.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE PluginFlashViewer : public PluginEditorEx
{
public:
	typedef PluginEditorEx basetype;
	PluginFlashViewer(WndManager& w);

	DataPtrT<MvcModel> CreateSampleModel();
};

EW_LEAVE
#endif
