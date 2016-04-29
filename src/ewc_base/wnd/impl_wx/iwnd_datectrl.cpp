#include "ewc_base/wnd/impl_wx/iwnd_datectrl.h"


EW_ENTER

IWnd_datectrl::IWnd_datectrl(wxWindow* p,const WndPropertyEx& h)
	:wxDatePickerCtrl(p,h.id(),wxDefaultDateTime,h,h)
{

}

IMPLEMENT_WX_CONTROL(IWnd_datectrl)

EW_LEAVE
