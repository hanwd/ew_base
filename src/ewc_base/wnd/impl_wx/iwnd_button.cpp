#include "ewc_base/wnd/impl_wx/iwnd_button.h"

EW_ENTER

IWnd_button::IWnd_button(wxWindow* p,const WndPropertyEx& h)
	:wxButton(p,h.id(),h.label1(),h,h)
{

}

IWnd_cmdlink_button::IWnd_cmdlink_button(wxWindow* p,const WndPropertyEx& h)
	:wxCommandLinkButton(p,h.id(),h.label1(),h.desc(),h,h)
{

}

template<>
class ValidatorW<IWnd_cmdlink_button> : public Validator
{
public:
	LitePtrT<IWnd_cmdlink_button> pWindow;
	ValidatorW(IWnd_cmdlink_button* w):pWindow(w)
	{
		pWindow->m_pVald.reset(this);
	}

	virtual bool DoSetLabel(const String& v)
	{
		pWindow->SetLabel(str2wx(v));
		return true;
	}

	virtual bool DoGetLabel(String& v)
	{
		v=wx2str(pWindow->GetLabel());
		return true;
	}

	virtual bool DoSetValue(const String& v)
	{
		pWindow->SetNote(str2wx(v));
		return true;
	}

	virtual bool DoGetValue(String& v)
	{
		v=wx2str(pWindow->GetNote());
		return true;
	}
};


template<>
class ValidatorW<IWnd_button> : public Validator
{
public:
	LitePtrT<IWnd_button> pWindow;

	ValidatorW(IWnd_button* w):pWindow(w)
	{
		pWindow->m_pVald.reset(this);
	}

	~ValidatorW()
	{
		
	}

	//virtual bool OnWndEvent(IWndParam& cmd,int phase)
	//{
	//	if(cmd.action==IDefs::ACTION_UPDATECTRL)
	//	{
	//		pWindow->Enable(!proxy->flags.get(EvtBase::FLAG_DISABLE));
	//		pWindow->Show(proxy->flags.get(EvtBase::FLAG_HIDE_UI));
	//		DoSetLabel(proxy->MakeLabel(EvtCommand::LABEL_TOOL));
	//		return true;
	//	}
	//	else
	//	{
	//		return Validator::OnWndEvent(cmd,phase);
	//	}
	//}

	virtual bool DoSetLabel(const String& v)
	{
		pWindow->SetLabel(str2wx(v));
		return true;
	}

	virtual bool DoGetLabel(String& v)
	{
		v=wx2str(pWindow->GetLabel());
		return true;
	}

};

template<>
class WndInfoT<IWnd_button> : public WndInfoBaseT<IWnd_button> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_button>(s)
	{

	}

	virtual Validator* CreateValidator(wxWindow* w,EvtBase* p)
	{
		return CreateValidatorVoidT(w,p);
	}
};

template<>
class WndInfoT<IWnd_cmdlink_button> : public WndInfoBaseT<IWnd_cmdlink_button> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_cmdlink_button>(s)
	{

	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<String>* p)
	{
		return CreateValidatorBaseT(w,p);
	}

	virtual Validator* CreateValidator(wxWindow* w,EvtBase* p)
	{
		return CreateValidatorVoidT(w,p);
	}


};

template<>
void WndInfoManger_Register<IWnd_button>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_button> info(name);
	imgr.Register(&info);
}

template<>
void WndInfoManger_Register<IWnd_cmdlink_button>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_cmdlink_button> info(name);
	imgr.Register(&info);
}


EW_LEAVE
