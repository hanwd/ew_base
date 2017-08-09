#ifndef __UI_IWND_MODELVIEW__
#define __UI_IWND_MODELVIEW__

#include "ewc_base/wnd/impl_wx/window.h"
#include "ewc_base/wnd/wnd_glcontext.h"
#include "ewc_base/data/data_node.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE IWnd_modelview : public wxWindow
{
public:
	IWnd_modelview(wxWindow* p, const WndPropertyEx& h);

	void OnPaint(wxPaintEvent& evt);
	void OnMouseEvent(wxMouseEvent& evt);
	void OnKeyEvent(wxKeyEvent& evt);
	void OnSizeEvent(wxSizeEvent& evt);

	void ImageUpdate();

	void PendingRefresh();

	GLTool gt;
	GLDC dc;
	LitePtrT<DataModel> pmodel;

protected:

	bool m_bPendingRefresh;

	bool Create(wxWindow *parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size,
		long style
		);

	bool CreateWindow(wxWindow *parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size,
		long style,
		const wxString& name);
};

EW_LEAVE

#endif
