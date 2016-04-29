#ifndef __H_UI_EDITOR_PLUGIN_WEBVIEW__
#define __H_UI_EDITOR_PLUGIN_WEBVIEW__

#include "ewc_base/plugin/plugin_plugineditor.h"


EW_ENTER
class IWnd_webview;

class DLLIMPEXP_EWC_BASE PluginWebView : public PluginEditorEx
{
public:

	typedef PluginEditorEx basetype;

	PluginWebView(WndManager& w);


	virtual DataPtrT<MvcModel> CreateSampleModel();
	virtual bool OnAttach();

	void LoadURL(const String& s);

protected:

	static IWnd_webview* GetActiveWebView();

	bool OnWndEvent(IWndParam& cmd,int phase);
	bool OnCmdEvent(ICmdParam& cmd,int phase);
	String temp_url;
};


EW_LEAVE
#endif
