#ifndef __H_EW_UI_EVT_GROUP__
#define __H_EW_UI_EVT_GROUP__

#include "ewc_base/evt/evt_command.h"
#include "ewc_base/app/data_defs.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE EvtGroup  : public ObjectGroupT<EvtCommand,EvtCommandWindow>
{
public:
	typedef ObjectGroupT<EvtCommand,EvtCommandWindow> basetype;

	friend class HeTbarImpl;
	friend class HeMenuImpl;

	EvtGroup(const String& name="");

	virtual void DoUpdateCtrl(IUpdParam& upd);
	virtual void PrepareItems();

	virtual HeToolItemImpl* CreateToolItem(HeTbarImpl* tb);
	virtual HeMenuItemImpl* CreateMenuItem(HeMenuImpl* mu);

	IMenuPtr CreateMenu(HeMenuImpl* mu=NULL,bool prepare=true);
	ITbarPtr CreateTbar(IWindowPtr pw,int wd=-1);

	static void ClearMenu(wxMenu* mu);

	virtual EvtGroup* cast_group(){return this;}

	void add(const String& s,const String& e,int f=0);
	void add(const String& s,int f=0);
	void get(arr_1t<EvtItem>& a);
	void set(const arr_1t<EvtItem>& a);

	const arr_1t<EvtItem>& items(){return m_aItems;}

	 void UnLink();

	virtual bool CmdExecute(ICmdParam& cmd);
	virtual bool StdExecute(IStdParam& cmd);
	virtual bool WndExecute(IWndParam& cmd);

protected:

	bst_set<HeTbarImpl*> m_aSubTbars;
	bst_set<HeMenuImpl*> m_aSubMenus;
	arr_1t<EvtItem> m_aItems;

	void DoPrepareItems(const arr_1t<EvtItem>& a);
	void DoAppendItem(const arr_1t<EvtItem>& a);

};

EW_LEAVE

#endif
