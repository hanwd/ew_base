#ifndef __UI_IWND_BUTTON__
#define __UI_IWND_BUTTON__
#include "ewc_base/wnd/impl_wx/window.h"
#include "wx/commandlinkbutton.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE IWnd_cmdlink_button : public  wxCommandLinkButton
{
public:
	IWnd_cmdlink_button(wxWindow* p,const WndPropertyEx& h);
	ValidatorHolderT<Validator> m_pVald;
};

class DLLIMPEXP_EWC_BASE IWnd_button : public wxButton
{
public:
	IWnd_button(wxWindow* p,const WndPropertyEx& h);
	ValidatorHolderT<Validator> m_pVald;
};

EW_LEAVE

#endif
