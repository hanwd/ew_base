#include "ewc_base/wnd/impl_wx/iwnd_glcanvas.h"


EW_ENTER

IWnd_glcanvas::IWnd_glcanvas(wxWindow* p,const WndPropertyEx& h)
	:wxGLCanvas(p,h.id(),NULL,h,h)
{
	
}

template<> 
class ValidatorW<IWnd_glcanvas> : public ValidatorGroup
{
public:
	LitePtrT<IWnd_glcanvas> pWindow;
	ValidatorW(IWnd_glcanvas* w):pWindow(w){}
};


template<>
class WndInfoT<IWnd_glcanvas> : public WndInfoBaseT<IWnd_glcanvas> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_glcanvas>(s)
	{

	}

	Validator* CreateValidator(wxWindow* w)
	{
		return new ValidatorW<IWnd_glcanvas>((IWnd_glcanvas*)w);
	}

	void Update(WndMaker& wk,wxWindow* pw,EvtBase* pv)
	{
		wk.vd_set(CreateValidatorEx(pw,pv));
	}

};

template<>
void WndInfoManger_Register<IWnd_glcanvas>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_glcanvas> info(name);
	imgr.Register(&info);
}

EW_LEAVE
