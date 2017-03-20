#include "ewc_base/plugin/plugin_search.h"
#include "ewc_base/evt/evt_cmdproc.h"

#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/wnd/wnd_updator.h"
#include "ewc_base/wnd/wnd_model.h"
#include "ewc_base/wnd/impl_wx/window.h"

EW_ENTER

class EvtCmdSearch : public EvtCommandCmdProc
{
public:

	EvtCmdSearch(WndManager& w,const String& s,int d)
		:EvtCommandCmdProc(w,s,d)
	{
		
	}


	bool DoCmdExecute(ICmdParam& cmd)
	{
		wm.evtmgr["search.group"].WndExecuteEx(IDefs::ACTION_TRANSFER2MODEL);
		return EvtCommandCmdProc::DoCmdExecute(cmd);
	}
};

class EvtCmdShowSearch : public EvtCommandCmdProc
{
public:

	EvtCmdShowSearch(WndManager& w,const String& s,int d):EvtCommandCmdProc(w,s,d){}
	bool DoCmdExecute(ICmdParam&)
	{
		wm.evtmgr["search.replace"].WndExecuteEx(IDefs::ACTION_VALUE_CHANGED,id==CmdProc::CP_REPLACE?1:0);
		wm.evtmgr["Model.Search"].StdExecuteEx(1);
		return true;
	}
};


PluginSearch::PluginSearch(WndManager& w):PluginCommon(w,"Plugin.Search")
{

}

bool PluginSearch::OnCmdEvent(ICmdParam& cmd,int phase)
{
	return true;
}

bool PluginSearch::OnWndEvent(IWndParam& cmd,int phase)
{

	if(cmd.evtptr && cmd.evtptr->m_sId=="search.text_new")
	{
		return true;
	}

	if(phase>0 && cmd.action==IDefs::ACTION_VALUE_CHANGED && cmd.evtptr && cmd.evtptr->m_sId=="search.regexp")
	{
		WndManager& wm(WndManager::current());	
		wm.evtmgr["search.case"].flags.set(EvtBase::FLAG_DISABLE,cmd.param1!=0);
		wm.evtmgr["search.word"].flags.set(EvtBase::FLAG_DISABLE,cmd.param1!=0);
		wm.evtmgr["search.case"].UpdateCtrl();
		wm.evtmgr["search.word"].UpdateCtrl();
		return true;
	}

	if(phase>0 && cmd.action==IDefs::ACTION_VALUE_CHANGED && cmd.evtptr && cmd.evtptr->m_sId=="search.replace")
	{
		WndManager& wm(WndManager::current());	
		wm.evtmgr["Btn.Replace"].flags.set(EvtBase::FLAG_HIDE_UI,cmd.param1==0);
		wm.evtmgr["Btn.ReplaceAll"].flags.set(EvtBase::FLAG_HIDE_UI,cmd.param1==0);
		wm.evtmgr["search.text_new"].flags.set(EvtBase::FLAG_HIDE_UI,cmd.param1==0);
		return true;
	}


	if(phase>0 && cmd.action==IDefs::ACTION_VALUE_CHANGED && cmd.evtptr && cmd.evtptr->m_sId=="search.text_old")
	{
		WndManager::current().evtmgr["Btn.Find"].CmdExecuteEx(1);
	}

	return true;
}

bool PluginSearch::OnAttach()
{

	EvtManagerTop& ec(wm.evtmgr);


	ec.append(new WndModelScript("Model.Search","scripting/ui/dlg_search.ewsl"));

	ec.gp_beg("CmdProc");
		ec.gp_add(new EvtCmdShowSearch(wm,_kT("Find"),CmdProc::CP_FIND));
		ec.gp_add(new EvtCmdShowSearch(wm,_kT("Replace"),CmdProc::CP_REPLACE));

		ec.gp_add(new EvtCmdSearch(wm,_kT("Btn.Find"),CmdProc::CP_FIND));
		ec.gp_add(new EvtCmdSearch(wm,_kT("Btn.Replace"),CmdProc::CP_REPLACE));
		ec.gp_add(new EvtCmdSearch(wm,_kT("Btn.ReplaceAll"),CmdProc::CP_REPLACEALL));
	ec.gp_end();

	ec.append(new EvtCommand("search.replace"));

	IDat_search& data(IDat_search::current());

	ec.gp_beg("search.group");
		ec.link("search.text_old",data.text_old);
		ec.link("search.text_new",data.text_new);
		ec.link("search.case",data.flags,4);
		ec.link("search.word",data.flags,2);
		ec.link("search.regexp",data.flags,0x00200000);
	ec.gp_end();

	ec["search.group"].flags.add(EvtBase::FLAG_RECURSIVE);

	AttachEvent("search.text_old");
	AttachEvent("search.text_new");
	AttachEvent("search.regexp");
	AttachEvent("search.replace");

	return true;
}

IMPLEMENT_IPLUGIN(PluginSearch)

EW_LEAVE
