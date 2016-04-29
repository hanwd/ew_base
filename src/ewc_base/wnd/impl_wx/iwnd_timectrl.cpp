
#include "ewc_base/wnd/impl_wx/iwnd_timectrl.h"

EW_ENTER


IWnd_timectrl::IWnd_timectrl(wxWindow* p,const WndPropertyEx& h)
	:wxTimePickerCtrl(p,h.id(),wxDefaultDateTime,h,h)
{
		
}

IMPLEMENT_WX_CONTROL(IWnd_timectrl)

EW_LEAVE
