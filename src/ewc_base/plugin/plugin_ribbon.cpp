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



class ICtl_ribbon_panel : public wxRibbonPanel, public ICtl_object
{
public:
	String name;
	ICtl_ribbon_panel(EvtGroup* pevt) :ICtl_object(pevt),name(pevt->m_sId){}
	virtual wxWindow* GetWindow(){ return this; }
};

class ICtl_ribbon_sizer : public ICtl_ribbon_panel
{
public:

	bool AddWindow(wxWindow* p)
	{
		if (!p) return false;
		p_top_sizer->Add(p);
		return true;
	}

	bool AddCtrlItem(EvtCommand* pevt)
	{
		return AddWindow(pevt->CreateWndsItem(this));
	}

	bool AddCtrlItem(EvtCommand* pevt, wxControl* p)
	{
		return AddWindow(p);
	}

	bool AddCtrlItem(EvtGroup* pevt)
	{
		return false;
	}

	ICtl_ribbon_sizer(IWindowPtr p, EvtGroup* pevt, int wd) :ICtl_ribbon_panel(pevt)
	{
		Create(p, pevt->m_nId, str2wx(Translate(name)),
			wxNullBitmap, wxDefaultPosition, wxDefaultSize,
			wxRIBBON_PANEL_DEFAULT_STYLE |
			wxRIBBON_PANEL_EXT_BUTTON);

		
	}


	wxBoxSizer* p_top_sizer;
	
};


class ICtl_ribbon_gallery : public ICtl_ribbon_panel
{
public:

	bool AddCtrlItem(EvtCommand* pevt)
	{

		if (pevt->flags.get(EvtCommand::FLAG_SEPARATOR))
		{
			return true;
		}


		const BitmapBundle& bundle(pevt->GetBundle(16, 1));
		wxRibbonGalleryItem* item = m_p_wx_impl->Append(bundle.bmp_normal, pevt->m_nId);

		InitToolItem(pevt, item);
		return true;

	}

	bool AddCtrlItem(EvtCommand* pevt, wxControl* p)
	{

		return false;
	}
	bool AddCtrlItem(EvtGroup* pevt)
	{
		return false;
	}

	wxRibbonGallery* m_p_wx_impl;

	ICtl_ribbon_gallery(IWindowPtr p, EvtGroup* pevt, int wd) :ICtl_ribbon_panel(pevt)
	{
		Create(p, pevt->m_nId, str2wx(Translate(name)),
			wxNullBitmap, wxDefaultPosition, wxDefaultSize,
			wxRIBBON_PANEL_NO_AUTO_MINIMISE |
			wxRIBBON_PANEL_EXT_BUTTON);

		wd = 16;
		m_p_wx_impl = new wxRibbonGallery(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
		m_p_wx_impl->SetName(str2wx(m_pGroup->m_sId));

		for (size_t i = 0; i<pevt->size(); i++)
		{
			EvtCommand* pCommand = (*pevt)[i].get();
			if (pCommand->flags.get(EvtBase::FLAG_HIDE_UI)) continue;
			if (pCommand->flags.get(EvtBase::FLAG_SEPARATOR)) continue; 

			pCommand->CreateCtrlItem(this);
		}

		m_p_wx_impl->SetName(str2wx(pevt->m_sId));
		this->Connect(wxID_ANY, wxEVT_RIBBONGALLERY_SELECTED, wxRibbonGalleryEventHandler(ICtl_ribbon_gallery::OnRibbonGallerySelected));
	}


	
	void OnRibbonGallerySelected(wxRibbonGalleryEvent& evt)
	{
		int evtid = -1;
		for (size_t i = 0; i < m_aItems.size(); i++)
		{
			if (static_cast<ICtl_wxctrl_itemdata*>(m_aItems[i].get())->item == evt.GetGalleryItem())
			{
				evtid = m_aItems[i]->pevt->m_nId;
				break;
			}
		}
		evt.SetEventType(wxEVT_COMMAND_BUTTON_CLICKED);
		evt.SetId(evtid);
		evt.Skip();
	}

	bool StdExecute(IStdParam& cmd)
	{
		return true;
	}


protected:

	class ICtl_wxctrl_itemdata : public ICtl_itemdata
	{
	public:
		ICtl_wxctrl_itemdata(EvtCommand* pevt_, wxRibbonGalleryItem* item_, wxRibbonGallery* tbar_)
			:ICtl_itemdata(pevt_), item(item_), tbar(tbar_){}
		wxRibbonGalleryItem* item;
		wxRibbonGallery* tbar;
		void UpdateCtrl(){}
	};

protected:
	void InitToolItem(EvtCommand* pevt, wxRibbonGalleryItem* item)
	{
		m_aItems.append(new ICtl_wxctrl_itemdata(pevt, item, m_p_wx_impl));
	}

};

class ICtl_ribbon_toolbar : public ICtl_ribbon_panel
{
public:

