#ifndef __UI_IWND_LABEL__
#define __UI_IWND_LABEL__

#include "ewc_base/wnd/impl_wx/window.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE IWnd_label : public wxStaticText
{
public:
	IWnd_label(wxWindow* p,const WndPropertyEx& h);
	ValidatorHolderT<Validator> m_pVald;
};


EW_LEAVE

#endif
