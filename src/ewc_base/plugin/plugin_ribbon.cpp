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



class IEW_RibbonToolBarImpl : public wxRibbonToolBar, public IEW_Ctrl
{
public:

	IEW_RibbonToolBarImpl(EvtGroup* mu, wxWindow* pw, int wd);
	~IEW_RibbonToolBarImpl();

	bool AddCtrlItem(EvtCommand* pevt);
	bool AddCtrlItem(EvtCommand* pevt, wxControl* p);
	bool AddCtrlItem(EvtGroup* pevt);

	virtual wxWindow* GetWindow(){return this;}

	bool StdExecute(IStdParam& cmd);

	static void EvtPopupMenu(wxWindow*, wxMenu* p, wxRibbonToolBarEvent* e)
	{
		e->PopupMenu(p);
	}

	void OnRibbonToolDropdown(wxRibbonToolBarEvent& evt)
	{
		evt.SetEventType(AppData::current().evt_user_dropdown_menu);
		AppData::current().popup_dropdown_menu.bind(&IEW_RibbonToolBarImpl::EvtPopupMenu, _1, _2, &evt);
		evt.Skip();
	}

	void OnRibbonToolClick(wxRibbonToolBarEvent& evt)
	{
		evt.SetEventType(wxEVT_COMMAND_BUTTON_CLICKED);
		evt.Skip();
	}

protected:

	class IEW_WxCtrlData_tool : public IEW_CtrlData
	{
	public:
		IEW_WxCtrlData_tool(EvtCommand* pevt_, wxRibbonToolBarToolBase* item_, IEW_RibbonToolBarImpl* tbar_) 
			:IEW_CtrlData(pevt_), item(item_),tbar(tbar_){}
		wxRibbonToolBarToolBase* item;
		IEW_RibbonToolBarImpl* tbar;
		void UpdateCtrl();
		void UpdateBmps();
	};

protected:
	void InitToolItem(EvtCommand* pevt, wxRibbonToolBarToolBase* item);

};


IEW_RibbonToolBarImpl::IEW_RibbonToolBarImpl(EvtGroup* pevt, wxWindow* pw, int wd)
:wxRibbonToolBar()
, IEW_Ctrl(pevt)
{
	wd=wd<0?WndManager::current().data.toolbitmap_size:wd;
	this->Create(pw, pevt->m_nId, wxDefaultPosition, wxDefaultSize);
	this->SetName(str2wx(m_pGroup->m_sId));

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

	this->Connect(wxID_ANY, wxEVT_RIBBONTOOLBAR_CLICKED, wxRibbonToolBarEventHandler(IEW_RibbonToolBarImpl::OnRibbonToolClick));
	this->Connect(wxID_ANY, wxEVT_RIBBONTOOLBAR_DROPDOWN_CLICKED, wxRibbonToolBarEventHandler(IEW_RibbonToolBarImpl::OnRibbonToolDropdown));

}

IEW_RibbonToolBarImpl::~IEW_RibbonToolBarImpl()
{

}

bool IEW_RibbonToolBarImpl::StdExecute(IStdParam& cmd)
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

bool IEW_RibbonToolBarImpl::AddCtrlItem(EvtCommand* pevt, wxControl* p)
{

	return false;
}

bool IEW_RibbonToolBarImpl::AddCtrlItem(EvtCommand* pevt)
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

bool IEW_RibbonToolBarImpl::AddCtrlItem(EvtGroup* pevt)
{
	const BitmapBundle& bundle(pevt->GetBundle(16,1));
	wxRibbonToolBarToolBase* item = wxRibbonToolBar::AddTool(pevt->m_nId, bundle.bmp_normal,bundle.bmp_disabled,str2wx(pevt->MakeLabel()), wxRIBBON_BUTTON_DROPDOWN);
	InitToolItem(pevt, item);
	return true;
}



void IEW_RibbonToolBarImpl::IEW_WxCtrlData_tool::UpdateCtrl()
{
	if (pevt->m_nId<0)
	{
		return;
	}

	if (!tbar) return;
	tbar->SetToolHelpString(pevt->m_nId, str2wx(pevt->MakeLabel(EvtBase::LABEL_TOOL)));

	if (pevt->flags.get(EvtBase::FLAG_CHECK))
	{
		tbar->ToggleTool(pevt->m_nId, pevt->flags.get(EvtBase::FLAG_CHECKED));
	}
	

	tbar->EnableTool(pevt->m_nId, !pevt->flags.get(EvtBase::FLAG_DISABLE | EvtBase::FLAG_HIDE_UI));

	//item->SetShortHelp(str2wx(pevt->MakeLabel()));
	//item->SetLabel(str2wx(pevt->MakeLabel(EvtBase::LABEL_TOOL)));
	//item->SetLongHelp(str2wx(pevt->m_sHelp));

}


