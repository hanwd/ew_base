#ifndef __UI_IWND_WEBVIEW__
#define __UI_IWND_WEBVIEW__

#include "ewc_base/wnd/impl_wx/window.h"

class wxWebView;
EW_ENTER


class DLLIMPEXP_EWC_BASE IWnd_webview : public wxWindow
{
public:
	IWnd_webview(wxWindow* p,const WndPropertyEx& h);

	void RunScript(const String& s);

	bool LoadURL(const String& s);

	CmdProc* GetCmdProc(){return m_pCmdProc.get();}

	bool Invoke(Variant& ret,const String& fn,const arr_1t<Variant>& pm);
	bool FindObj(const String& fn);

	bool Invoke(Variant& ret,const String& fn)
	{
		arr_1t<Variant> apm;
		return Invoke(ret,fn,apm);
	}
	bool Invoke(Variant& ret,const String& fn,const Variant& v)
	{
		arr_1t<Variant> apm;apm.push_back(v);
		return Invoke(ret,fn,apm);
	}

	String GetURL();

public:
	wxWebView* m_pImpl;
	DataPtrT<CmdProc> m_pCmdProc;
};

EW_LEAVE

#endif

