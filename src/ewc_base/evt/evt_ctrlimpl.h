#include "ewc_base/evt/evt_command.h"
#include "ewc_base/app/res_manager.h"

#include "ewc_base/wnd/impl_wx/iwnd_textctrl.h"
#include "ewc_base/wnd/impl_wx/iwnd_bookbase.h"
#include "wx/aui/auibar.h"

EW_ENTER


class IEW_MenuImpl : public wxMenu, public IEW_Ctrl
{
public:

	IEW_MenuImpl(EvtGroup* mu = NULL);
	~IEW_MenuImpl();

	void AddCtrlItem(EvtCommand* pevt, wxMenu* menu);
	void AddCtrlItem(EvtCommand* pevt);

	bool StdExecute(IStdParam& cmd)
	{
		if(cmd.extra1=="clear")
		{
			m_aItems.clear();
			int nc=this->GetMenuItemCount();
			while(--nc>=0)
			{
				wxMenuItem* mi=this->FindItemByPosition(0);
				this->Destroy(mi);
			}		
		}
		else if(cmd.extra1=="update")
		{
			m_pGroup->CreateMenu(this,false);
		}
		return true;
	}


protected:

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


class IEW_TBarImpl : public wxToolBar, public IEW_Ctrl
{
public:

	IEW_TBarImpl(EvtGroup* mu, wxWindow* pw, int wd);
	~IEW_TBarImpl();

	void AddCtrlItem(EvtCommand* pevt);
	void AddCtrlItem(EvtCommand* pevt, wxControl* p);
	void AddCtrlItem(EvtCommand* pevt, wxMenu* menu);

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

	void AddCtrlItem(EvtCommand* pevt);
	void AddCtrlItem(EvtCommand* pevt, wxControl* p);
	void AddCtrlItem(EvtCommand* pevt, wxMenu* menu);

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

};

EW_LEAVE
