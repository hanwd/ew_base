#ifndef __UI_IWND_CHECKBOX__
#define __UI_IWND_CHECKBOX__

#include "ewc_base/wnd/impl_wx/window.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE IWnd_checkbox : public wxCheckBox
{
public:
	IWnd_checkbox(wxWindow* p,const WndPropertyEx& h);
	ValidatorHolderT<Validator> m_pVald;
};


EW_LEAVE

#endif
