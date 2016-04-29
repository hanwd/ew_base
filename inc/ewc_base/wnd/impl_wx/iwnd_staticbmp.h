#ifndef __UI_IWND_STATICBMP__
#define __UI_IWND_STATICBMP__

#include "ewc_base/wnd/impl_wx/window.h"
#include "wx/statbmp.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE IWnd_bitmap : public wxStaticBitmap
{
public:
	IWnd_bitmap(wxWindow* p,const WndPropertyEx& h);
	ValidatorHolderT<Validator> m_pVald;
};

EW_LEAVE

#endif
