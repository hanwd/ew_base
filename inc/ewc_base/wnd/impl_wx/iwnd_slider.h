#ifndef __UI_IWND_SLIDER__
#define __UI_IWND_SLIDER__

#include "ewc_base/wnd/impl_wx/window.h"
#include "wx/slider.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE IWnd_slider : public wxSlider
{
public:
	IWnd_slider(wxWindow* w,const WndPropertyEx& h);
	ValidatorHolderT<Validator> m_pVald;
};

EW_LEAVE

#endif
