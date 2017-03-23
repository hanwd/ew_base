#include "evt_ctrlimpl.h"
#include "ewc_base/evt/evt_group.h"
#include "ewc_base/app/res_manager.h"

EW_ENTER



IEW_MenuImpl::IEW_MenuImpl(EvtGroup* mu)
:IEW_Ctrl(mu)
{

}

IEW_MenuImpl::~IEW_MenuImpl()
{

}

void IEW_MenuImpl::AddCtrlItem(EvtCommand* pevt, wxMenu* menu)
{
	IMenuItemPtr item = new wxMenuItem(this, pevt->m_nId, str2wx(pevt->MakeLabel(EvtBase::LABEL_MENU)));
	item->SetSubMenu(menu);
	InitMenuItem(pevt, item);
}

void IEW_MenuImpl::AddCtrlItem(EvtCommand* pevt)
{
	if(pevt->flags.get(EvtCommand::FLAG_SEPARATOR))
	{
		this->AppendSeparator();
		return;
	}

	IMenuItemPtr item = new wxMenuItem(this, pevt->m_nId, str2wx(pevt->MakeLabel(EvtBase::LABEL_MENU)));
	if (pevt->flags.get(EvtCommand::FLAG_CHECK))
	{
		item->SetCheckable(true);
	}

	InitMenuItem(pevt, item);

}


void IEW_MenuImpl::IEW_WxCtrlData_menu::UpdateBmps()
{
	if(item->IsCheckable()) return;

	const BitmapBundle& bundle(pevt->GetBundle(16,0));
	if(!bundle.IsOk())
	{
		return;
	}

	item->SetBitmap(bundle.bmp_normal);
	item->SetDisabledBitmap(bundle.bmp_disabled);
}

void IEW_MenuImpl::IEW_WxCtrlData_menu::UpdateCtrl()
{
	IEW_MenuImpl* mu = (IEW_MenuImpl*)item->GetMenu();
	if (!mu) return;

	if (pevt->flags.get(EvtBase::FLAG_HIDE_UI))
	{
		mu->Delete(item);
		return;
	}

	item->SetHelp(str2wx(pevt->m_sHelp));
	item->SetItemLabel(str2wx(pevt->MakeLabel(EvtBase::LABEL_MENU)));
	item->SetCheckable(pevt->flags.get(EvtBase::FLAG_CHECK));
	item->Enable(!pevt->flags.get(EvtBase::FLAG_DISABLE));

	if (pevt->flags.get(EvtBase::FLAG_CHECK))
	{
		item->Check(pevt->flags.get(EvtBase::FLAG_CHECKED));
	}
}


void IEW_MenuImpl::InitMenuItem(EvtCommand* pevt, IMenuItemPtr item)
{
	IEW_WxCtrlData_menu* ctrl = new IEW_WxCtrlData_menu(pevt, item);
	m_aItems.append(ctrl);

	ctrl->UpdateBmps();	
	Append(item);
	ctrl->UpdateCtrl();
}



IEW_TBarImpl::IEW_TBarImpl(EvtGroup* mu, wxWindow* pw, int wd)
:wxToolBar()
, IEW_Ctrl(mu)
{
	wd=wd<0?WndManager::current().data.toolbitmap_size:wd;
	this->Create(pw, mu->m_nId, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
	this->SetName(str2wx(m_pGroup->m_sId));
	this->SetToolBitmapSize(wxSize(wd, wd));
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

void IEW_TBarImpl::AddCtrlItem(EvtCommand* pevt, wxControl* p)
{
	pevt->CreateValidator(p);
	IToolItemPtr item = wxToolBar::AddControl(p);
	InitToolItem(pevt, item);
}

void IEW_TBarImpl::AddCtrlItem(EvtCommand* pevt)
{
	IToolItemPtr item = wxToolBar::AddTool(pevt->m_nId, str2wx(pevt->MakeLabel()), wxNullBitmap, wxString(), pevt->flags.get(EvtCommand::FLAG_CHECK) ? wxITEM_CHECK : wxITEM_NORMAL);
	InitToolItem(pevt, item);
}

void IEW_TBarImpl::AddCtrlItem(EvtCommand* pevt, wxMenu* menu)
{
	IToolItemPtr item = wxToolBar::AddTool(pevt->m_nId, str2wx(pevt->MakeLabel()), wxNullBitmap, wxString(), wxITEM_DROPDOWN);
	item->SetDropdownMenu(menu);
	InitToolItem(pevt, item);
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





IEW_AuiTBarImpl::IEW_AuiTBarImpl(EvtGroup* mu, wxWindow* pw, int wd)
: IEW_Ctrl(mu)
{
	wd=wd<0?WndManager::current().data.toolbitmap_size:wd;
	this->Create(pw, mu->m_nId, wxDefaultPosition, wxDefaultSize);

	this->SetName(str2wx(m_pGroup->m_sId));
	this->SetToolBitmapSize(wxSize(wd, wd));


}

IEW_AuiTBarImpl::~IEW_AuiTBarImpl()
{

}

void IEW_AuiTBarImpl::AddCtrlItem(EvtCommand* pevt, wxControl* p)
{
	pevt->CreateValidator(p);
	IAuiToolItemPtr item = AddControl(p);
	InitToolItem(pevt, item);
}

void IEW_AuiTBarImpl::AddCtrlItem(EvtCommand* pevt)
{
	IAuiToolItemPtr item = AddTool(pevt->m_nId, str2wx(pevt->MakeLabel()), wxNullBitmap, wxString(), pevt->flags.get(EvtCommand::FLAG_CHECK) ? wxITEM_CHECK : wxITEM_NORMAL);
	InitToolItem(pevt, item);
}

void IEW_AuiTBarImpl::AddCtrlItem(EvtCommand* pevt, wxMenu* menu)
{
	delete menu;
	IAuiToolItemPtr item = AddTool(pevt->m_nId, str2wx(pevt->MakeLabel()), wxNullBitmap);
	item->SetHasDropDown(true);
	InitToolItem(pevt, item);
}


void IEW_AuiTBarImpl::IEW_WxCtrlData_tool::UpdateCtrl()
{
	if (pevt->m_nId<0)
	{
		return;
	}

	//IEW_AuiTBarImpl* tb = (IEW_AuiTBarImpl*)item->GetToolBar();
	//if (!tb) return;

	item->SetShortHelp(str2wx(pevt->MakeLabel()));
	item->SetLabel(str2wx(pevt->MakeLabel(EvtBase::LABEL_TOOL)));
	item->SetLongHelp(str2wx(pevt->m_sHelp));

	if (pevt->flags.get(EvtBase::FLAG_CHECK))
	{
		item->SetKind(wxITEM_CHECK);
		//item->SetToggle(true);
		tbar->ToggleTool(pevt->m_nId, pevt->flags.get(EvtBase::FLAG_CHECKED));
	}
	tbar->EnableTool(pevt->m_nId, !pevt->flags.get(EvtBase::FLAG_DISABLE | EvtBase::FLAG_HIDE_UI));
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

EW_LEAVE
