#include "ewc_base/plugin/plugin_ribbon.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/evt/evt_base.h"
#include "ewc_base/evt/evt_cmdproc.h"
#include "ewc_base/evt/evt_command.h"
#include "ewc_base/evt/evt_group.h"

#include "ewc_base/wnd/impl_wx/window.h"
#include "wx/ribbon/bar.h"
#include "wx/ribbon/buttonbar.h"
#include "wx/ribbon/gallery.h"
#include "wx/ribbon/toolbar.h"

EW_ENTER



class IEW_RibbonImpl : public wxRibbonToolBar, public IEW_Ctrl
{
public:

	IEW_RibbonImpl(EvtGroup* mu, wxWindow* pw, int wd);
	~IEW_RibbonImpl();

	bool AddCtrlItem(EvtCommand* pevt);
	bool AddCtrlItem(EvtCommand* pevt, wxControl* p);
	bool AddCtrlItem(EvtGroup* pevt);

	virtual wxWindow* GetWindow(){return this;}

	bool StdExecute(IStdParam& cmd);


protected:

	class IEW_WxCtrlData_tool : public IEW_CtrlData
	{
	public:
		IEW_WxCtrlData_tool(EvtCommand* pevt_, wxRibbonToolBarToolBase* item_) :IEW_CtrlData(pevt_), item(item_){}
		wxRibbonToolBarToolBase* item;

		void UpdateCtrl();
		void UpdateBmps();
	};

protected:
	void InitToolItem(EvtCommand* pevt, wxRibbonToolBarToolBase* item);

};


IEW_RibbonImpl::IEW_RibbonImpl(EvtGroup* pevt, wxWindow* pw, int wd)
:wxRibbonToolBar()
, IEW_Ctrl(pevt)
{
	wd=wd<0?WndManager::current().data.toolbitmap_size:wd;
	this->Create(pw, pevt->m_nId, wxDefaultPosition, wxDefaultSize);
	this->SetName(str2wx(m_pGroup->m_sId));
	//this->SetToolBitmapSize(wxSize(wd, wd));

	for(size_t i=0;i<pevt->size();i++)
	{
		EvtCommand* pCommand=(*pevt)[i].get();
		if(pCommand->flags.get(EvtBase::FLAG_HIDE_UI)) continue;
		if(pCommand->flags.get(EvtBase::FLAG_SEPARATOR)) {this->AddSeparator();continue;}

		pCommand->CreateCtrlItem(this);
	}

	this->SetRows(2, -1);

	bool flag=this->Realize();

	if (!flag)
	{
		System::LogMessage("wxToolBar realize failed!");
	}

	if(pevt->flags.get(EvtBase::FLAG_HIDE_UI)) this->Show(false);
	if(pevt->flags.get(EvtBase::FLAG_DISABLE)) this->Enable(false);

	pevt->flags.set(EvtBase::FLAG_CHECKED,!pevt->flags.get(EvtBase::FLAG_HIDE_UI));

	this->SetName(str2wx(pevt->m_sId));

}

IEW_RibbonImpl::~IEW_RibbonImpl()
{

}

bool IEW_RibbonImpl::StdExecute(IStdParam& cmd)
{
	if(cmd.extra1=="clear")
	{
		m_aItems.clear();
		int nc=this->GetToolCount();
		while(--nc>=0)
		{
			this->DeleteToolByPos(0);
		}		
	}
	else if(cmd.extra1=="update")
	{
		this->Enable(!m_pGroup->flags.get(EvtBase::FLAG_DISABLE));
	}
	return true;
}

bool IEW_RibbonImpl::AddCtrlItem(EvtCommand* pevt, wxControl* p)
{

	return false;
}

bool IEW_RibbonImpl::AddCtrlItem(EvtCommand* pevt)
{
	if (pevt->flags.get(EvtCommand::FLAG_SEPARATOR))
	{
		this->AddSeparator();
		return true;
	}

	wxRibbonButtonKind kind = wxRIBBON_BUTTON_NORMAL;
	if (pevt->flags.get(EvtCommand::FLAG_CHECK)) kind = wxRIBBON_BUTTON_TOGGLE;

	const BitmapBundle& bundle(pevt->GetBundle(16,1));
	wxRibbonToolBarToolBase* item = wxRibbonToolBar::AddTool(pevt->m_nId, bundle.bmp_normal,bundle.bmp_disabled,str2wx(pevt->MakeLabel()), kind);

	InitToolItem(pevt, item);
	return true;
}

