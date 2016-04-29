#ifndef __UI_IWND_SPINCTRL__
#define __UI_IWND_SPINCTRL__

#include "ewc_base/wnd/impl_wx/window.h"
#include "wx/spinctrl.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE IWnd_spinctrl : public wxSpinCtrl
{
public:
	IWnd_spinctrl(wxWindow* p,const WndPropertyEx& h);
	ValidatorHolderT<Validator> m_pVald;
};

EW_LEAVE

#endif
