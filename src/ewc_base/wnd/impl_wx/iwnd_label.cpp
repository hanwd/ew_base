#include "ewc_base/wnd/impl_wx/iwnd_label.h"

EW_ENTER

IWnd_label::IWnd_label(wxWindow* p,const WndPropertyEx& h)
	:wxStaticText(p,h.id(),h.label2(),h,h,h.flag_text())
{

}

template<>
class ValidatorW<IWnd_label> : public Validator
{
public:
	LitePtrT<IWnd_label> pWindow;
	ValidatorW(IWnd_label* w):pWindow(w)
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
};

IMPLEMENT_WX_CONTROL(IWnd_label)

EW_LEAVE
