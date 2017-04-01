#ifndef __UI_IWND_GLCANVAS__
#define __UI_IWND_GLCANVAS__

#include "ewc_base/wnd/impl_wx/window.h"
#include "wx/glcanvas.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE IWnd_glcanvas : public wxGLCanvas
{
public:
	IWnd_glcanvas(wxWindow* p, const WndPropertyEx& h);


};

EW_LEAVE

#endif
