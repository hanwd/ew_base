#ifndef __UI_IWND_NOTEBOOK__
#define __UI_IWND_NOTEBOOK__

#include "ewc_base/wnd/impl_wx/window.h"
#include "wx/notebook.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE IWnd_notebook : public wxNotebook
{
public:

	IWnd_notebook(wxWindow* p,const WndPropertyEx& h);

	void OnNBChanging(wxBookCtrlEvent& evt);
	void OnNBChanged(wxBookCtrlEvent& evt);

	bool m_bPageOnly;

	ValidatorHolderT<Validator> m_pVald;
};

EW_LEAVE

#endif
