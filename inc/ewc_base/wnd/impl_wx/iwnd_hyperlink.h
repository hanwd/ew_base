#ifndef __UI_IWND_HYPERLINK__
#define __UI_IWND_HYPERLINK__

#include "ewc_base/wnd/impl_wx/window.h"
#include "wx/hyperlink.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE IWnd_hyperlink : public wxHyperlinkCtrl
{
public:
	IWnd_hyperlink(wxWindow* p,const WndPropertyEx& h);
	ValidatorHolderT<Validator> m_pVald;
};

EW_LEAVE

#endif
