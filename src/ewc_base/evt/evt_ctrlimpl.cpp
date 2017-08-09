#include "evt_ctrlimpl.h"
#include "ewc_base/evt/evt_group.h"
#include "ewc_base/app/res_manager.h"

EW_ENTER



IEW_MenuImpl::IEW_MenuImpl(EvtGroup* mu)
:m_pGroup(mu)
{
	if (m_pGroup) m_pGroup->m_aSubMenus.insert(this);
}

IEW_MenuImpl::~IEW_MenuImpl()
{
	if (m_pGroup) m_pGroup->m_aSubMenus.erase(this);
}

IMenuItemPtr IEW_MenuImpl::AddMenuItem(EvtCommand* pevt, wxMenu* menu)
{
	IMenuItemPtr item = new wxMenuItem(this, pevt->m_nId, str2wx(pevt->MakeLabel(EvtBase::LABEL_MENU)));
	item->SetSubMenu(menu);
	return InitMenuItem(pevt, item);
}

IMenuItemPtr IEW_MenuImpl::AddMenuItem(EvtCommand* pevt)
{
	IMenuItemPtr item = new wxMenuItem(this, pevt->m_nId, str2wx(pevt->MakeLabel(EvtBase::LABEL_MENU)));
	if (pevt->flags.get(EvtCommand::FLAG_CHECK))
	{
		item->SetCheckable(true);
	}
	return InitMenuItem(pevt, item);
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


IMenuItemPtr IEW_MenuImpl::InitMenuItem(EvtCommand* pevt, IMenuItemPtr item)
{

	pevt->m_bmpParam.update(item);
	Append(item);

	IEW_WxCtrlData_menu* ctrl = new IEW_WxCtrlData_menu(pevt, item);
	item->SetRefData(ctrl);
	ctrl->UpdateCtrl();

	return item;
}



IEW_TBarImpl::IEW_TBarImpl(EvtGroup* mu, wxWindow* pw, int wd)
:wxToolBar()
, m_pGroup(mu)
{
	if (wd<0) wd = WndManager::current().conf.bmp_tool_size;
	m_pGroup->m_aSubTbars.insert(this);

	this->Create(pw, mu->m_nId, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
	this->SetName(str2wx(m_pGroup->m_sId));
	this->SetToolBitmapSize(wxSize(wd, wd));
}

IEW_TBarImpl::~IEW_TBarImpl()
{
	m_pGroup->m_aSubTbars.erase(this);
}

IToolItemPtr IEW_TBarImpl::AddToolItem(EvtCommand* pevt, wxControl* p)
{
	IToolItemPtr item = wxToolBar::AddControl(p);
	return InitToolItem(pevt, item);
}

IToolItemPtr IEW_TBarImpl::AddToolItem(EvtCommand* pevt)
{
	IToolItemPtr item = wxToolBar::AddTool(pevt->m_nId, str2wx(pevt->MakeLabel()), wxNullBitmap, wxString(), pevt->flags.get(EvtCommand::FLAG_CHECK) ? wxITEM_CHECK : wxITEM_NORMAL);
	return InitToolItem(pevt, item);
}

IToolItemPtr IEW_TBarImpl::AddToolItem(EvtCommand* pevt, wxMenu* menu)
{
	IToolItemPtr item = wxToolBar::AddTool(pevt->m_nId, str2wx(pevt->MakeLabel()), wxNullBitmap, wxString(), wxITEM_DROPDOWN);
	item->SetDropdownMenu(menu);
	return InitToolItem(pevt, item);
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



IToolItemPtr IEW_TBarImpl::InitToolItem(EvtCommand* pevt, IToolItemPtr item)
{
	IEW_WxCtrlData_tool* pctrl = new IEW_WxCtrlData_tool(pevt, item);
	item->SetRefData(pctrl);
	pevt->m_bmpParam.update(item);
	pctrl->UpdateCtrl();
	return item;
}


EW_LEAVE
