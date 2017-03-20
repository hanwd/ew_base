#ifndef __H_UI_EDITOR_PLUGIN_TEXT_EDITOR__
#define __H_UI_EDITOR_PLUGIN_TEXT_EDITOR__

#include "ewc_base/plugin/plugin_plugineditor.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE IWnd_stc;

class DLLIMPEXP_EWC_BASE PluginTextEditor : public PluginEditorEx
{
public:
	typedef PluginEditorEx basetype;
	PluginTextEditor(WndManager& w);
	DataPtrT<MvcModel> CreateSampleModel();
	virtual bool OnAttach();

	bool OnCfgEvent(int lv);

	IWnd_stc* GetStc();
};


EW_LEAVE
#endif