bool IEW_RibbonImpl::AddCtrlItem(EvtGroup* pevt)
{
	const BitmapBundle& bundle(pevt->GetBundle(16,1));
	wxRibbonToolBarToolBase* item = wxRibbonToolBar::AddTool(pevt->m_nId, bundle.bmp_normal,bundle.bmp_disabled,str2wx(pevt->MakeLabel()), wxRIBBON_BUTTON_DROPDOWN);
	//item->SetDropdownMenu(pevt->CreateMenu());
	InitToolItem(pevt, item);
	return true;
}


void IEW_RibbonImpl::IEW_WxCtrlData_tool::UpdateCtrl()
{
	if (pevt->m_nId<0)
	{
		return;
	}

	

	IEW_RibbonImpl* tb = (IEW_RibbonImpl*)NULL;//item->GetToolBar();
	if (!tb) return;

	//item->SetShortHelp(str2wx(pevt->MakeLabel()));
	//item->SetLabel(str2wx(pevt->MakeLabel(EvtBase::LABEL_TOOL)));
	//item->SetLongHelp(str2wx(pevt->m_sHelp));

	//if (pevt->flags.get(EvtBase::FLAG_CHECK))
	//{
	//	item->SetToggle(true);
	//	tb->ToggleTool(pevt->m_nId, pevt->flags.get(EvtBase::FLAG_CHECKED));
	//}
	tb->EnableTool(pevt->m_nId, !pevt->flags.get(EvtBase::FLAG_DISABLE | EvtBase::FLAG_HIDE_UI));
}


void IEW_RibbonImpl::IEW_WxCtrlData_tool::UpdateBmps()
{


}

void IEW_RibbonImpl::InitToolItem(EvtCommand* pevt, wxRibbonToolBarToolBase* item)
{
	IEW_WxCtrlData_tool* pctrl = new IEW_WxCtrlData_tool(pevt, item);
	m_aItems.append(pctrl);
	pctrl->UpdateBmps();
	pctrl->UpdateCtrl();
}


class XXRibbonBar : public wxRibbonBar, public IEW_Ctrl
{
public:
	XXRibbonBar(wxWindow * p):wxRibbonBar(p,-1,wxDefaultPosition, wxDefaultSize, wxRIBBON_BAR_FLOW_HORIZONTAL
                                | wxRIBBON_BAR_SHOW_PAGE_LABELS
                                | wxRIBBON_BAR_SHOW_PANEL_EXT_BUTTONS
                                | wxRIBBON_BAR_SHOW_TOGGLE_BUTTON
                                | wxRIBBON_BAR_SHOW_HELP_BUTTON
                                )
	{
        


	}

	wxWindow* GetWindow(){return this;}


};


static XXRibbonBar* p_ribbon_bar=NULL;

static IEW_Ctrl* WndCreateRibbonToolBar(const ICtlParam& param,EvtGroup* pevt)
{
	bool first=!p_ribbon_bar;

	if(!p_ribbon_bar)
	{
		p_ribbon_bar=new XXRibbonBar(param.parent);	
	}

	static int n=0;

	wxRibbonPage* page = new wxRibbonPage(p_ribbon_bar, wxID_ANY, wxT("Examples"), wxNullBitmap);
    wxRibbonPanel* t = new wxRibbonPanel(page, wxID_ANY, wxT("Toolbar"), 
                                        wxNullBitmap, wxDefaultPosition, wxDefaultSize, 
                                        wxRIBBON_PANEL_NO_AUTO_MINIMISE |
                                        wxRIBBON_PANEL_EXT_BUTTON);

	new IEW_RibbonImpl(pevt,t,param.bmpsize);

	if(first)
	{
		p_ribbon_bar->Realize();
		return p_ribbon_bar;
	}
	else
	{
		p_ribbon_bar->Realize();
		return NULL;
	}

}


PluginRibbon::PluginRibbon(WndManager& w):PluginCommon(w,"Plugin.Ribbon")
{

}

bool PluginRibbon::OnAttach()
{

	EvtManager& ec(wm.evtmgr);

	IEW_Ctrl::WndRegister("ribbon_toolbar",&WndCreateRibbonToolBar);

	return true;

}

IMPLEMENT_IPLUGIN(PluginRibbon)

EW_LEAVE

