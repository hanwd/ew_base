#ifndef __UI_IWND_TIMECTRL__
#define __UI_IWND_TIMECTRL__

#include "ewc_base/wnd/impl_wx/window.h"
#include "wx/timectrl.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE IWnd_timectrl : public wxTimePickerCtrl
{
public:
	IWnd_timectrl(wxWindow* p,const WndPropertyEx& h);
	ValidatorHolderT<Validator> m_pVald;
};


EW_LEAVE

#endif
