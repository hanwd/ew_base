#ifndef __UI_IWND_FRAME__
#define __UI_IWND_FRAME__

#include "ewc_base/wnd/impl_wx/topwindow.h"
#include "ewc_base/evt/evt_command.h"
#include "ewc_base/evt/evt_manager.h"

#include "wx/frame.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE IWnd_frame : public IWnd_topwindow<wxFrame>
{
public:

	typedef IWnd_topwindow<wxFrame> basetype;

	IWnd_frame(wxWindow* w,const WndPropertyEx& h);
	~IWnd_frame();

	void OnPaneClose(wxAuiManagerEvent& evt);
	bool UpdateModel();

	bool Layout();
	
};

EW_LEAVE

#endif
