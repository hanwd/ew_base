#ifndef __H_UI_EDITOR_PLUGIN_EWSL_EDITOR__
#define __H_UI_EDITOR_PLUGIN_EWSL_EDITOR__

#include "ewc_base/plugin/plugin_plugineditor.h"

EW_ENTER

class IWnd_stc;

class PluginEwslEditor : public PluginEditorEx
{
public:
	typedef PluginEditorEx basetype;

	PluginEwslEditor(WndManager& w);

	DataPtrT<MvcModel> CreateSampleModel();
	virtual bool OnAttach();

	bool OnCfgEvent(int lv);

	bool OnCmdEvent(ICmdParam& cmd,int phase);

	IWnd_stc* GetStc();
};


EW_LEAVE
#endif

