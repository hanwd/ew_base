#include "ewc_base/wnd/impl_wx/iwnd_breaker.h"


EW_ENTER

IWnd_hline::IWnd_hline(wxWindow* w,const WndPropertyEx& h)
	:wxStaticLine(w,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxHORIZONTAL)
{
}


IWnd_vline::IWnd_vline(wxWindow* w,const WndPropertyEx& h)
	:wxStaticLine(w,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxVERTICAL)
{
}

IMPLEMENT_WX_CONTROL(IWnd_hline)
IMPLEMENT_WX_CONTROL(IWnd_vline)


EW_LEAVE
