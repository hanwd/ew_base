#ifndef __H_EW_UI_EVT_GROUP__
#define __H_EW_UI_EVT_GROUP__

#include "ewc_base/evt/evt_command.h"
#include "ewc_base/app/data_defs.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE EvtGroup  : public ObjectGroupT<EvtCommand,EvtCommandWindow>
{
public:
	typedef ObjectGroupT<EvtCommand,EvtCommandWindow> basetype;

	friend class IEW_Ctrl;

	EvtGroup(const String& name="");

	virtual void DoUpdateCtrl(IUpdParam& upd);
	virtual void PrepareItems();

	virtual void CreateCtrlItem(IEW_Ctrl* pctrl);

	IMenuPtr CreateMenu(IEW_MenuImpl* mu=NULL,bool prepare=true);
	IWindowPtr CreateCtrl(IWindowPtr pw,int wd=-1,const String& type="");

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

	bst_set<IEW_Ctrl*> m_aCtrls;
	arr_1t<EvtItem> m_aItems;

	void DoPrepareItems(const arr_1t<EvtItem>& a);
	virtual void DoAppendItem(const arr_1t<EvtItem>& a);

};

class DLLIMPEXP_EWC_BASE EvtRadio : public EvtGroup
{
public:
	typedef EvtGroup basetype;
	EvtRadio(const String& id);

	EvtGroup* CreateGroup(const String& s){return new EvtRadio(s);}

	virtual void DoUpdateCtrl(IUpdParam& upd);
	void DoAppendItem(const arr_1t<EvtItem>& a);
	bool OnCmdEvent(ICmdParam& cmd, int phase);
};


EW_LEAVE

#endif
