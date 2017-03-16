#ifndef __H_EW_UI_DOCVIEW__
#define __H_EW_UI_DOCVIEW__

#include "ewc_base/app/cmdproc.h"
#include "ewc_base/wnd/impl_wx/window.h"

EW_ENTER

class MvcViewProxy;
class DLLIMPEXP_EWC_BASE MvcView : public wxEvtHandler
{
public:

	enum
	{
		FLAG_INITED=1<<0,
		FLAG_ACTIVE=1<<1,
	};


	friend class MvcViewProxy;

	MvcView(MvcModel& tar);
	~MvcView();

	static MvcView* GetViewByCanvas(wxWindow* w);
	static MvcView* GetViewByHandler(wxEvtHandler* h);

	virtual bool OnCreated();
	virtual void OnDestroy();

	virtual void SetCanvas(wxWindow* w);
	wxWindow* GetCanvas(){return m_pCanvas;}

	virtual wxWindow* CreateCanvas(wxWindow* w);

	bool IsActive();

	virtual bool OnClose(WndManager&);
	virtual bool OnActivate(WndManager&,int);

	virtual void UpdateView();

	virtual String GetName();
	virtual String GetTitle();

	void Refresh();
	void CaptureMouse();
	void ReleaseMouse();
	void SetFocus();

	MvcModel& Target;

	virtual bool OnCmdEvent(ICmdParam&,int){return true;}
	virtual bool OnWndEvent(IWndParam&,int){return true;}
	virtual bool OnStdEvent(IStdParam&,int){return true;}

	static MvcView* ms_pActiveView;

	IFileNameHolder fn;

protected:
	virtual bool DoClose(WndManager&);
	virtual bool DoActivate(WndManager&,int);

	LitePtrT<wxWindow> m_pCanvas;
	BitFlags flags;

};

class DLLIMPEXP_EWC_BASE MvcViewEx : public MvcView
{
public:

	MvcViewEx(MvcModel& tar);

	bool DoActivate(WndManager&,int v);

protected:
	DataPtrT<CmdProc> m_pCmdProc;

};

EW_LEAVE

#endif
