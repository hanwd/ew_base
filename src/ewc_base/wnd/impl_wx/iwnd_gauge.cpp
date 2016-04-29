#include "ewc_base/wnd/impl_wx/iwnd_gauge.h"

EW_ENTER

IWnd_gauge::IWnd_gauge(wxWindow* w,const WndPropertyEx& h)
	:wxGauge(w,h.id(),1000,h,h)
{
}

template<>
class ValidatorW<IWnd_gauge> : public Validator
{
public:
	LitePtrT<IWnd_gauge> pWindow;
	ValidatorW(IWnd_gauge* w):pWindow(w)
	{
		pWindow->m_pVald.reset(this);
	}

	virtual bool DoSetValue(int32_t v)
	{
		if(v<0)
		{
			pWindow->Pulse();
		}
		else
		{
			pWindow->SetValue(v);
		}
		return true;
	}

	virtual bool DoGetValue(int32_t& v)
	{
		v=pWindow->GetValue();
		return true;
	}

	virtual bool DoSetValue(double v)
	{
		if(v<0)
		{
			pWindow->Pulse();
		}
		else
		{
			if(v>1.0) v=1.0;
			pWindow->SetValue((int)(v*1000.0));
		}
		return true;
	}

	virtual bool DoGetValue(double& v)
	{
		int n=pWindow->GetValue();
		v=double(n)/1000.0;
		return true;
	}
};


template<>
class WndInfoT<IWnd_gauge> : public WndInfoBaseT<IWnd_gauge> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_gauge>(s)
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
		return new ValidatorW<IWnd_gauge>((IWnd_gauge*)w);
	}

};


template<>
void WndInfoManger_Register<IWnd_gauge>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_gauge> info(name);
	imgr.Register(&info);
}

EW_LEAVE
