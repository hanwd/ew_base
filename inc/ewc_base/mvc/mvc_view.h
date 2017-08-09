#ifndef __H_EW_UI_DOCVIEW__
#define __H_EW_UI_DOCVIEW__

#include "ewc_base/app/cmdproc.h"
#include "ewc_base/wnd/impl_wx/window.h"

EW_ENTER

class MvcViewProxy;
class DataModel;

class MvcItemData : public ObjectData
{
public:
	virtual bool DoActivate(WndManager&, int) = 0;
};

class DLLIMPEXP_EWC_BASE MvcView : public wxEvtHandler
{
public:

	enum
	{
		FLAG_MIN	= 1,
		FLAG_INITED = FLAG_MIN << 0,
		FLAG_ACTIVE = FLAG_MIN << 1,
		FLAG_MAX = FLAG_MIN << 2,
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

	virtual void UpdateStatus(){}

	static MvcView* ms_pActiveView;

	IFileNameHolder fn;

	bool LinkBookData(const String& sbook, DataModel* pmodel);

	void DoRefresh();

protected:
	virtual bool DoClose(WndManager&);
	virtual bool DoActivate(WndManager&,int);

	arr_1t<DataPtrT<MvcItemData> > arr_items;

	LitePtrT<wxWindow> m_pCanvas;
	BitFlags flags;

	AtomicInt32 m_nRequestFresh;

	virtual bool DoInitialize();



};

class MvcViewEx : public MvcView
{
public:

	MvcViewEx(MvcModel& tar);

	bool DoActivate(WndManager&,int v);

protected:
	DataPtrT<CmdProc> m_pCmdProc;

};

EW_LEAVE

#endif
