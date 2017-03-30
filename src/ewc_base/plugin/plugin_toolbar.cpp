#include "ewc_base/plugin/plugin_toolbar.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/evt/evt_base.h"
#include "ewc_base/evt/evt_cmdproc.h"
#include "ewc_base/evt/evt_command.h"
#include "ewc_base/evt/evt_group.h"
#include "ewc_base/app/res_manager.h"

#include "ewc_base/wnd/impl_wx/window.h"
#include "wx/aui/auibar.h"


EW_ENTER

PluginToolBar::PluginToolBar(WndManager& w):PluginCommon(w,"Plugin.ToolBar")
{

}

class ICtl_toolbar : public wxToolBar, public ICtl_object
{
public:

	ICtl_toolbar(EvtGroup* mu, wxWindow* pw, int wd);
	~ICtl_toolbar();

	bool AddCtrlItem(EvtCommand* pevt);
	bool AddCtrlItem(EvtCommand* pevt, wxControl* p);
	bool AddCtrlItem(EvtGroup* pevt);

	virtual wxWindow* GetWindow(){return this;}

	bool StdExecute(IStdParam& cmd);


protected:

	class ICtl_wxctrl_itemdata : public ICtl_itemdata
	{
	public:
		ICtl_wxctrl_itemdata(EvtCommand* pevt_, IToolItemPtr item_) :ICtl_itemdata(pevt_), item(item_){}
		IToolItemPtr item;

		void UpdateCtrl();
		void UpdateBmps();
	};

protected:
	void InitToolItem(EvtCommand* pevt, IToolItemPtr item);

};


class ICtl_aui_toolbar : public wxAuiToolBar, public ICtl_object
{
public:

	ICtl_aui_toolbar(EvtGroup* mu, wxWindow* pw, int wd);
	~ICtl_aui_toolbar();

	bool AddCtrlItem(EvtCommand* pevt);
	bool AddCtrlItem(EvtCommand* pevt, wxControl* p);
	bool AddCtrlItem(EvtGroup* pevt);

	virtual wxWindow* GetWindow(){return this;}

	bool StdExecute(IStdParam& cmd);

protected:

	class ICtl_wxctrl_itemdata : public ICtl_itemdata
	{
	public:
		ICtl_wxctrl_itemdata(EvtCommand* pevt_, IAuiToolItemPtr item_) :ICtl_itemdata(pevt_), item(item_){}
		IAuiToolItemPtr item;
		ICtl_aui_toolbar* tbar;
		void UpdateCtrl();
		void UpdateBmps();
	};

protected:
	void InitToolItem(EvtCommand* pevt, IAuiToolItemPtr item);




	void OnDropDown(wxAuiToolBarEvent& evt)
	{
		if (!evt.IsDropDownClicked())
		{
			evt.Skip();
			return;
		}

		evt.SetEventType(AppData::current().evt_user_dropdown_menu);
		AppData::current().popup_dropdown_menu.bind(&ICtl_aui_toolbar::EvtPopupMenu, _1, _2, evt);

		evt.Skip();
	}

	static void EvtPopupMenu(wxWindow*, wxMenu* menu, wxAuiToolBarEvent& evt)
	{
		int evtid = evt.GetId();
		wxAuiToolBar* tbar = static_cast<wxAuiToolBar*>(evt.GetEventObject());
		if (!tbar) return;
		wxRect rect = tbar->GetToolRect(evtid);
		tbar->PopupMenu(menu, rect.GetBottomLeft());
	}



};



