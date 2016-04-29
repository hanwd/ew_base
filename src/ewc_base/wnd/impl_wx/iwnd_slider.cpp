#include "ewc_base/wnd/impl_wx/iwnd_slider.h"


EW_ENTER

IWnd_slider::IWnd_slider(wxWindow* w,const WndPropertyEx& h)
	:wxSlider(w,h.id(),0,0,100,h,h)
{

}

template<>
class ValidatorW<IWnd_slider> : public Validator
{
public:
	LitePtrT<IWnd_slider> pWindow;
	ValidatorW(IWnd_slider* w):pWindow(w)
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

	virtual bool DoSetValue(double v)
	{
		pWindow->SetValue((int)(v*100.0));
		return true;
	}

	virtual bool DoGetValue(double& v)
	{
		v=0.01*(pWindow->GetValue());
		return true;
	}
};



template<>
class WndInfoT<IWnd_slider> : public WndInfoBaseT<IWnd_slider> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_slider>(s)
	{

	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<int32_t>* p)
	{
		return CreateValidatorBaseT(w,p);
	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<double>* p)
	{
		return CreateValidatorBaseT(w,p);
	}

	Validator* CreateValidator(wxWindow* w)
	{
		return new ValidatorW<IWnd_slider>((IWnd_slider*)w);
	}
};


template<>
void WndInfoManger_Register<IWnd_slider>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_slider> info(name);
	imgr.Register(&info);
}

EW_LEAVE
