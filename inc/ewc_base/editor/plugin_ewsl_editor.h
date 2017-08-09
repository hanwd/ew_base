#ifndef __H_UI_EDITOR_PLUGIN_EWSL_EDITOR__
#define __H_UI_EDITOR_PLUGIN_EWSL_EDITOR__

#include "ewc_base/plugin/plugin_plugineditor.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE IWnd_stc;

class DLLIMPEXP_EWC_BASE PluginEwslEditor : public PluginEditorEx
{
public:
	typedef PluginEditorEx basetype;

	PluginEwslEditor(WndManager& w);

	DataPtrT<MvcModel> CreateSampleModel();
	virtual bool OnAttach();

	bool OnCfgEvent(int lv);

	bool OnCmdEvent(ICmdParam& cmd,int phase);

	bool GetExts(arr_1t<String>& exts);

	IWnd_stc* GetStc();
};


EW_LEAVE
#endif

