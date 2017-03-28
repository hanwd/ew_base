#include "ewc_base/plugin/plugin_menu.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/evt/evt_base.h"
#include "ewc_base/evt/evt_cmdproc.h"
#include "ewc_base/evt/evt_command.h"
#include "ewc_base/evt/evt_group.h"
#include "ewc_base/app/res_manager.h"

#include "ewc_base/wnd/impl_wx/window.h"

EW_ENTER


class EvtCommandWindowMenuBar : public EvtCommandWindow
{
public:

	EvtCommandWindowMenuBar(const String& n=_kT("MenuBar")):EvtCommandWindow(n)
	{
		wxMenuBar* mb=new wxMenuBar;
		mb->SetName(str2wx(m_sId));
		SetWindow(mb);
	}

	~EvtCommandWindowMenuBar()
	{

	}
};


PluginMenu::PluginMenu(WndManager& w):PluginCommon(w,"Plugin.Menu")
{

}



class IEW_MenuImpl : public wxMenu, public IEW_Ctrl
{
public:

	IEW_MenuImpl(EvtGroup* mu = NULL);
	~IEW_MenuImpl();

	bool AddCtrlItem(EvtGroup* pevt);
	bool AddCtrlItem(EvtCommand* pevt);

	bool StdExecute(IStdParam& cmd);

	virtual wxMenu* GetMenu(){ return this; }

protected:

	void WndUpdateCtrl();

	class IEW_WxCtrlData_menu : public IEW_CtrlData
	{
	public:
		IEW_WxCtrlData_menu(EvtCommand* pevt_, IMenuItemPtr item_) :IEW_CtrlData(pevt_), item(item_){ }
		IMenuItemPtr item;

		void UpdateCtrl();
		void UpdateBmps();
	};

protected:
	void InitMenuItem(EvtCommand* pevt, IMenuItemPtr item);
};




IEW_MenuImpl::IEW_MenuImpl(EvtGroup* pevt)
:IEW_Ctrl(pevt)
{
	WndUpdateCtrl();
}

IEW_MenuImpl::~IEW_MenuImpl()
{

}

void IEW_MenuImpl::WndUpdateCtrl()
{
	if(!m_pGroup)
	{
		return;
	}

	m_aItems.clear();
		
	int nc = this->GetMenuItemCount();
	while (--nc >= 0)
	{
		wxMenuItem* mi = this->FindItemByPosition(0);
		this->Destroy(mi);
	}

	int last_is_seperator=-1;
	for(size_t i=0;i<m_pGroup->size();i++)
	{
		EvtCommand* pCommand=(*m_pGroup)[i].get();
		if(pCommand->flags.get(EvtBase::FLAG_HIDE_UI)) continue;
		if(pCommand->flags.get(EvtCommand::FLAG_SEPARATOR))
		{
			if(last_is_seperator==0) last_is_seperator=1;
		}
		else
		{
			if(last_is_seperator==1) this->AppendSeparator();
			last_is_seperator=0;
			pCommand->CreateCtrlItem(this);
		}
	}

	if(last_is_seperator==-1)
	{
		EvtCommand* pCommand=EvtManager::current()["Empty"].cast_command();
		if(pCommand)
		{
			this->Append(wxID_ANY,str2wx(pCommand->MakeLabel()))->Enable(false);
		}
	}
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
		WndUpdateCtrl();
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


static IEW_Ctrl* WndCreateMenu(const ICtlParam& param,EvtGroup* pevt)
{
	return new IEW_MenuImpl(pevt);
}



bool PluginMenu::OnAttach()
{

	EvtManager& ec(wm.evtmgr);

	IEW_Ctrl::WndRegister("menu",&WndCreateMenu);

	ec.append(new EvtCommandWindowMenuBar);

	ec.gp_beg("Menu.Extra");

	ec.gp_end();

	ec.gp_beg("MenuBar.default");
		ec.gp_add("File");
		ec.gp_add("Edit");
		ec.gp_add("View");
		ec.gp_add("Menu.Extra",1);
		ec.gp_add("Help");
	ec.gp_end();

	return true;
}

IMPLEMENT_IPLUGIN(PluginMenu)

EW_LEAVE

