#include "ewc_base/wnd/impl_wx/iwnd_checkbox.h"


EW_ENTER

IWnd_checkbox::IWnd_checkbox(wxWindow* p,const WndPropertyEx& h)
	:wxCheckBox(p,h.id(),h.label(),h,h)
{
	this->Connect(wxEVT_CHECKBOX,wxCommandEventHandler(IWnd_controlT<IWnd_checkbox>::OnCommandIntChanged));
}

template<>
class ValidatorW<IWnd_checkbox> : public Validator
{
public:
	LitePtrT<IWnd_checkbox> pWindow;
	wxWindow* GetWindow(){return pWindow;}

	ValidatorW(IWnd_checkbox* w):pWindow(w)
	{
		pWindow->m_pVald.reset(this);
	}

	virtual bool DoSetValue(int32_t v)
	{
		pWindow->SetValue(v!=0);
		return true;
	}

	virtual bool DoGetValue(int32_t& v)
	{
		v=pWindow->GetValue()?1:0;
		return true;
	}
};


template<>
class WndInfoT<IWnd_checkbox> : public WndInfoBaseT<IWnd_checkbox> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_checkbox>(s)
	{

	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<int32_t>* p)
	{
		return CreateValidatorBaseT(w,p);
	}


	Validator* CreateValidator(wxWindow* w)
	{
		return new ValidatorW<IWnd_checkbox>((IWnd_checkbox*)w);
	}
};


template<>
void WndInfoManger_Register<IWnd_checkbox>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_checkbox> info(name);
	imgr.Register(&info);
}

EW_LEAVE
