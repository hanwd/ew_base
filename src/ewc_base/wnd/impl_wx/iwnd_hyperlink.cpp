
#include "ewc_base/wnd/impl_wx/iwnd_hyperlink.h"


EW_ENTER

IWnd_hyperlink::IWnd_hyperlink(wxWindow* p,const WndPropertyEx& h)
	:wxHyperlinkCtrl(p,h.id(),h.label(),h.desc(),h,h)
{
		
}

template<>
class ValidatorW<IWnd_hyperlink> : public Validator
{
public:
	LitePtrT<IWnd_hyperlink> pWindow;
	ValidatorW(IWnd_hyperlink* w):pWindow(w)
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
		pWindow->SetURL(str2wx(v));
		return true;
	}

	virtual bool DoGetValue(String& v)
	{
		v=wx2str(pWindow->GetURL());
		return true;
	}

};


IMPLEMENT_WX_CONTROL(IWnd_hyperlink)

EW_LEAVE




