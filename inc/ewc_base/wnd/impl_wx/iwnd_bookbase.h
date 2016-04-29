#ifndef __UI_IWND_BOOKBASE__
#define __UI_IWND_BOOKBASE__

#include "ewc_base/wnd/impl_wx/window.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE IWnd_bookbase : public wxWindow
{
public:

	typedef wxWindow basetype;

	IWnd_bookbase(wxWindow* p,const WndPropertyEx& h);

	enum
	{
		FLAG_DYNAMIC_SIZE_X	=1<<0,
		FLAG_DYNAMIC_SIZE_Y	=1<<1,
		FLAG_FITINSIDE		=1<<2,
	};

	IWnd_bookbase(wxWindow *p,int f=0);
	~IWnd_bookbase();

	wxWindow* GetPage();
	wxWindow* GetDefault();

	bool SelPage(wxWindow *w);
	void DefPage(wxWindow *w);

	void SetMinSizeEx(const wxSize& size);
	void EnableDynamicSize(bool bflag);
	virtual wxSize DoGetBestSize() const;

	BitFlags flags;

	ValidatorHolderT<Validator> m_pVald;

protected:

	wxSize m_v2MinSize;
	LitePtrT<wxWindow> win;
	LitePtrT<wxWindow> def;

	DECLARE_EVENT_TABLE();
};

EW_LEAVE
#endif
