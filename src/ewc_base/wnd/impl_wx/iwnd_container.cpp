#include "ewc_base/wnd/impl_wx/iwnd_container.h"


EW_ENTER

IWnd_container::IWnd_container(wxWindow* p,const WndPropertyEx& h)
	:wxWindow(p,h.id(),h,h)
{
	
}

template<> 
class ValidatorW<IWnd_container> : public ValidatorGroup
{
public:
	LitePtrT<IWnd_container> pWindow;
	ValidatorW(IWnd_container* w):pWindow(w){}
};


template<>
class WndInfoT<IWnd_container> : public WndInfoBaseT<IWnd_container> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_container>(s)
	{

	}

	Validator* CreateValidator(wxWindow* w)
	{
		return new ValidatorW<IWnd_container>((IWnd_container*)w);
	}

	void Update(WndMaker& wk,wxWindow* pw,EvtBase* pv)
	{
		wk.vd_set(CreateValidatorEx(pw,pv));
	}

};

template<>
void WndInfoManger_Register<IWnd_container>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_container> info(name);
	imgr.Register(&info);
}

EW_LEAVE
