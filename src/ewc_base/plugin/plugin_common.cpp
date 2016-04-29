#include "ewc_base/plugin/plugin_common.h"

EW_ENTER

PluginCommon* PluginCommon::cast_common()
{
	return this;
}

void PluginCommon2::GetDependency(arr_1t<String>& arr)
{
	PluginCommon::GetDependency(arr);
	arr.push_back("Plugin.MainWindow");
}

EW_LEAVE
