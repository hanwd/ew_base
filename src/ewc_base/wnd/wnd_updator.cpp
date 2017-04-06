#include "ewc_base/wnd/wnd_updator.h"
#include "ewc_base/wnd/wnd_model.h"
#include "ewc_base/evt/evt_manager.h"

#include "ewc_base/wnd/impl_wx/window.h"
#include <wx/toolbar.h>
#include <wx/aui/auibar.h>

EW_ENTER



WndUpdator::WndUpdator()
{
	m_nLocked=0;
}

void WndUpdator::update()
{
	if(m_nLocked)
	{
		m_bUpdate=true;
	}
	else
	{
		m_bUpdate=false;
		WndManager::current().model.OnChildWindow(NULL,0);
	}
}

void WndUpdator::gp_add(const String& s)
{
	if(m_nLocked)
	{
		m_aUpdate.insert(s);
	}
	else
	{
		WndManager::current().evtmgr[s].UpdateCtrl();
	}
}

void WndUpdator::gp_add(const String& s,int)
{
	WndManager& wm(WndManager::current());
	wm.evtmgr[s].flags.del(EvtCommand::FLAG_G_LOADED);
	gp_add(s);
}

void WndUpdator::tb_add(const String& s,int f)
{
	WndManager& wm(WndManager::current());

	EvtGroup* pg=wm.evtmgr.get_group(s);
	if(!pg) return;

	pg->flags.add(f);
	wxWindow* tb=pg->CreateTbar(ICtlParam("aui_toolbar",24,wm.model.GetWindow()));
	if(!tb) return;
	
	pg->flags.del(EvtCommand::FLAG_DOTDOT);
	pg->flags.add(EvtCommand::FLAG_CHECK);
	pg->flags.set(EvtCommand::FLAG_CHECKED,tb->IsShown());

	tb->SetName(str2wx(s));

	pg->SetWindow(tb);


	EvtManager &ec(wm.evtmgr);
	ec.gp_beg("ToolBars");
		ec.gp_add(s);
	ec.gp_end();

	pg->UpdateCtrl();

}

void WndUpdator::mu_set(const String& s)
{
	WndManager& wm(WndManager::current());

	if(m_nLocked)
	{
		m_sMenubar=s;
	}
	else
	{
		wm.model.mu_set(s);
	}
}

void WndUpdator::tb_set(const String& s)
{
	WndManager& wm(WndManager::current());

	arr_1t<EvtItem> arr;
	wm.evtmgr.item_get(s,arr);
	if(arr.empty()) return;

	for(size_t i=0;i<arr.size();i++)
	{
		tb_add(arr[i].sname,arr[i].flags.val());
	}
}

void WndUpdator::sb_set(const String& s)
{
	WndManager& wm(WndManager::current());

	if(m_nLocked)
	{
		m_sStatbar=s;
	}
	else
	{
		wm.evtmgr[s].StdExecuteEx(1);
	}
}


WndUpdator::operator bool()
{
	return m_nLocked!=0;
}

static wxWindow* g_pMainWindow=NULL;

void WndUpdator::lock()
{
	WndManager& wm(WndManager::current());

	if(m_nLocked++!=0) return;
	g_pMainWindow=wm.model.GetWindow();
	if(g_pMainWindow) g_pMainWindow->Freeze();

	upd.clear();
}

void WndUpdator::unlock()
{
	WndManager& wm(WndManager::current());

	if(--m_nLocked!=0) return;

	if(m_sMenubar!="")
	{
		wm.model.mu_set(m_sMenubar);
		m_sMenubar="";
	}

	if(m_sStatbar!="")
	{
		wm.evtmgr[m_sStatbar].StdExecuteEx(1);
		m_sStatbar="";
	}

	for(indexer_set<String>::iterator it=m_aUpdate.begin();it!=m_aUpdate.end();++it)
	{
		wm.evtmgr[*it].DoUpdateCtrl(upd);
	}
	m_aUpdate.clear();

	if(m_bUpdate)
	{
		m_bUpdate=false;
		wm.model.OnChildWindow(NULL,0);
	}

	if(g_pMainWindow) g_pMainWindow->Thaw();
}

EW_LEAVE
