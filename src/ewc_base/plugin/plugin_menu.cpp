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



class ICtl_menu : public wxMenu, public ICtl_object
{
public:

	ICtl_menu(EvtGroup* mu = NULL);
	~ICtl_menu();

	bool AddCtrlItem(EvtGroup* pevt);
	bool AddCtrlItem(EvtCommand* pevt);

	bool StdExecute(IStdParam& cmd);

	virtual wxMenu* GetMenu(){ return this; }

	void ClearMenus()
	{
		while (this->GetMenuItemCount() > 0 && this->Destroy(this->FindItemByPosition(0)));
	}

protected:


	void WndUpdateCtrl();

	class IEW_WxCtrlData_menu : public ICtl_itemdata
	{
	public:
		IEW_WxCtrlData_menu(EvtCommand* pevt_, IMenuItemPtr item_) :ICtl_itemdata(pevt_), item(item_){ }
		IMenuItemPtr item;

		int UpdateCtrl();
		void UpdateBmps();
	};

protected:
	void InitMenuItem(EvtCommand* pevt, IMenuItemPtr item);
};




ICtl_menu::ICtl_menu(EvtGroup* pevt)
:ICtl_object(pevt)
{
	WndUpdateCtrl();
}

ICtl_menu::~ICtl_menu()
{

}

void ICtl_menu::WndUpdateCtrl()
{
	if(!m_pGroup)
	{
		return;
	}

	m_aItems.clear();
		
	ClearMenus();


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

bool ICtl_menu::StdExecute(IStdParam& cmd)
{
	if (cmd.extra1 == "clear")
	{
		m_aItems.clear();
		ClearMenus();
	}
	else if (cmd.extra1 == "update")
	{
		WndUpdateCtrl();
	}
	return true;
}


bool ICtl_menu::AddCtrlItem(EvtGroup* pevt)
{

	IMenuItemPtr item = new wxMenuItem(this, pevt->m_nId, str2wx(pevt->MakeLabel(EvtBase::LABEL_MENU)));
	item->SetSubMenu(pevt->CreateMenu());

	this->Append(item);
	InitMenuItem(pevt, item);

	return true;
}

bool ICtl_menu::AddCtrlItem(EvtCommand* pevt)
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


void ICtl_menu::IEW_WxCtrlData_menu::UpdateBmps()
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

int ICtl_menu::IEW_WxCtrlData_menu::UpdateCtrl()
{
	ICtl_menu* mu = (ICtl_menu*)item->GetMenu();
	if (!mu) return 0;

	if (pevt->flags.get(EvtBase::FLAG_HIDE_UI))
	{
		mu->Delete(item);
		return -1;
	}

	item->SetHelp(str2wx(pevt->m_sHelp));
	item->SetItemLabel(str2wx(pevt->MakeLabel(EvtBase::LABEL_MENU)));
	item->SetCheckable(pevt->flags.get(EvtBase::FLAG_CHECK));
	item->Enable(!pevt->flags.get(EvtBase::FLAG_DISABLE));

	if (pevt->flags.get(EvtBase::FLAG_CHECK))
	{
		item->Check(pevt->flags.get(EvtBase::FLAG_CHECKED));
	}

	return 0;
}


void ICtl_menu::InitMenuItem(EvtCommand* pevt, IMenuItemPtr item)
{
	IEW_WxCtrlData_menu* ctrl = new IEW_WxCtrlData_menu(pevt, item);
	m_aItems.append(ctrl);

	ctrl->UpdateBmps();	
	//Append(item);
	ctrl->UpdateCtrl();
}


static ICtl_object* WndCreateMenu(const ICtlParam& param,EvtGroup* pevt)
{
	return new ICtl_menu(pevt);
}



bool PluginMenu::OnAttach()
{

	EvtManager& ec(wm.evtmgr);

	ICtl_object::WndRegister("menu",&WndCreateMenu);

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

