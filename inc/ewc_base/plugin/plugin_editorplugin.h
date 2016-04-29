#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN_EDITORPLUGIN__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN_EDITORPLUGIN__

#include "ewc_base/plugin/plugin.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE IEditorPlugin : public PluginEx
{
public:
	typedef PluginEx basetype;
	IEditorPlugin(WndManager& w,const String& s):basetype(w,s){}

	virtual IEditorPlugin* cast_editor_plugin();
};


EW_LEAVE
#endif
