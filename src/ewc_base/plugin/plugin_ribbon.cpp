#include "ewc_base/plugin/plugin_ribbon.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/evt/evt_base.h"
#include "ewc_base/evt/evt_cmdproc.h"
#include "ewc_base/evt/evt_command.h"
#include "ewc_base/evt/evt_group.h"

EW_ENTER

PluginRibbon::PluginRibbon(WndManager& w):PluginCommon(w,"Plugin.Ribbon")
{

}

bool PluginRibbon::OnAttach()
{

	EvtManager& ec(wm.evtmgr);


	return true;

}

IMPLEMENT_IPLUGIN(PluginRibbon)

EW_LEAVE

