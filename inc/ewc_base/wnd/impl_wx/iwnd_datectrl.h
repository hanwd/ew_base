#ifndef __UI_IWND_DATECTRL__
#define __UI_IWND_DATECTRL__

#include "ewc_base/wnd/impl_wx/window.h"
#include "wx/datectrl.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE IWnd_datectrl : public wxDatePickerCtrl
{
public:
	IWnd_datectrl(wxWindow* p,const WndPropertyEx& h);
	ValidatorHolderT<Validator> m_pVald;
};

EW_LEAVE

#endif