	bool AddCtrlItem(EvtCommand* pevt);
	bool AddCtrlItem(EvtCommand* pevt, wxControl* p);
	bool AddCtrlItem(EvtGroup* pevt);

	wxRibbonToolBar* m_p_wx_impl;

	ICtl_ribbon_toolbar(IWindowPtr p, EvtGroup* pevt,int wd) :ICtl_ribbon_panel(pevt)
	{
		Create(p, pevt->m_nId, str2wx(Translate(name)),
			wxNullBitmap, wxDefaultPosition, wxDefaultSize,
			wxRIBBON_PANEL_NO_AUTO_MINIMISE |
			wxRIBBON_PANEL_EXT_BUTTON);

		wd = 16;
		m_p_wx_impl=new wxRibbonToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
		m_p_wx_impl->SetName(str2wx(m_pGroup->m_sId));

		for (size_t i = 0; i<pevt->size(); i++)
		{
			EvtCommand* pCommand = (*pevt)[i].get();
			if (pCommand->flags.get(EvtBase::FLAG_HIDE_UI)) continue;
			if (pCommand->flags.get(EvtBase::FLAG_SEPARATOR)) { m_p_wx_impl->AddSeparator(); continue; }

			pCommand->CreateCtrlItem(this);
		}

		m_p_wx_impl->SetRows(2, -1);

		bool flag = m_p_wx_impl->Realize();

		if (!flag)
		{
			System::LogMessage("wxToolBar realize failed!");
		}

		if (pevt->flags.get(EvtBase::FLAG_HIDE_UI)) m_p_wx_impl->Show(false);
		if (pevt->flags.get(EvtBase::FLAG_DISABLE)) m_p_wx_impl->Enable(false);

		pevt->flags.set(EvtBase::FLAG_CHECKED, !pevt->flags.get(EvtBase::FLAG_HIDE_UI));

		m_p_wx_impl->SetName(str2wx(pevt->m_sId));

		this->Connect(wxID_ANY, wxEVT_RIBBONTOOLBAR_CLICKED, wxRibbonToolBarEventHandler(ICtl_ribbon_toolbar::OnRibbonToolClick));
		this->Connect(wxID_ANY, wxEVT_RIBBONTOOLBAR_DROPDOWN_CLICKED, wxRibbonToolBarEventHandler(ICtl_ribbon_toolbar::OnRibbonToolDropdown));
	}


	static void EvtPopupMenu(wxWindow*, wxMenu* p, wxRibbonToolBarEvent* e)
	{
		e->PopupMenu(p);
	}

	void OnRibbonToolDropdown(wxRibbonToolBarEvent& evt)
	{
		evt.SetEventType(AppData::current().evt_user_dropdown_menu);
		AppData::current().popup_dropdown_menu.bind(&ICtl_ribbon_toolbar::EvtPopupMenu, _1, _2, &evt);
		evt.Skip();
	}

	void OnRibbonToolClick(wxRibbonToolBarEvent& evt)
	{
		evt.SetEventType(wxEVT_COMMAND_BUTTON_CLICKED);
		evt.Skip();
	}


	bool StdExecute(IStdParam& cmd);


protected:

