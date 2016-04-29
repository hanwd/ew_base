#ifndef __UI_IWND_CONTAINER__
#define __UI_IWND_CONTAINER__

#include "ewc_base/wnd/impl_wx/window.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE IWnd_container : public wxWindow
{
public:
	IWnd_container(wxWindow* p,const WndPropertyEx& h);


};

EW_LEAVE

#endif
