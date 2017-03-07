#include "ewc_base/wnd/impl_wx/iwnd_textctrl.h"


EW_ENTER


IWnd_textctrl::IWnd_textctrl(wxWindow* p,const WndPropertyEx& h,int w)
	:wxTextCtrl(p,h.id(),h.value(),h,h,h.flag_text()|w|wxTE_PROCESS_ENTER)
{
	if(!h.flags().get(IDefs::IWND_MULTILINE))
	{
		this->SetHint(h.hint());
	}

	this->Connect(wxEVT_TEXT,wxCharEventHandler(IWnd_controlT<IWnd_textctrl>::OnChar));
	this->Connect(wxEVT_TEXT_ENTER,wxCommandEventHandler(IWnd_controlT<IWnd_textctrl>::OnEnter));
	this->Connect(wxEVT_RIGHT_UP,wxMouseEventHandler(IWnd_controlT<IWnd_textctrl>::OnContextEditMenu));
	this->Connect(wxEVT_MENU,wxCommandEventHandler(IWnd_controlT<IWnd_textctrl>::OnCommandEditEvent));

}

IPassword::IPassword(wxWindow* p,const WndPropertyEx& h):IWnd_textctrl(p,h,wxTE_PASSWORD)
{

}

class DLLIMPEXP_EWC_BASE IWndControlT_SearchCtrl : public IWnd_controlT<ISearchCtrl>
{
public:
	void OnClear(wxCommandEvent&)
	{
		if(!m_pVald) return;
		this->Clear();
		m_pVald->WndExecuteEx(IDefs::ACTION_VALUE_CHANGED);
	}
};

ISearchCtrl::ISearchCtrl()
{

}

ISearchCtrl::ISearchCtrl(wxWindow* p,const WndPropertyEx& h)
	:wxSearchCtrl(p,h.id(),wxEmptyString,h,h,wxTE_PROCESS_ENTER)
{
	this->ShowCancelButton(true);
	this->Connect(wxEVT_TEXT,wxCharEventHandler(IWnd_controlT<ISearchCtrl>::OnChar));
	this->Connect(wxEVT_TEXT_ENTER,wxCommandEventHandler(IWnd_controlT<ISearchCtrl>::OnEnter));

	this->Connect(wxEVT_SEARCHCTRL_CANCEL_BTN,wxCommandEventHandler(IWndControlT_SearchCtrl::OnClear));
}

template<>
class ValidatorW<IWnd_textctrl> : public ValidatorW<wxTextEntryBase>
{
public:
	LitePtrT<IWnd_textctrl> pWindow;
	ValidatorW(IWnd_textctrl* w):ValidatorW<wxTextEntryBase>(w),pWindow(w)
	{
		pWindow->m_pVald.reset(this);
	}
};


template<>
class ValidatorW<ISearchCtrl> : public ValidatorW<wxTextEntryBase>
{
public:
	LitePtrT<ISearchCtrl> pWindow;
	ValidatorW(ISearchCtrl* w):ValidatorW<wxTextEntryBase>(w),pWindow(w)
	{
		pWindow->m_pVald.reset(this);
	}
};

template<>
class WndInfoT<IWnd_textctrl> : public WndInfoBaseT<IWnd_textctrl> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_textctrl>(s)
	{

	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<int32_t>* p)
	{
		return CreateValidatorBaseT(w,p);
	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<String>* p)
	{
		return CreateValidatorBaseT(w,p);
	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<double>* p)
	{
		return CreateValidatorBaseT(w,p);
	}

	Validator* CreateValidator(wxWindow* w,EvtBase* p)
	{
		return CreateValidatorVoidT(w,p);
	}

	Validator* CreateValidator(wxWindow* w)
	{
		return new ValidatorW<IWnd_textctrl>((IWnd_textctrl*)w);
	}
};


template<>
class WndInfoT<IPassword> : public WndInfoT<IWnd_textctrl> 
{
public:
	typedef IPassword T;

	WndInfoT(const String& s):WndInfoT<IWnd_textctrl>(s){}

	wxWindow* Create(wxWindow* w,const WndPropertyEx& h)
	{
		return new T(w,h);
	}

};