	class ICtl_wxctrl_itemdata : public ICtl_itemdata
	{
	public:
		ICtl_wxctrl_itemdata(EvtCommand* pevt_, wxRibbonToolBarToolBase* item_, wxRibbonToolBar* tbar_)
			:ICtl_itemdata(pevt_), item(item_), tbar(tbar_){}
		wxRibbonToolBarToolBase* item;
		wxRibbonToolBar* tbar;
		void UpdateCtrl();
	};

protected:
	void InitToolItem(EvtCommand* pevt, wxRibbonToolBarToolBase* item);

};


bool ICtl_ribbon_toolbar::StdExecute(IStdParam& cmd)
{
	if(cmd.extra1=="clear")
	{
		m_aItems.clear();
		DestroyChildren();	
	}
	else if(cmd.extra1=="update")
	{
		this->Enable(!m_pGroup->flags.get(EvtBase::FLAG_DISABLE));
	}
	return true;
}

bool ICtl_ribbon_toolbar::AddCtrlItem(EvtCommand* pevt, wxControl* p)
{
	return false;
}

bool ICtl_ribbon_toolbar::AddCtrlItem(EvtCommand* pevt)
{
	if (pevt->flags.get(EvtCommand::FLAG_SEPARATOR))
	{
		m_p_wx_impl->AddSeparator();
		return true;
	}

	wxRibbonButtonKind kind = wxRIBBON_BUTTON_NORMAL;
	if (pevt->flags.get(EvtCommand::FLAG_CHECK)) kind = wxRIBBON_BUTTON_TOGGLE;

	const BitmapBundle& bundle(pevt->GetBundle(16,1));
	wxRibbonToolBarToolBase* item = m_p_wx_impl->AddTool(pevt->m_nId, bundle.bmp_normal, bundle.bmp_disabled, str2wx(pevt->MakeLabel()), kind);

	InitToolItem(pevt, item);
	return true;
}

bool ICtl_ribbon_toolbar::AddCtrlItem(EvtGroup* pevt)
{
	const BitmapBundle& bundle(pevt->GetBundle(16,1));
	wxRibbonToolBarToolBase* item = m_p_wx_impl->AddTool(pevt->m_nId, bundle.bmp_normal, bundle.bmp_disabled, str2wx(pevt->MakeLabel()), wxRIBBON_BUTTON_DROPDOWN);
	InitToolItem(pevt, item);
	return true;
}

void ICtl_ribbon_toolbar::ICtl_wxctrl_itemdata::UpdateCtrl()
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
}



void ICtl_ribbon_toolbar::InitToolItem(EvtCommand* pevt, wxRibbonToolBarToolBase* item)
{
	ICtl_wxctrl_itemdata* pctrl = new ICtl_wxctrl_itemdata(pevt, item,m_p_wx_impl);
	m_aItems.append(pctrl);
	pctrl->UpdateCtrl();
}

class ICtl_ribbon_buttonbar : public ICtl_ribbon_panel
{
public:

	bool AddCtrlItem(EvtCommand* pevt);
	bool AddCtrlItem(EvtCommand* pevt, wxControl* p);
	bool AddCtrlItem(EvtGroup* pevt);

	wxRibbonButtonBar* m_p_wx_impl;

	ICtl_ribbon_buttonbar(IWindowPtr p, EvtGroup* pevt, int wd) :ICtl_ribbon_panel(pevt)
	{
		Create(p, pevt->m_nId, str2wx(Translate(name)),
			wxNullBitmap, wxDefaultPosition, wxDefaultSize,
			wxRIBBON_PANEL_NO_AUTO_MINIMISE |
			wxRIBBON_PANEL_EXT_BUTTON);

		wd = 16;
		m_p_wx_impl = new wxRibbonButtonBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
		m_p_wx_impl->SetName(str2wx(m_pGroup->m_sId));

		for (size_t i = 0; i<pevt->size(); i++)
		{
			EvtCommand* pCommand = (*pevt)[i].get();
			if (pCommand->flags.get(EvtBase::FLAG_HIDE_UI)) continue;
			if (pCommand->flags.get(EvtBase::FLAG_SEPARATOR)) { continue; }

			pCommand->CreateCtrlItem(this);
		}

		bool flag = m_p_wx_impl->Realize();

		if (!flag)
		{
			System::LogMessage("wxRibbonButtonBar realize failed!");
		}

		if (pevt->flags.get(EvtBase::FLAG_HIDE_UI)) m_p_wx_impl->Show(false);
		if (pevt->flags.get(EvtBase::FLAG_DISABLE)) m_p_wx_impl->Enable(false);

		pevt->flags.set(EvtBase::FLAG_CHECKED, !pevt->flags.get(EvtBase::FLAG_HIDE_UI));

		m_p_wx_impl->SetName(str2wx(pevt->m_sId));

		this->Connect(wxID_ANY, wxEVT_RIBBONBUTTONBAR_CLICKED, wxRibbonButtonBarEventHandler(ICtl_ribbon_buttonbar::OnRibbonButtonClick));
		this->Connect(wxID_ANY, wxEVT_RIBBONBUTTONBAR_DROPDOWN_CLICKED, wxRibbonButtonBarEventHandler(ICtl_ribbon_buttonbar::OnRibbonButtonDropdown));
	}


