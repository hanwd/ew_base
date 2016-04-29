#ifndef __UI_IWND_DIALOG__
#define __UI_IWND_DIALOG__

#include "ewc_base/wnd/impl_wx/topwindow.h"
#include "wx/dialog.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE IWnd_dialog : public IWnd_topwindow<wxDialog>
{
public:

	IWnd_dialog(wxWindow* w,const WndPropertyEx& h);

};


EW_LEAVE

#endif