void IEW_RibbonToolBarImpl::IEW_WxCtrlData_tool::UpdateBmps()
{


}

void IEW_RibbonToolBarImpl::InitToolItem(EvtCommand* pevt, wxRibbonToolBarToolBase* item)
{
	IEW_WxCtrlData_tool* pctrl = new IEW_WxCtrlData_tool(pevt, item,this);
	m_aItems.append(pctrl);
	pctrl->UpdateBmps();
	pctrl->UpdateCtrl();
}

class XXRibbonPanel : public wxRibbonPanel
{
public:
	String name;
	XXRibbonPanel(const String& n) :name(n){}
};

class XXRibbonPanelToolbar : public XXRibbonPanel
{
public:

	XXRibbonPanelToolbar(const String& n,IWindowPtr p,EvtGroup* pevt) :XXRibbonPanel(n)
	{
		Create(p, wxID_ANY, str2wx(Translate(name)),
		wxNullBitmap, wxDefaultPosition, wxDefaultSize,
		wxRIBBON_PANEL_NO_AUTO_MINIMISE |
		wxRIBBON_PANEL_EXT_BUTTON);
		new IEW_RibbonToolBarImpl(pevt, this, 16);
	}

};


class XXRibbonPage : public wxRibbonPage
{
public:
	XXRibbonPage(const String& n) :name(n){}

	String name;
	arr_1t<XXRibbonPanel*> panels;

	void AddPanel(EvtGroup* pevt)
	{
		String panel_name = pevt->m_sId;
		XXRibbonPanelToolbar* p = new XXRibbonPanelToolbar(panel_name, this, pevt);
		for (size_t i = 0; i < panels.size(); i++)
		{
			if (panels[i]->name == panel_name)
			{
				delete panels[i];
				panels[i] = p;
				return;
			}
		}
		panels.push_back(p);
	}

};

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
		wxRibbonBarEvent evt;
		                       


		this->Connect(wxID_ANY,wxEVT_RIBBONBUTTONBAR_CLICKED,  wxRibbonButtonBarEventHandler(XXRibbonBar::OnRibbonButtonClick));
		this->Connect(wxID_ANY,wxEVT_RIBBONBUTTONBAR_DROPDOWN_CLICKED,  wxRibbonButtonBarEventHandler(XXRibbonBar::OnRibbonButtonDropdown));


		this->Connect(wxID_ANY,wxEVT_RIBBONBAR_HELP_CLICK,wxRibbonBarEventHandler(XXRibbonBar::OnRibbonBarHelpClicked));
		this->Connect(wxID_ANY,wxEVT_RIBBONBAR_TOGGLED,wxRibbonBarEventHandler(XXRibbonBar::OnRibbonBarToggled));
		this->Connect(wxID_ANY, wxEVT_RIBBONBAR_PAGE_CHANGED, wxRibbonBarEventHandler(XXRibbonBar::OnRibbonBarPageChanged));
	}





	void OnRibbonButtonDropdown(wxRibbonButtonBarEvent& evt)
	{
		evt.SetEventType(AppData::current().evt_user_dropdown_menu);
		evt.Skip();
	}

	void OnRibbonButtonClick(wxRibbonButtonBarEvent& evt)
	{
		evt.SetEventType(wxEVT_COMMAND_BUTTON_CLICKED);
		evt.Skip();
	}

	void OnRibbonBarToggled(wxRibbonBarEvent&)
	{
		wxSize sz1 = this->GetBestSize();
		sz1.x = -1;

		this->SetSize(sz1);

		wxWindow* p = GetParent();
		p->Layout();
	}

	void OnRibbonBarPageChanged(wxRibbonBarEvent&)
	{
		wxSize sz1 = this->GetBestSize();
		sz1.x = -1;

		this->SetSize(sz1);

		wxWindow* p = GetParent();
		p->Layout();
	}

	void OnRibbonBarHelpClicked(wxRibbonBarEvent&)
	{

	}
	


	XXRibbonPage* find2(const String& name)
	{
		XXRibbonPage* p = NULL;
		for (size_t i = 0; i < pages.size(); i++)
		{
			if (pages[i]->name == name)
			{
				p = pages[i];
				break;
			}
		}
		if (!p)
		{
			p = new XXRibbonPage(name);
			p->Create(this, wxID_ANY, str2wx(Translate(name)), wxNullBitmap);
			pages.push_back(p);
		}
		return p;
	}

	void AddPanel(const String& page_name, EvtGroup* pevt)
	{
		XXRibbonPage* p = find2(page_name);
		p->AddPanel(pevt);
	}


	arr_1t<XXRibbonPage*> pages;

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
	p_ribbon_bar->AddPanel(String::Format("page_%d",n++), pevt);


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