template<>
class WndInfoT<ISearchCtrl> : public WndInfoBaseT<ISearchCtrl> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<ISearchCtrl>(s)
	{

	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<int32_t>* p)
	{
		return CreateValidatorBaseT(w,p);
	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<String>* p)
	{
		return CreateValidatorBaseT(w,p);
	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<double>* p)
	{
		return CreateValidatorBaseT(w,p);
	}

	Validator* CreateValidator(wxWindow* w,EvtBase* p)
	{
		return CreateValidatorVoidT(w,p);
	}

	Validator* CreateValidator(wxWindow* w)
	{
		return new ValidatorW<ISearchCtrl>((ISearchCtrl*)w);
	}
};

template<>
void WndInfoManger_Register<IWnd_textctrl>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_textctrl> info(name);
	imgr.Register(&info);
}

template<>
void WndInfoManger_Register<IPassword>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IPassword> info(name);
	imgr.Register(&info);
}

template<>
void WndInfoManger_Register<ISearchCtrl>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<ISearchCtrl> info(name);
	imgr.Register(&info);
}

ICmdProcTextEntryCtrl::ICmdProcTextEntryCtrl(IWnd_textctrl& t):basetype(t),Target(t),data(IDat_search::current()){}

bool ICmdProcTextEntryCtrl::TestSelection()
{
	long p1,p2(0);
	Target.GetSelection(&p1,&p2);
	if(p1==p2)
	{
		return false;
	}

	if(Target.GetStringSelection()==str2wx(data.text_old))
	{
		return true;
	}
	return false;
}

bool ICmdProcTextEntryCtrl::DoFind(ICmdParam& cmd)
{
	
	long p1,p2(0);
	Target.GetSelection(&p1,&p2);

	if(TestSelection())
	{
		p1=p2;
	}
	else
	{
		p1=Target.GetInsertionPoint();
	}


	Target.SetSelection(p1,-1);
	wxString v=Target.GetStringSelection();

	long n=v.Find(str2wx(data.text_old));
	if(n<0)
	{
		Target.SelectNone();
		return false;
	}

	p1+=n;
	p2=p1+data.text_old.size();

	Target.SetInsertionPoint(p1);
	Target.SetSelection(p1,p2);

	return true;
}


bool ICmdProcTextEntryCtrl::DoReplace(ICmdParam& cmd)
{	
	if(!TestSelection())
	{
		return DoFind(cmd);
	}

	long p1,p2(0);
	Target.GetSelection(&p1,&p2);

	Target.Replace(p1,p2,str2wx(data.text_new));
	p2=p1+data.text_new.size();

	Target.SetInsertionPoint(p2);
	Target.SetSelection(p1,p2);

	DoFind(cmd);
	return true;
}

bool ICmdProcTextEntryCtrl::DoReplaceAll(ICmdParam& cmd)
{
	cmd.param2=1;

	Target.SetInsertionPoint(0);
	if(!TestSelection() && !DoFind(cmd))
	{
		Wrapper::MsgsDialog(_hT("no result!"),0);
		return false;
	}

	int n=0;
	try
	{
		while(DoReplace(cmd)) n++;
	}
	catch(std::exception&)
	{
			
	}

	Wrapper::MsgsDialog(String::Format(_hT("%d results!"),n),0);
	return true;
}


bool ICmdProcTextEntryCtrl::DoExecId(ICmdParam& cmd)
{
	switch(cmd.param1)
	{
	case CP_DIRTY:
		Target.MarkDirty();
		break;
	case CP_SAVE_TEMP:
		return true;
	case CP_SAVE_FILE:
		return false;
	case CP_FIND:
		if(data.text_old=="") return false;
		return DoFind(cmd);
	case CP_REPLACE:
		if(data.text_old=="") return false;
		return DoReplace(cmd);
	case CP_REPLACEALL:
		if(data.text_old=="") return false;
		return DoReplaceAll(cmd);
	default:
		return basetype::DoExecId(cmd);
	}
	return true;
}

bool ICmdProcTextEntryCtrl::DoTestId(ICmdParam& cmd)
{
	switch(cmd.param1)
	{
	case CP_DIRTY:
		return Target.IsModified();
	case CP_SAVEAS:
		return true;
	default:
		return basetype::DoTestId(cmd);
	}
}

template class IWnd_controlT<ISearchCtrl>;

EW_LEAVE
