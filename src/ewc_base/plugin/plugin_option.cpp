#include "ewc_base/plugin/plugin_option.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/evt/evt_option.h"
#include "ewc_base/wnd/wnd_updator.h"
#include "ewc_base/wnd/wnd_model.h"
#include "ewc_base/wnd/impl_wx/window.h"
#include "ewc_base/wnd/impl_wx/iwnd_treectrl.h"
#include "ewc_base/wnd/impl_wx/iwnd_bookbase.h"
#include "ewc_base/app/res_manager.h"

EW_ENTER


class WndModelOption : public WndModel
{
public:
	typedef WndModel basetype;

	WndManager& wm;

	IPageSelector page_selector;

	WndModelOption(WndManager& wm_):basetype("Model.Option"),wm(wm_)
	{
		flags.add(FLAG_NO_CLOSE);
	}
	
	bool WndExecute(IWndParam& cmd)
	{
		if(!basetype::WndExecute(cmd)) return false;
		if(cmd.action==IDefs::ACTION_APPLY)
		{
			wm.LoadConfig();
		}
		return true;
	}


	bool Create()
	{

		WndMaker km(this);

		km.flags(IDefs::IWND_EXPAND).sv(1);
		km.propotion(1).flags(IDefs::IWND_EXPAND).sv(2);

		km.win("dialog",WndProperty().label(_hT("options")).size(800,480));

			page_selector.init_tree(km.icur.hwnd, "Option.pages");
			page_selector.pevtmgr = &local_evtmgr;

			page_selector.ptype->SetMinSize(wxSize(200, 500));
			page_selector.pbook->SetMinSize(wxSize(280, 500));

			km.row(km.ld(0).flags(IDefs::IWND_EXPAND));
				km.col(km.ld(2));
					km.add(page_selector.ptype	,km.ld(1).size(280,500));
					km.add(page_selector.pbook	,km.ld(2).size(280,500));
				km.end();
				km.add("hline",km.ld(0).flags(IDefs::IWND_EXPAND));
				km.col(km.ld(0).flags(IDefs::IWND_EXPAND));
					km.add("space",km.propotion(1));
					km.add("button",km.ld(0).name(_kT("Btn.Ok")));
					km.add("button",km.ld(0).name(_kT("Btn.Apply")));
					km.add("button",km.ld(0).name(_kT("Btn.Reset")));
					km.add("button",km.ld(0).name(_kT("Btn.Cancel")));
				km.end();
			km.end();
		km.end();

		vald_grp->append(page_selector.pvald.get());

		return true;
	}

};



PluginOption::PluginOption(WndManager& w):PluginCommon(w,"Plugin.Option")
{

}

bool PluginOption::OnCmdEvent(ICmdParam& cmd,int phase)
{
	return true;
}


bool PluginOption::OnAttach()
{

	wm.evtmgr.append(new WndModelOption(wm));
	wm.evtmgr.append(new EvtCommandShowModel(_kT("Option"),"Model.Option"));

	wm.evtmgr["Option"].flags.del(FLAG_CHECK);

	return true;
}

IMPLEMENT_IPLUGIN(PluginOption)

EW_LEAVE
