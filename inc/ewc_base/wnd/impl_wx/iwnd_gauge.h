#ifndef __UI_IWND_GAUGE__
#define __UI_IWND_GAUGE__

#include "ewc_base/wnd/impl_wx/window.h"
#include "wx/gauge.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE IWnd_gauge : public wxGauge
{
public:
	IWnd_gauge(wxWindow* w,const WndPropertyEx& h);
	ValidatorHolderT<Validator> m_pVald;
};


EW_LEAVE

#endif
