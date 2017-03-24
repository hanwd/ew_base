#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/mvc/mvc_book.h"
#include "ewc_base/wnd/impl_wx/window.h"

EW_ENTER

void PluginManager::Register(Plugin* p)
{
	if(!p) return;
	String s=p->m_sId;
	if(plugin_map[s])
	{
		return;
	}

	plugin_map[s].reset(p);

	if(p->cast_common())
	{
		plugin_common.append(p->cast_common());
	}

	if(p->cast_editor())
	{
		plugin_editor.append(p->cast_editor());
	}

	if(p->cast_editor_plugin())
	{
		plugin_plugin.append(p->cast_editor_plugin());
	}
	
}


bool PluginManager::AttachPlugin(Plugin* p)
{
	if(!p)
	{
		return false;
	}

	// 判断是否已经加载
	if(p->GetState(Plugin::FLAG_ATTACHED))
	{
		return true;
	}

	// 判断依赖是否循环
	if(p->GetState(Plugin::FLAG_PENDING)) // recursive dependency
	{
		System::LogDebug("recursive plugin dependency");
		return false;
	}

	// 设置正在处理的标识
	p->SetState(Plugin::FLAG_PENDING,true);

	// 取得依赖
	arr_1t<String> d;
	p->GetDependency(d);

	// 加载依赖的plugin
	bool flag=true;
	for(size_t i=0;i<d.size();i++)
	{
		flag=flag&&AttachPlugin(plugin_map[d[i]].get());
	}

	flag=flag&&p->OnAttach();

	p->SetState(Plugin::FLAG_ATTACHED,flag);
	p->SetState(Plugin::FLAG_PENDING,false);

	return flag;
}

PluginManager::PluginManager()
{

}

EW_LEAVE
