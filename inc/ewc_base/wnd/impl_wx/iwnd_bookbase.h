#ifndef __UI_IWND_BOOKBASE__
#define __UI_IWND_BOOKBASE__

#include "ewc_base/wnd/impl_wx/window.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE IWnd_bookbase : public wxWindow
{
public:

	typedef wxWindow basetype;

	BitFlags flags;

	IWnd_bookbase(wxWindow* p,const WndPropertyEx& h);
	IWnd_bookbase(wxWindow *p,int f=0);
	~IWnd_bookbase();

	wxWindow* GetPage();
	wxWindow* GetDefault();

	bool SelPage(wxWindow *w);
	void DefPage(wxWindow *w);

	bool IAddPage(wxWindow *w, const WndProperty& wp);

	virtual wxSize DoGetBestSize() const;

	mutable ValidatorHolderT<ValidatorGroupBook> m_pVald;

protected:
	
	LitePtrT<wxWindow> win;
	LitePtrT<wxWindow> def;

	DECLARE_EVENT_TABLE();
};

EW_LEAVE
#endif
