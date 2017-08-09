#ifndef __H_EW_UI_VALUEPROXY_COMMAND__
#define __H_EW_UI_VALUEPROXY_COMMAND__

#include "ewc_base/config.h"
#include "ewc_base/evt/evt_base.h"
#include "ewc_base/app/res_bitmap.h"
#include "ewc_base/wnd/wnd_property.h"



EW_ENTER

class DLLIMPEXP_EWC_BASE IWnd_bookbase;

class DLLIMPEXP_EWC_BASE ICtl_itemdata : public ObjectData
{
public:

	ICtl_itemdata(EvtCommand* p);
	~ICtl_itemdata();

	// UpdateCtrl()  返回-1表示相应控件已经被删除
	virtual int UpdateCtrl(){ return 0; }

	virtual void UpdateBmps(){}

	DataPtrT<EvtCommand> pevt;

};

class DLLIMPEXP_EWC_BASE ICtl_object : public Object
{
public:

	ICtl_object(EvtGroup* p=NULL);
	~ICtl_object();

	virtual bool StdExecute(IStdParam&){return true;}

	virtual bool AddCtrlItem(EvtCommand* pevt);
	virtual bool AddCtrlItem(EvtCommand* pevt, wxControl* p);
	virtual bool AddCtrlItem(EvtGroup* pevt);

	virtual wxMenu* GetMenu(){ return NULL; }
	virtual wxWindow* GetWindow(){return NULL;}


	static void WndRegister(const String& type,Functor<ICtl_object*(const ICtlParam&,EvtGroup*)> func);
	static ICtl_object* WndCreateCtrl(const ICtlParam& param,EvtGroup* pevt);

	virtual void WndUpdateCtrl(){}
	virtual bool WndIsOk(){return m_pGroup;}

	bool StdExecuteEx(const String& func)
	{
		IStdParam cmd(func);
		return StdExecute(cmd);
	}

protected:
	ObjectGroupT<ICtl_itemdata> m_aItems;
	DataPtrT<EvtGroup> m_pGroup;
};


class DLLIMPEXP_EWC_BASE EvtCommand  : public EvtBase
{
public:

	friend class ICtl_itemdata;

	String m_sText;
	String m_sExtra;
	String m_sLabel;
	String m_sHelp;
	String m_sHotkey;
	String m_sAccel;
	
	EvtCommand(){}
	EvtCommand(const String& s):EvtBase(s)
	{
		m_sText=Translate(s);
	}

	EvtCommand& text(const String& s)
	{
		m_sText=s;
		return *this;
	}

	EvtCommand& help(const String& s)
	{
		m_sHelp=s;
		return *this;
	}

	EvtCommand& hotkey(const String& s)
	{
		m_sHotkey=s;
		return *this;
	}

	EvtCommand& accel(const String& s)
	{
		m_sAccel=s;
		return *this;
	}

	EvtCommand& checkable()
	{
		flags.add(FLAG_CHECK);
		return *this;
	}

	EvtCommand& radio()
	{
		flags.add(FLAG_CHECK|FLAG_RADIO);
		return *this;
	}

	virtual String MakeLabel(int hint=LABEL_TOOL) const;

	virtual void CreateCtrlItem(ICtl_object* pctrl);
	virtual IWindowPtr CreateWndsItem(IWindowPtr pw);
	virtual Validator* CreateValidator(wxWindow*);

	virtual void DoUpdateCtrl(IUpdParam& upd);

	virtual EvtCommand* cast_command(){return this;}

	const BitmapBundle& GetBundle(int w,int t);

	bool DoCmdExecute(ICmdParam&);

protected:

	bst_set<ICtl_itemdata*> m_setAttachedControls;
	BitmapHolder m_bmpParam;

};

class DLLIMPEXP_EWC_BASE EvtCommandSeparator : public EvtCommand
{
public:
	EvtCommandSeparator(const String& s):EvtCommand(s){flags.add(FLAG_SEPARATOR);}
};

class DLLIMPEXP_EWC_BASE EvtCommandWindow : public EvtCommand
{
public:
	typedef EvtCommand basetype;

	EvtCommandWindow(wxWindow* pw=NULL);
	EvtCommandWindow(const String& n,wxWindow* pw=NULL);

