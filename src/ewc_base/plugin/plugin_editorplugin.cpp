#include "ewc_base/plugin/plugin_editorplugin.h"

EW_ENTER

IEditorPlugin* IEditorPlugin::cast_editor_plugin()
{
	return this;
}

EW_LEAVE
