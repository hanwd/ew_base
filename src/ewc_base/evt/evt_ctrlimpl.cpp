#include "evt_ctrlimpl.h"
#include "ewc_base/evt/evt_group.h"
#include "ewc_base/app/res_manager.h"
#include "ewc_base/wnd/wnd_manager.h"
#include "ewc_base/evt/evt_manager.h"

EW_ENTER



IEW_MenuImpl::IEW_MenuImpl(EvtGroup* mu)
:IEW_Ctrl(mu)
{

}

IEW_MenuImpl::~IEW_MenuImpl()
{

}


bool IEW_MenuImpl::StdExecute(IStdParam& cmd)
{
	if (cmd.extra1 == "clear")
	{
		m_aItems.clear();
		
		
		int nc = this->GetMenuItemCount();
		while (--nc >= 0)
		{
			wxMenuItem* mi = this->FindItemByPosition(0);
			this->Destroy(mi);
		}
	}
	else if (cmd.extra1 == "update")
	{
		m_pGroup->CreateMenu(this, false);
	}
	return true;
}


bool IEW_MenuImpl::AddCtrlItem(EvtGroup* pevt)
{

	IMenuItemPtr item = new wxMenuItem(this, pevt->m_nId, str2wx(pevt->MakeLabel(EvtBase::LABEL_MENU)));
	item->SetSubMenu(pevt->CreateMenu());

	this->Append(item);
	InitMenuItem(pevt, item);

	return true;
}

bool IEW_MenuImpl::AddCtrlItem(EvtCommand* pevt)
{
	if(pevt->flags.get(EvtCommand::FLAG_SEPARATOR))
	{
		this->AppendSeparator();
		return true;
	}

	IMenuItemPtr item = NULL;

	if (pevt->flags.get(EvtCommand::FLAG_CHECK))
	{
		item = this->AppendCheckItem(pevt->m_nId, str2wx(pevt->MakeLabel(EvtBase::LABEL_MENU)));

		if (pevt->flags.get(EvtBase::FLAG_RADIO))
		{
			static const BitmapBundle bundle(ResManager::current().icons.get("IconRadio").GetBundle(16, 0));
			if (bundle.IsOk())
			{
				item->SetBitmap(bundle.bmp_normal);
			}
		}
		else
		{
			static const BitmapBundle bundle(ResManager::current().icons.get("IconCheck").GetBundle(16, 0));
			if (bundle.IsOk())
			{
				item->SetBitmap(bundle.bmp_normal);
			}
		}
	}
	else
	{
		item = this->Append(pevt->m_nId, str2wx(pevt->MakeLabel(EvtBase::LABEL_MENU)));
	}

	InitMenuItem(pevt, item);
	return true;
}


void IEW_MenuImpl::IEW_WxCtrlData_menu::UpdateBmps()
{
	if (item->IsCheckable())
	{
		return;
	}

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
	//Append(item);
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
		tbar->RefreshRect(tbar->GetToolRect(pevt->m_nId));
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