	IWindowPtr CreateWndsItem(IWindowPtr pw);

	virtual void DoUpdateCtrl(IUpdParam& upd);

	 wxWindow* GetWindow(){return m_pWindow;}
	 virtual void SetWindow(wxWindow* w);

	bool DoCmdExecute(ICmdParam& cmd);

	virtual bool OnWindow(int flag);

protected:
	LitePtrT<wxWindow> m_pWindow;
};

class DLLIMPEXP_EWC_BASE EvtCommandCtrl : public EvtCommandWindow
{
public:
	typedef EvtCommandWindow basetype;

	EvtCommandCtrl(const String& id,const String& type,const WndProperty& w=WndProperty());


	void CreateCtrlItem(ICtl_object* pctrl);

	virtual IWindowPtr CreateWndsItem(IWindowPtr pw);
	Validator* CreateValidator(wxWindow*);

	virtual void DoUpdateCtrl(IUpdParam& upd);

	bool DoStdExecute(IStdParam& cmd);

protected:

	WndProperty wp;
	String m_sCtrlType;
};


class DLLIMPEXP_EWC_BASE EvtCommandWindowSharedBook : public EvtCommandWindow
{
public:
	typedef EvtCommandWindow basetype;

	EvtCommandWindowSharedBook(const String& n);

	static bool SelPage(const String& s,wxWindow* w);

protected:
	 virtual bool OnWindow(int flag);

	LitePtrT<IWnd_bookbase> m_pWindow;
};

class DLLIMPEXP_EWC_BASE EvtCommandExtraWindow : public EvtCommandCtrl
{
public:
	EvtCommandExtraWindow(const String& id,const String& type,int h=FLAG_HIDE_UI,const WndProperty& w=WndProperty());
};

class DLLIMPEXP_EWC_BASE EvtCommandText : public EvtCommandCtrl
{
public:
	typedef EvtCommandCtrl basetype;
	EvtCommandText(const String& s):basetype(s,"textctrl"){}

	String value;

	virtual bool DoStdExecute(IStdParam& cmd);
	virtual bool DoWndExecute(IWndParam& cmd);

};



template<typename W>
class DLLIMPEXP_EWC_BASE EvtCommandMemberFunction : public EvtCommand
{
public:
	typedef bool (W::*func_type)(ICmdParam&);

	EvtCommandMemberFunction(const String& s,func_type f):EvtCommand(s),func(f){}

	virtual bool CmdExecute(ICmdParam& cmd)
	{
		W* w=dynamic_cast<W*>(cmd.iwmptr.get());
		if(!w) return false;
		return (w->*func)(cmd);
	}

	~EvtCommandMemberFunction(){}

private:
	func_type func;
};


class DLLIMPEXP_EWC_BASE EvtCommandFunctor : public EvtCommand
{
public:
	typedef Functor<bool(ICmdParam&)> func_type;

	EvtCommandFunctor(){}
	EvtCommandFunctor(const String& s):EvtCommand(s){}
	EvtCommandFunctor(const String& s,func_type f):EvtCommand(s),func(f){}

	virtual bool CmdExecute(ICmdParam& cmd);

	~EvtCommandFunctor(){}

private:
	func_type func;
};


class DLLIMPEXP_EWC_BASE EvtCommandShowModel : public EvtCommand
{
public:
	typedef EvtCommand basetype;

	EvtCommandShowModel(const String& s,WndModel* p);
	EvtCommandShowModel(const String& s,const String& p);

	virtual void DoUpdateCtrl(IUpdParam& upd);	

	bool DoCmdExecute(ICmdParam&);

	bool OnWndEvent(IWndParam& cmd,int phase);

protected:

	void _DoSetModel(WndModel* p);

	DataPtrT<WndModel> m_pModel;
};


class DLLIMPEXP_EWC_BASE EvtCommandTimer : public EvtCommand
{
public:

	EvtCommandTimer(const String& s);

	// param2=-1 one_shot timer, param1=delay in ms
	// param2=+1 repeated timer, param1=repeat time in ms
	// param2= 0 stop the timer
	virtual bool DoStdExecute(IStdParam& cmd);

private:
	virtual bool OnCmdEvent(ICmdParam& cmd,int phase);
	AutoPtrT<Object> m_pTimerData;
};


EW_LEAVE
#endif