ICtl_toolbar::ICtl_toolbar(EvtGroup* pevt, wxWindow* pw, int wd)
:wxToolBar()
, ICtl_object(pevt)
{
	wd=wd<0?WndManager::current().data.toolbitmap_size:wd;
	this->Create(pw, pevt->m_nId, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
	this->SetName(str2wx(m_pGroup->m_sId));
	this->SetToolBitmapSize(wxSize(wd, wd));

	for(size_t i=0;i<pevt->size();i++)
	{
		EvtCommand* pCommand=(*pevt)[i].get();
		if(pCommand->flags.get(EvtBase::FLAG_HIDE_UI)) continue;
		if(pCommand->flags.get(EvtBase::FLAG_SEPARATOR)) {this->AddSeparator();continue;}

		//this->AddCtrlItem(pCommand);
		pCommand->CreateCtrlItem(this);
	}

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

ICtl_toolbar::~ICtl_toolbar()
{

}

bool ICtl_toolbar::StdExecute(IStdParam& cmd)
{
	if(cmd.extra1=="clear")
	{
		m_aItems.clear();
		ClearTools();
	}
	else if(cmd.extra1=="update")
	{
		this->Enable(!m_pGroup->flags.get(EvtBase::FLAG_DISABLE));
	}
	return true;
}

bool ICtl_toolbar::AddCtrlItem(EvtCommand* pevt, wxControl* p)
{
	pevt->CreateValidator(p);
	IToolItemPtr item = wxToolBar::AddControl(p);
	InitToolItem(pevt, item);
	return true;
}

bool ICtl_toolbar::AddCtrlItem(EvtCommand* pevt)
{
	if (pevt->flags.get(EvtCommand::FLAG_SEPARATOR))
	{
		this->AddSeparator();
		return true;
	}

	wxItemKind kind = wxITEM_NORMAL;
	if (pevt->flags.get(EvtCommand::FLAG_CHECK)) kind = wxITEM_CHECK;

	IToolItemPtr item = wxToolBar::AddTool(pevt->m_nId, str2wx(pevt->MakeLabel()), wxNullBitmap, wxString(), kind);
	InitToolItem(pevt, item);
	return true;
}

bool ICtl_toolbar::AddCtrlItem(EvtGroup* pevt)
{
	IToolItemPtr item = wxToolBar::AddTool(pevt->m_nId, str2wx(pevt->MakeLabel()), wxNullBitmap, wxString(), wxITEM_DROPDOWN);
	item->SetDropdownMenu(pevt->CreateMenu());
	InitToolItem(pevt, item);
	return true;
}


void ICtl_toolbar::ICtl_wxctrl_itemdata::UpdateCtrl()
{
	if (pevt->m_nId<0)
	{
		return;
	}

	ICtl_toolbar* tb = (ICtl_toolbar*)item->GetToolBar();
	if (!tb) return;

	item->SetShortHelp(str2wx(pevt->MakeLabel()));
	item->SetLabel(str2wx(pevt->MakeLabel(EvtBase::LABEL_TOOL)));
	item->SetLongHelp(str2wx(pevt->m_sHelp));

	if (pevt->flags.get(EvtBase::FLAG_CHECK))
	{
		item->SetToggle(true);
		tb->ToggleTool(pevt->m_nId, pevt->flags.get(EvtBase::FLAG_CHECKED));
	}
	tb->EnableTool(pevt->m_nId, !pevt->flags.get(EvtBase::FLAG_DISABLE | EvtBase::FLAG_HIDE_UI));
}


void ICtl_toolbar::ICtl_wxctrl_itemdata::UpdateBmps()
{
	wxToolBarBase* tb=item->GetToolBar();
	wxSize sz=tb->GetToolBitmapSize();

	const BitmapBundle& bundle(pevt->GetBundle(sz.y,1));
	if(!bundle.IsOk())
	{
		return;
	}

	item->SetNormalBitmap(bundle.bmp_normal);
	item->SetDisabledBitmap(bundle.bmp_disabled);

}

void ICtl_toolbar::InitToolItem(EvtCommand* pevt, IToolItemPtr item)
{
	ICtl_wxctrl_itemdata* pctrl = new ICtl_wxctrl_itemdata(pevt, item);
	m_aItems.append(pctrl);
	pctrl->UpdateBmps();
	pctrl->UpdateCtrl();
}





ICtl_aui_toolbar::ICtl_aui_toolbar(EvtGroup* pevt, wxWindow* pw, int wd)
: ICtl_object(pevt)
{
	wd=wd<0?WndManager::current().data.toolbitmap_size:wd;
	this->Create(pw, pevt->m_nId, wxDefaultPosition, wxDefaultSize);

	this->SetName(str2wx(m_pGroup->m_sId));
	this->SetToolBitmapSize(wxSize(wd, wd));



	for(size_t i=0;i<pevt->size();i++)
	{
		EvtCommand* pCommand=(*pevt)[i].get();
		if(pCommand->flags.get(EvtBase::FLAG_HIDE_UI)) continue;
		if(pCommand->flags.get(EvtBase::FLAG_SEPARATOR)) {this->AddSeparator();continue;}

		pCommand->CreateCtrlItem(this);
	}

	bool flag=this->Realize();

	if (!flag)
	{
		System::LogMessage("tb realize failed!");
	}

	if(pevt->flags.get(EvtBase::FLAG_HIDE_UI)) this->Show(false);
	if(pevt->flags.get(EvtBase::FLAG_DISABLE)) this->Enable(false);

	pevt->flags.set(EvtBase::FLAG_CHECKED,!pevt->flags.get(EvtBase::FLAG_HIDE_UI));

	this->SetName(str2wx(pevt->m_sId));


	this->Connect(wxID_ANY, wxEVT_AUITOOLBAR_TOOL_DROPDOWN, wxAuiToolBarEventHandler(ICtl_aui_toolbar::OnDropDown));

}

ICtl_aui_toolbar::~ICtl_aui_toolbar()
{

}

bool ICtl_aui_toolbar::AddCtrlItem(EvtCommand* pevt, wxControl* p)
{
	pevt->CreateValidator(p);
	IAuiToolItemPtr item = AddControl(p);
	InitToolItem(pevt, item);
	return true;
}

bool ICtl_aui_toolbar::AddCtrlItem(EvtCommand* pevt)
{
	if (pevt->flags.get(EvtCommand::FLAG_SEPARATOR))
	{
		this->AddSeparator();
		return true;
	}

	wxString label = str2wx(pevt->MakeLabel(EvtBase::LABEL_TOOL));
	wxItemKind kind = wxITEM_NORMAL;
	if(pevt->flags.get(EvtCommand::FLAG_CHECK)) kind=wxITEM_CHECK;

	IAuiToolItemPtr item = AddTool(pevt->m_nId, label, wxNullBitmap, label, kind);
	InitToolItem(pevt, item);
	return true;
}

bool ICtl_aui_toolbar::AddCtrlItem(EvtGroup* pevt)
{
	IAuiToolItemPtr item = AddTool(pevt->m_nId, str2wx(pevt->MakeLabel()), wxNullBitmap);
	item->SetHasDropDown(true);
	InitToolItem(pevt, item);
	return true;
}


void ICtl_aui_toolbar::ICtl_wxctrl_itemdata::UpdateCtrl()
{
	if (pevt->m_nId<0)
	{
		return;
	}

	item->SetShortHelp(str2wx(pevt->MakeLabel()));
	item->SetLabel(str2wx(pevt->MakeLabel(EvtBase::LABEL_TOOL)));
	item->SetLongHelp(str2wx(pevt->m_sHelp));

	if (pevt->flags.get(EvtBase::FLAG_CHECK))
	{
		tbar->ToggleTool(pevt->m_nId, pevt->flags.get(EvtBase::FLAG_CHECKED));
	}

	tbar->EnableTool(pevt->m_nId, !pevt->flags.get(EvtBase::FLAG_DISABLE | EvtBase::FLAG_HIDE_UI));
	tbar->RefreshRect(tbar->GetToolRect(pevt->m_nId));
	
}

void ICtl_aui_toolbar::ICtl_wxctrl_itemdata::UpdateBmps()
{

	const BitmapBundle& bundle(pevt->GetBundle(24,1));
	if(!bundle.IsOk())
	{
		return;
	}

	item->SetBitmap(bundle.bmp_normal);
	item->SetDisabledBitmap(bundle.bmp_disabled);

}


bool ICtl_aui_toolbar::StdExecute(IStdParam& cmd)
{
	if(cmd.extra1=="clear")
	{
		m_aItems.clear();
		ClearTools();
	}
	else if(cmd.extra1=="update")
	{
		this->Enable(!m_pGroup->flags.get(EvtBase::FLAG_DISABLE));
	}
	return true;
}

void ICtl_aui_toolbar::InitToolItem(EvtCommand* pevt, IAuiToolItemPtr item)
{
	ICtl_wxctrl_itemdata* pctrl = new ICtl_wxctrl_itemdata(pevt, item);
	pctrl->tbar=this;
	m_aItems.append(pctrl);

	pctrl->UpdateBmps();
	pctrl->UpdateCtrl();

}


static ICtl_object* WndCreateToolBar(const ICtlParam& param,EvtGroup* pevt)
{
	return new ICtl_toolbar(pevt,param.parent,param.bmpsize);
}

static ICtl_object* WndCreateAuiToolBar(const ICtlParam& param,EvtGroup* pevt)
{
	return new ICtl_aui_toolbar(pevt,param.parent,param.bmpsize);
}


bool PluginToolBar::OnAttach()
{

	EvtManager& ec(wm.evtmgr);

	ICtl_object::WndRegister("toolbar",&WndCreateToolBar);
	ICtl_object::WndRegister("aui_toolbar",&WndCreateAuiToolBar);

	return true;

}

IMPLEMENT_IPLUGIN(PluginToolBar)

EW_LEAVE

