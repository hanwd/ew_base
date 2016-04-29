#include "ewc_base/app/app.h"
#include "ewc_base/plugin/plugin.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/mvc/mvc_view.h"

EW_ENTER


void Plugin::DoRegister(WndManager& wm,Plugin* p)
{
	wm.plugin.Register(p);
}

bool PluginEx::OnAttach(){return true;}
void PluginEx::OnDetach(){}

void PluginEx::OnEditorActivate(PluginEditor*,PluginEditor*){}



bool PluginEx::GetState(int f) const {return flags.get(f);}
void PluginEx::SetState(int f,bool b){return flags.set(f,b);}

void PluginEx::GetDependency(arr_1t<String>& arr)
{
	arr=m_aDependency;
}

EW_LEAVE
