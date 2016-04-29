#ifndef __UI_IWND_COMBO__
#define __UI_IWND_COMBO__

#include "ewc_base/wnd/impl_wx/window.h"
#include "wx/combobox.h"
#include "wx/ownerdrw.h"
#include "wx/odcombo.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE IWnd_combo : public wxOwnerDrawnComboBox
{
public:
	IWnd_combo(wxWindow* p,const WndPropertyEx& h);
	ValidatorHolderT<Validator> m_pVald;
};

EW_LEAVE

#endif
