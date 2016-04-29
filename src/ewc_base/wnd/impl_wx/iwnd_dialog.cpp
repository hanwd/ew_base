
#include "ewc_base/wnd/impl_wx/iwnd_dialog.h"


EW_ENTER

int make_wnd_dialog_flag(const WndPropertyEx& h)
{
	
	BitFlags flags(wxRESIZE_BORDER|wxDEFAULT_DIALOG_STYLE);
	if(h.flags().get(IDefs::IWND_NO_RESIZABLE)) flags.del(wxRESIZE_BORDER);
	if(h.flags().get(IDefs::IWND_NO_CAPTION)) flags.del(wxCAPTION);
	if(h.flags().get(IDefs::IWND_NO_SYS_MENU)) flags.del(wxSYSTEM_MENU);
	if(h.flags().get(IDefs::IWND_NO_CLOSE_BOX)) flags.del(wxCLOSE_BOX);

	return flags.val();
}



IWnd_dialog::IWnd_dialog(wxWindow* w,const WndPropertyEx& h)
	:IWnd_topwindow<wxDialog>(w,h,make_wnd_dialog_flag(h))
{

}

template<> 
class ValidatorW<IWnd_dialog> : public ValidatorTop
{
public:
	LitePtrT<IWnd_dialog> pWindow;
	ValidatorW(IWnd_dialog* w)
		:ValidatorTop(w)
		,pWindow(w)
	{
		pWindow->SetValidatorTop(this);
	}
};

template<>
class WndInfoT<IWnd_dialog> : public WndInfoBaseT<IWnd_dialog> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_dialog>(s)
	{

	}

	Validator* CreateValidator(wxWindow* w)
	{
		return new ValidatorW<IWnd_dialog>((IWnd_dialog*)w);
	}

	void Update(WndMaker& wk,wxWindow* pw,EvtBase* pv)
	{
		wk.vd_set(CreateValidatorEx(pw,pv));
	}

};


template<>
void WndInfoManger_Register<IWnd_dialog>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_dialog> info(name);
	imgr.Register(&info);
}



EW_LEAVE

