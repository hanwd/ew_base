#ifndef __UI_IWND_BREAKER__
#define __UI_IWND_BREAKER__

#include "ewc_base/wnd/impl_wx/window.h"
#include "wx/statline.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE IWnd_hline : public wxStaticLine
{
public:
	IWnd_hline(wxWindow* w,const WndPropertyEx& h);
};

class DLLIMPEXP_EWC_BASE IWnd_vline : public wxStaticLine
{
public:
	IWnd_vline(wxWindow* w,const WndPropertyEx& h);
};

EW_LEAVE
#endif
