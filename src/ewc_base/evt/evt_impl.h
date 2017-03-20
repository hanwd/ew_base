#include "ewc_base/evt/evt_command.h"
#include "ewc_base/wnd/impl_wx/iwnd_textctrl.h"
#include "ewc_base/wnd/impl_wx/iwnd_bookbase.h"

EW_ENTER

class HeMenuImpl : public wxMenu
{
public:
	DataPtrT<EvtGroup> m_pMenu;
	WndManager& wm;

	HeMenuImpl(EvtGroup* mu)
		:wm(WndManager::current())
		,m_pMenu(mu)
	{
		if(m_pMenu) m_pMenu->m_aSubMenus.insert(this);
	}

	HeMenuImpl()
		:wm(WndManager::current())
	{

	}

	~HeMenuImpl()
	{
		if(m_pMenu) m_pMenu->m_aSubMenus.erase(this);
	}


};


class HeTbarImpl : public wxToolBar
{
public:
	DataPtrT<EvtGroup> m_pGroup;
	WndManager& wm;

	HeTbarImpl(EvtGroup* mu,wxWindow* pw,int wd)
		:wxToolBar()
		,m_pGroup(mu)
		,wm(WndManager::current())
	{
		if(wd<0) wd=WndManager::current().conf.bmp_tool_size;
		m_pGroup->m_aSubTbars.insert(this);

		this->Create(pw,mu->m_nId,wxDefaultPosition,wxDefaultSize,wxTB_FLAT|wxTB_NODIVIDER);
		this->SetName(str2wx(m_pGroup->m_sId));
		this->SetToolBitmapSize(wxSize(wd,wd));
	}

	~HeTbarImpl()
	{
		m_pGroup->m_aSubTbars.erase(this);
	}
};

class HeMenuItemImpl : public wxMenuItem
{
public:
	HeMenuItemImpl(HeMenuImpl* mu,EvtCommand* it):wxMenuItem(mu,it->m_nId,str2wx(it->MakeLabel()))
	{
		if(it->flags.get(EvtCommand::FLAG_CHECK))
		{
			this->SetCheckable(true);
		}
		m_pCtrlImpl.reset(it);
		m_pCtrlImpl->m_setMenuImpls.insert(this);
	}

	~HeMenuItemImpl()
	{
		if(m_pCtrlImpl) m_pCtrlImpl->m_setMenuImpls.erase(this);
	}

	HeMenuImpl* GetMenu() const
	{
		return (HeMenuImpl*)wxMenuItem::GetMenu();
	}
	
	DataPtrT<EvtCommand> m_pCtrlImpl;

};

class HeToolItemImpl : public wxToolBarToolBase
{
public:

	HeToolItemImpl(HeTbarImpl* tb,EvtCommand* it,wxControl* p)
		:wxToolBarToolBase(tb,p,str2wx(it->MakeLabel()))
	{
		_init(it);
		m_kind=wxITEM_MAX;
	}

	HeToolItemImpl(wxToolBar* tb,EvtCommand* it)
		:wxToolBarToolBase(tb,it->m_nId,str2wx(it->MakeLabel()))
	{

		this->SetToggle(it->flags.get(EvtCommand::FLAG_CHECK));
		_init(it);
	}

	HeToolItemImpl(wxToolBar* tb,EvtGroup* it)
		:wxToolBarToolBase(tb,it->m_nId,str2wx(it->MakeLabel()))
	{

		this->SetToggle(it->flags.get(EvtCommand::FLAG_CHECK));
		_init(it);
		m_kind=wxITEM_DROPDOWN;
	}

	void _init(EvtCommand* it)
	{
		m_pCtrlImpl.reset(it);
		m_pCtrlImpl->m_setToolImpls.insert(this);
		padding[0]=0;
		padding[1]=0;
	}

	~HeToolItemImpl()
	{
		m_pCtrlImpl->m_setToolImpls.erase(this);
	}

	HeTbarImpl* GetToolBar() const
	{
		return  (HeTbarImpl*)wxToolBarToolBase::GetToolBar();
	}

	// wxToolBarTool padding
	size_t padding[2];

	DataPtrT<EvtCommand> m_pCtrlImpl; 

};

EW_LEAVE