	static void EvtPopupMenu(wxWindow*, wxMenu* p, wxRibbonButtonBarEvent* e)
	{
		e->PopupMenu(p);
	}

	void OnRibbonButtonDropdown(wxRibbonButtonBarEvent& evt)
	{
		evt.SetEventType(AppData::current().evt_user_dropdown_menu);
		AppData::current().popup_dropdown_menu.bind(&ICtl_ribbon_buttonbar::EvtPopupMenu, _1, _2, &evt);
		evt.Skip();
	}

	void OnRibbonButtonClick(wxRibbonButtonBarEvent& evt)
	{
		evt.SetEventType(wxEVT_COMMAND_BUTTON_CLICKED);
		evt.Skip();
	}


	bool StdExecute(IStdParam& cmd);


protected:

	class ICtl_wxctrl_itemdata : public ICtl_itemdata
	{
	public:
		ICtl_wxctrl_itemdata(EvtCommand* pevt_, wxRibbonButtonBarButtonBase* item_, wxRibbonButtonBar* tbar_)
			:ICtl_itemdata(pevt_), item(item_), tbar(tbar_){}
		wxRibbonButtonBarButtonBase* item;
		wxRibbonButtonBar* tbar;
		void UpdateCtrl();
	};

protected:
	void InitToolItem(EvtCommand* pevt, wxRibbonButtonBarButtonBase* item);

};


bool ICtl_ribbon_buttonbar::StdExecute(IStdParam& cmd)
{
	if (cmd.extra1 == "clear")
	{
		
		m_aItems.clear();
		DestroyChildren();
	}
	else if (cmd.extra1 == "update")
	{
		this->Enable(!m_pGroup->flags.get(EvtBase::FLAG_DISABLE));
	}
	return true;
}

bool ICtl_ribbon_buttonbar::AddCtrlItem(EvtCommand* pevt, wxControl* p)
{
	return false;
}

bool ICtl_ribbon_buttonbar::AddCtrlItem(EvtCommand* pevt)
{
	if (pevt->flags.get(EvtCommand::FLAG_SEPARATOR))
	{
		return true;
	}

	wxRibbonButtonKind kind = wxRIBBON_BUTTON_NORMAL;
	if (pevt->flags.get(EvtCommand::FLAG_CHECK)) kind = wxRIBBON_BUTTON_TOGGLE;

	const BitmapBundle& bundle(pevt->GetBundle(16, 1));
	wxRibbonButtonBarButtonBase* item = m_p_wx_impl->AddButton(pevt->m_nId,  str2wx(pevt->MakeLabel()),bundle.bmp_normal,"", kind);

	InitToolItem(pevt, item);
	return true;
}

bool ICtl_ribbon_buttonbar::AddCtrlItem(EvtGroup* pevt)
{
	const BitmapBundle& bundle(pevt->GetBundle(16, 1));
	wxRibbonButtonBarButtonBase* item = m_p_wx_impl->AddButton(pevt->m_nId, str2wx(pevt->MakeLabel()),bundle.bmp_normal,"",   wxRIBBON_BUTTON_DROPDOWN);
	InitToolItem(pevt, item);
	return true;
}

void ICtl_ribbon_buttonbar::ICtl_wxctrl_itemdata::UpdateCtrl()
{
	if (pevt->m_nId<0)
	{
		return;
	}

	if (!tbar) return;

	if (pevt->flags.get(EvtBase::FLAG_CHECK))
	{
		tbar->ToggleButton(pevt->m_nId, pevt->flags.get(EvtBase::FLAG_CHECKED));
	}

	tbar->EnableButton(pevt->m_nId, !pevt->flags.get(EvtBase::FLAG_DISABLE | EvtBase::FLAG_HIDE_UI));
}



void ICtl_ribbon_buttonbar::InitToolItem(EvtCommand* pevt, wxRibbonButtonBarButtonBase* item)
{
	ICtl_wxctrl_itemdata* pctrl = new ICtl_wxctrl_itemdata(pevt, item, m_p_wx_impl);
	m_aItems.append(pctrl);
	pctrl->UpdateCtrl();
}




