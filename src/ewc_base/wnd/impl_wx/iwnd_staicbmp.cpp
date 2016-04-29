#include "ewc_base/wnd/impl_wx/iwnd_staticbmp.h"


EW_ENTER

IWnd_bitmap::IWnd_bitmap(wxWindow* p,const WndPropertyEx& h)
	:wxStaticBitmap(p,h.id(),wxBitmap(40,40),h,h)
{

}
	
template<>
class ValidatorW<IWnd_bitmap> : public Validator
{
public:
	LitePtrT<IWnd_bitmap> pWindow;
	ValidatorW(IWnd_bitmap* w):pWindow(w)
	{
		pWindow->m_pVald.reset(this);
	}

	virtual bool DoSetValue(const String& v)
	{
		wxBitmap bmp;
		bmp.LoadFile(str2wx(v),wxBITMAP_TYPE_ANY);
		if(bmp.IsOk())
		{
			pWindow->SetBitmap(bmp);
		}
		return true;
	}
};

IMPLEMENT_WX_CONTROL(IWnd_bitmap)

EW_LEAVE
