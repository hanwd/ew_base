#include "ewc_base/evt/evt_command.h"
#include "ewc_base/app/res_manager.h"
#include "ewc_base/wnd/impl_wx/iwnd_textctrl.h"
#include "ewc_base/wnd/impl_wx/iwnd_bookbase.h"

EW_ENTER

class IEW_WxCtrlData : public IEW_CtrlData, public wxObjectRefData
{
public:
	IEW_WxCtrlData(EvtCommand* p) :IEW_CtrlData(p){}
};

class IEW_MenuImpl : public wxMenu
{
public:

	IEW_MenuImpl(EvtGroup* mu = NULL);
	~IEW_MenuImpl();
	IMenuItemPtr AddMenuItem(EvtCommand* pevt, wxMenu* menu);
	IMenuItemPtr AddMenuItem(EvtCommand* pevt);

protected:

	class IEW_WxCtrlData_menu : public IEW_WxCtrlData
	{
	public:
		IEW_WxCtrlData_menu(EvtCommand* pevt_, IMenuItemPtr item_) :IEW_WxCtrlData(pevt_), item(item_){ }
		IMenuItemPtr item;

		void UpdateCtrl();
	};

	IMenuItemPtr InitMenuItem(EvtCommand* pevt, IMenuItemPtr item);

	DataPtrT<EvtGroup> m_pGroup;

};


class IEW_TBarImpl : public wxToolBar
{
public:

	IEW_TBarImpl(EvtGroup* mu, wxWindow* pw, int wd);
	~IEW_TBarImpl();

	IToolItemPtr AddToolItem(EvtCommand* pevt, wxControl* p);
	IToolItemPtr AddToolItem(EvtCommand* pevt);
	IToolItemPtr AddToolItem(EvtCommand* pevt, wxMenu* menu);

protected:

	class IEW_WxCtrlData_tool : public IEW_WxCtrlData
	{
	public:
		IEW_WxCtrlData_tool(EvtCommand* pevt_, IToolItemPtr item_) :IEW_WxCtrlData(pevt_), item(item_){}
		IToolItemPtr item;

		void UpdateCtrl();

	};

	IToolItemPtr InitToolItem(EvtCommand* pevt, IToolItemPtr item);

	DataPtrT<EvtGroup> m_pGroup;
};

EW_LEAVE