class ICtl_ribbon_page : public wxRibbonPage
{
public:
	ICtl_ribbon_page(const String& n) :name(n){}

	String name;
	arr_1t<ICtl_ribbon_panel*> panels;

	void AddPanel(EvtGroup* pevt)
	{
		String panel_name = pevt->m_sId;
		ICtl_ribbon_toolbar* p = new ICtl_ribbon_toolbar(this, pevt,16);
		//ICtl_ribbon_buttonbar* p = new ICtl_ribbon_buttonbar(this, pevt, 16);
		//ICtl_ribbon_sizer* p=new ICtl_ribbon_sizer(this, pevt, 16);
		//ICtl_ribbon_gallery* p = new ICtl_ribbon_gallery(this, pevt, 16);
		for (size_t i = 0; i < panels.size(); i++)
		{
			if (panels[i]->name == panel_name)
			{
				delete panels[i];
				panels[i] = p;
				return;
			}
		}
		p->Layout();
		panels.push_back(p);
	}

};

class ICtl_ribbon_bar : public wxRibbonBar, public ICtl_object
{
public:
	ICtl_ribbon_bar(wxWindow * p):wxRibbonBar(p,-1,wxDefaultPosition, wxDefaultSize
								, wxRIBBON_BAR_FLOW_HORIZONTAL
                                | wxRIBBON_BAR_SHOW_PAGE_LABELS
								| wxRIBBON_BAR_SHOW_PAGE_ICONS
                                | wxRIBBON_BAR_SHOW_PANEL_EXT_BUTTONS
                                | wxRIBBON_BAR_SHOW_TOGGLE_BUTTON
                                | wxRIBBON_BAR_SHOW_HELP_BUTTON
								| wxRIBBON_BAR_SHOW_PANEL_MINIMISE_BUTTONS
                                )
	{

		this->Connect(wxID_ANY,wxEVT_RIBBONBAR_HELP_CLICK,wxRibbonBarEventHandler(ICtl_ribbon_bar::OnRibbonBarHelpClicked));
	}


	void OnRibbonBarHelpClicked(wxRibbonBarEvent&)
	{

	}
	


	ICtl_ribbon_page* find2(const String& name)
	{
		ICtl_ribbon_page* p = NULL;
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
			p = new ICtl_ribbon_page(name);
			p->Create(this, wxID_ANY, str2wx(Translate(name)), wxNullBitmap);
			pages.push_back(p);
		}
		return p;
	}

	void AddPanel(const String& page_name, EvtGroup* pevt)
	{
		//ICtl_ribbon_page* p = NULL;
		//for (size_t i = 0; i < pages.size(); i++)
		//{
		//	if (pages[i]->name == page_name)
		//	{
		//		p = pages[i];
		//		break;
		//	}
		//}
		//if (!p)
		//{
		//	p = new ICtl_ribbon_page(page_name);
		//	const BitmapBundle& bundle(pevt->GetBundle(16,1));
		//	p->Create(this, wxID_ANY, str2wx(Translate(page_name)), bundle.bmp_normal);
		//	pages.push_back(p);
		//}
		ICtl_ribbon_page* p = find2(page_name);
		p->AddPanel(pevt);
	}


	arr_1t<ICtl_ribbon_page*> pages;

	wxWindow* GetWindow(){return this;}

};



static ICtl_ribbon_bar* p_ribbon_bar=NULL;

static ICtl_object* WndCreateRibbonToolBar(const ICtlParam& param,EvtGroup* pevt)
{
	bool first=!p_ribbon_bar;

	if(!p_ribbon_bar)
	{
		p_ribbon_bar=new ICtl_ribbon_bar(param.parent);	
	}

	static int n=0;
	p_ribbon_bar->AddPanel(String::Format("page_%d",n++), pevt);
	p_ribbon_bar->Realize();

	if(first)
	{		
		WndModel::current().OnChildWindow(p_ribbon_bar, IDefs::WND_ATTACH);
	}

	return NULL;

}


PluginRibbon::PluginRibbon(WndManager& w):PluginCommon(w,"Plugin.Ribbon")
{

}

bool PluginRibbon::OnAttach()
{

	EvtManager& ec(wm.evtmgr);

	ICtl_object::WndRegister("ribbon_toolbar",&WndCreateRibbonToolBar);

	return true;

}

IMPLEMENT_IPLUGIN(PluginRibbon)

EW_LEAVE

