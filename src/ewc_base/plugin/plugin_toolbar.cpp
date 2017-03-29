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

class IEW_TBarImpl : public wxToolBar, public IEW_Ctrl
{
public:

	IEW_TBarImpl(EvtGroup* mu, wxWindow* pw, int wd);
	~IEW_TBarImpl();

	bool AddCtrlItem(EvtCommand* pevt);
	bool AddCtrlItem(EvtCommand* pevt, wxControl* p);
	bool AddCtrlItem(EvtGroup* pevt);

	virtual wxWindow* GetWindow(){return this;}

	bool StdExecute(IStdParam& cmd);


protected:

	class IEW_WxCtrlData_tool : public IEW_CtrlData
	{
	public:
		IEW_WxCtrlData_tool(EvtCommand* pevt_, IToolItemPtr item_) :IEW_CtrlData(pevt_), item(item_){}
		IToolItemPtr item;

		void UpdateCtrl();
		void UpdateBmps();
	};

protected:
	void InitToolItem(EvtCommand* pevt, IToolItemPtr item);

};


class IEW_AuiTBarImpl : public wxAuiToolBar, public IEW_Ctrl
{
public:

	IEW_AuiTBarImpl(EvtGroup* mu, wxWindow* pw, int wd);
	~IEW_AuiTBarImpl();

	bool AddCtrlItem(EvtCommand* pevt);
	bool AddCtrlItem(EvtCommand* pevt, wxControl* p);
	bool AddCtrlItem(EvtGroup* pevt);

	virtual wxWindow* GetWindow(){return this;}

	bool StdExecute(IStdParam& cmd);

protected:

	class IEW_WxCtrlData_tool : public IEW_CtrlData
	{
	public:
		IEW_WxCtrlData_tool(EvtCommand* pevt_, IAuiToolItemPtr item_) :IEW_CtrlData(pevt_), item(item_){}
		IAuiToolItemPtr item;
		IEW_AuiTBarImpl* tbar;
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
		AppData::current().popup_dropdown_menu.bind(&IEW_AuiTBarImpl::EvtPopupMenu, _1, _2, evt);

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



IEW_TBarImpl::IEW_TBarImpl(EvtGroup* pevt, wxWindow* pw, int wd)
:wxToolBar()
, IEW_Ctrl(pevt)
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

IEW_TBarImpl::~IEW_TBarImpl()
{

}

bool IEW_TBarImpl::StdExecute(IStdParam& cmd)
{
	if(cmd.extra1=="clear")
	{
		m_aItems.clear();
		int nc=this->GetToolsCount();
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

bool IEW_TBarImpl::AddCtrlItem(EvtCommand* pevt, wxControl* p)
{
	pevt->CreateValidator(p);
	IToolItemPtr item = wxToolBar::AddControl(p);
	InitToolItem(pevt, item);
	return true;
}

bool IEW_TBarImpl::AddCtrlItem(EvtCommand* pevt)
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

bool IEW_TBarImpl::AddCtrlItem(EvtGroup* pevt)
{
	IToolItemPtr item = wxToolBar::AddTool(pevt->m_nId, str2wx(pevt->MakeLabel()), wxNullBitmap, wxString(), wxITEM_DROPDOWN);
	item->SetDropdownMenu(pevt->CreateMenu());
	InitToolItem(pevt, item);
	return true;
}


void IEW_TBarImpl::IEW_WxCtrlData_tool::UpdateCtrl()
{
	if (pevt->m_nId<0)
	{
		return;
	}

	IEW_TBarImpl* tb = (IEW_TBarImpl*)item->GetToolBar();
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


void IEW_TBarImpl::IEW_WxCtrlData_tool::UpdateBmps()
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

void IEW_TBarImpl::InitToolItem(EvtCommand* pevt, IToolItemPtr item)
{
	IEW_WxCtrlData_tool* pctrl = new IEW_WxCtrlData_tool(pevt, item);
	m_aItems.append(pctrl);
	pctrl->UpdateBmps();
	pctrl->UpdateCtrl();
}





IEW_AuiTBarImpl::IEW_AuiTBarImpl(EvtGroup* pevt, wxWindow* pw, int wd)
: IEW_Ctrl(pevt)
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


	this->Connect(wxID_ANY, wxEVT_AUITOOLBAR_TOOL_DROPDOWN, wxAuiToolBarEventHandler(IEW_AuiTBarImpl::OnDropDown));

}

IEW_AuiTBarImpl::~IEW_AuiTBarImpl()
{

}

bool IEW_AuiTBarImpl::AddCtrlItem(EvtCommand* pevt, wxControl* p)
{
	pevt->CreateValidator(p);
	IAuiToolItemPtr item = AddControl(p);
	InitToolItem(pevt, item);
	return true;
}

bool IEW_AuiTBarImpl::AddCtrlItem(EvtCommand* pevt)
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

bool IEW_AuiTBarImpl::AddCtrlItem(EvtGroup* pevt)
{
	IAuiToolItemPtr item = AddTool(pevt->m_nId, str2wx(pevt->MakeLabel()), wxNullBitmap);
	item->SetHasDropDown(true);
	InitToolItem(pevt, item);
	return true;
}


void IEW_AuiTBarImpl::IEW_WxCtrlData_tool::UpdateCtrl()
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

void IEW_AuiTBarImpl::IEW_WxCtrlData_tool::UpdateBmps()
{

	const BitmapBundle& bundle(pevt->GetBundle(24,1));
	if(!bundle.IsOk())
	{
		return;
	}

	item->SetBitmap(bundle.bmp_normal);
	item->SetDisabledBitmap(bundle.bmp_disabled);

}


bool IEW_AuiTBarImpl::StdExecute(IStdParam& cmd)
{
	if(cmd.extra1=="clear")
	{
		m_aItems.clear();
		int nc=this->GetToolCount();
		while(--nc>=0)
		{
			this->ClearTools();
		}		
	}
	else if(cmd.extra1=="update")
	{
		this->Enable(!m_pGroup->flags.get(EvtBase::FLAG_DISABLE));
	}
	return true;
}

void IEW_AuiTBarImpl::InitToolItem(EvtCommand* pevt, IAuiToolItemPtr item)
{
	IEW_WxCtrlData_tool* pctrl = new IEW_WxCtrlData_tool(pevt, item);
	pctrl->tbar=this;
	m_aItems.append(pctrl);

	pctrl->UpdateBmps();
	pctrl->UpdateCtrl();

}


static IEW_Ctrl* WndCreateToolBar(const ICtlParam& param,EvtGroup* pevt)
{
	return new IEW_TBarImpl(pevt,param.parent,param.bmpsize);
}

static IEW_Ctrl* WndCreateAuiToolBar(const ICtlParam& param,EvtGroup* pevt)
{
	return new IEW_AuiTBarImpl(pevt,param.parent,param.bmpsize);
}


bool PluginToolBar::OnAttach()
{

	EvtManager& ec(wm.evtmgr);

	IEW_Ctrl::WndRegister("toolbar",&WndCreateToolBar);
	IEW_Ctrl::WndRegister("aui_bar",&WndCreateAuiToolBar);

	return true;

}

IMPLEMENT_IPLUGIN(PluginToolBar)

EW_LEAVE

