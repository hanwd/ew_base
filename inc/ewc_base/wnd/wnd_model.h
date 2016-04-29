#ifndef __H_EW_UI_WINMODEL__
#define __H_EW_UI_WINMODEL__

#include "ewc_base/config.h"
#include "ewc_base/evt/validator.h"
#include "ewc_base/evt/evt_base.h"
#include "ewc_base/evt/evt_element.h"
#include "ewc_base/evt/evt_command.h"
#include "ewc_base/evt/evt_array.h"
#include "ewc_base/evt/evt_manager.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE WndModel : public EvtBase
{
public:

	typedef EvtBase basetype;

	enum
	{
		FLAG_MIN		=basetype::FLAG_MAX,
		FLAG_CREATED	=FLAG_MIN<<0,
		FLAG_MODEL_TOP	=FLAG_MIN<<1,
		FLAG_NO_CLOSE	=FLAG_MIN<<2,
		FLAG_AUTO_FIT	=FLAG_MIN<<3,
		FLAG_MAX		=FLAG_MIN<<4,


	};

	friend class WndManager;
	friend class WndMaker;

	WndModel(const String& s="");
	~WndModel();

	bool WndExecuteEx(int action);

	virtual bool DoWndExecute(IWndParam& cmd);

	virtual bool OnCmdExecute(int evtid,int p1);

	virtual void OnChildWindow(IWindowPtr w,int a);


	virtual void Close();

	void UpdateCtrl();
	virtual void DoUpdateCtrl(IUpdParam& upd);

	virtual bool Show(bool f);
	virtual int ShowModal();

	bool IsModal();
	void EndModal();
	virtual void EndModal(int h);

	void SetSize(int w,int h);


	IWindowPtr GetWindow();

	bool IsShown();

	virtual bool mu_set(const String& a);

	static WndModel& current();

	EvtBase* get_item(const String& s);

	void SetParent(wxWindow* w){m_pParent.reset(w);}
	wxWindow* GetParent(){return m_pParent.get();}

	void SetLabel(const String& s);

	bool EnsureCreated(bool t=false);

	void AttachEvent(const String& s);
	void DetachEvent(const String& s);

protected:

	virtual bool DoUpdate(WndMaker& km);

	bool DoCmdExecute(ICmdParam& cmd);
	bool DoStdExecute(IStdParam& cmd);

	virtual bool Create(){return false;}


	DataPtrT<ValidatorGroup> vald_grp;
	DataPtrT<ValidatorTop> vald_top;
	EvtManager local_evtmgr;

	LitePtrT<wxWindow> m_pWindow;
	LitePtrT<wxWindow> m_pParent;

};


class DLLIMPEXP_EWC_BASE WndModelTop : public WndModel
{
public:
	WndManager& wm;
	WndModelTop(WndManager& w):wm(w),WndModel()
	{
		flags.add(FLAG_MODEL_TOP);
	}
	bool Create();
};


class WndModelScript: public WndModel
{
public:
	typedef WndModel basetype;

	String m_sFilename;
	
	WndModelScript(const String& s,const String& f,int h=0);

	bool Create();

};


EW_LEAVE
#endif
