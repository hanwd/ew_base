#include "ewc_base/wnd/impl_wx/iwnd_spinctrl.h"


EW_ENTER

IWnd_spinctrl::IWnd_spinctrl(wxWindow* p,const WndPropertyEx& h)
	:wxSpinCtrl(p,h.id(),wxEmptyString,h,h)
{
	this->SetMin(0);
	this->SetMax(100);
}

template<>
class ValidatorW<IWnd_spinctrl> : public Validator
{
public:
	LitePtrT<IWnd_spinctrl> pWindow;
	ValidatorW(IWnd_spinctrl* w):pWindow(w)
	{
		pWindow->m_pVald.reset(this);
	}

	virtual bool DoSetValue(int32_t v)
	{
		pWindow->SetValue(v);
		return true;
	}

	virtual bool DoGetValue(int32_t& v)
	{
		v=pWindow->GetValue();
		return true;
	}
};


template<>
class WndInfoT<IWnd_spinctrl> : public WndInfoBaseT<IWnd_spinctrl> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_spinctrl>(s)
	{

	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<int32_t>* p)
	{
		return CreateValidatorBaseT(w,p);
	}

	Validator* CreateValidator(wxWindow* w)
	{
		return new ValidatorW<IWnd_spinctrl>((IWnd_spinctrl*)w);
	}

};


template<>
void WndInfoManger_Register<IWnd_spinctrl>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_spinctrl> info(name);
	imgr.Register(&info);
}


EW_LEAVE
