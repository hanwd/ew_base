#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN_MANAGER__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN_MANAGER__

#include "ewc_base/plugin/plugin_common.h"
#include "ewc_base/plugin/plugin_editorplugin.h"
#include "ewc_base/plugin/plugin_plugineditor.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE WndManager;

class DLLIMPEXP_EWC_BASE PluginManager
{
public:

	friend class WndManager;

	void Register(Plugin* p);

	static PluginManager& current();

	indexer_map<String,DataPtrT<Plugin> > plugin_map;
	ObjectGroupT<PluginCommon> plugin_common;
	ObjectGroupT<PluginEditor> plugin_editor;
	ObjectGroupT<IEditorPlugin> plugin_plugin;

	PluginManager();

protected:

	bool AttachPlugin(Plugin* p);



};


EW_LEAVE
#endif
