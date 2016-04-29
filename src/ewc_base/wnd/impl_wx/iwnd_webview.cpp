#include "ewc_base/wnd/impl_wx/iwnd_webview.h"

#include <wx/webview.h>
#include <wx/uri.h>

EW_ENTER



class ICmdProcWebView : public CmdProc
{
public:
	typedef CmdProc basetype;

	wxWebView& Target;
	IDat_search& data;

	ICmdProcWebView(wxWebView& t):Target(t),data(IDat_search::current()){}

	bool DoFind(ICmdParam&)
	{
		int fg=wxWEBVIEW_FIND_HIGHLIGHT_RESULT;
		if(data.flags.get(2))
		{
			fg|=wxWEBVIEW_FIND_ENTIRE_WORD;
		}
		if(data.flags.get(4))
		{
			fg|=wxWEBVIEW_FIND_MATCH_CASE;
		}

		long pos=Target.Find(data.text_old.c_str(),fg);
		if(pos<0)
		{
			Wrapper::MsgsDialog("no more results",0);
		}

		return pos>=0;
	}

	bool DoExecId(ICmdParam& cmd)
	{
		switch(cmd.param1)
		{
		case CP_CUT:
			Target.Cut();
			break;
		case CP_COPY:
			Target.Copy();
			break;
		case CP_PASTE:
			Target.Paste();
			break;
		case CP_REDO:
			Target.Redo();
			break;
		case CP_UNDO:
			Target.Undo();
			break;
		case CP_DIRTY:
			return false;
			break;
		case CP_SAVE_TEMP:
			return true;
		case CP_SAVE_FILE:
			if(cmd.extra=="")
			{					
				return false;
			}
			else
			{
				File fp;
				if(!fp.Open(cmd.extra,FileAccess::FLAG_CR|FileAccess::FLAG_WR))
				{
					return false;
				}
				String pagesrc=wx2str(Target.GetPageSource());
				int ret=fp.Write(pagesrc.c_str(),pagesrc.size());
				if(ret<0) return false;

				return true;
			}
			break;
		case CP_DELETE:			
			break;
		case CP_SELECTALL:
			Target.SelectAll();
			break;
		case CP_FIND:
			if(data.text_old=="") return false;
			return DoFind(cmd);
		default:
			return basetype::DoExecId(cmd);
		}
		return true;
	}

	bool DoTestId(ICmdParam& cmd)
	{
		switch(cmd.param1)
		{
		case CP_CUT:
			return Target.CanCut();
			break;
		case CP_COPY:
			return Target.CanCopy();
			break;
		case CP_PASTE:
			return Target.CanPaste();
			break;
		case CP_REDO:
			return Target.CanRedo();
			break;
		case CP_UNDO:
			return Target.CanUndo();
			break;
		case CP_DIRTY:
			return false;
			break;
		case CP_SELECTALL:
			return true;
			break;
		case CP_DELETE:	
			return Target.CanCut();
			break;
		case CP_SAVEAS:
			return false;
		case CP_FILEEXT:
			cmd.extra=_hT("Html Files")+"(*.html)|*.html";
			return true;
		case CP_FIND:
			return true;
		default:
			return basetype::DoTestId(cmd);
		}
		return false;
	}

};


class IWnd_webview_impl : public wxWebView
{
public:

	//IWnd_webview_impl(wxWindow* p,const WndPropertyEx& h);


	void OnNewWindow(wxWebViewEvent& evt)
	{
		wxString s=evt.GetURL();

		s=s;

	}
	void OnNavigating(wxWebViewEvent& evt)
	{
		wxString s=evt.GetURL();

		s=s;

	}

	void OnNavigated(wxWebViewEvent& evt)
	{
		wxString s=evt.GetURL();

		s=s;

	}

	void OnLoaded(wxWebViewEvent& evt)
	{
		wxString s=evt.GetURL();

		s=s;

	}

	void OnTitleChanged(wxWebViewEvent& evt)
	{
		wxString s=evt.GetURL();

		s=s;

	}

	void OnError(wxWebViewEvent& evt)
	{
		wxString s=evt.GetURL();

		s=s;

	}

};

//wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_WEBVIEW, wxEVT_WEBVIEW_NAVIGATING, wxWebViewEvent );
//wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_WEBVIEW, wxEVT_WEBVIEW_NAVIGATED, wxWebViewEvent );
//wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_WEBVIEW, wxEVT_WEBVIEW_LOADED, wxWebViewEvent );
//wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_WEBVIEW, wxEVT_WEBVIEW_ERROR, wxWebViewEvent );
//wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_WEBVIEW, wxEVT_WEBVIEW_NEWWINDOW, wxWebViewEvent );
//wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_WEBVIEW, wxEVT_WEBVIEW_TITLE_CHANGED, wxWebViewEvent );

IWnd_webview::IWnd_webview(wxWindow* p,const WndPropertyEx& h)
	:wxWindow(p,h.id(),h,h)
{
	m_pImpl=wxWebView::New(this,wxID_ANY);
	m_pCmdProc.reset(new ICmdProcWebView(*m_pImpl));

	wxSizer* sz=new wxBoxSizer(wxVERTICAL);
	sz->Add(m_pImpl,1,wxEXPAND);	
	SetSizer(sz);

	m_pImpl->Connect(wxEVT_WEBVIEW_NEWWINDOW,wxWebViewEventHandler(IWnd_webview_impl::OnNewWindow));
	m_pImpl->Connect(wxEVT_WEBVIEW_NAVIGATING,wxWebViewEventHandler(IWnd_webview_impl::OnNavigating));
	m_pImpl->Connect(wxEVT_WEBVIEW_NAVIGATED,wxWebViewEventHandler(IWnd_webview_impl::OnNavigated));
	m_pImpl->Connect(wxEVT_WEBVIEW_LOADED,wxWebViewEventHandler(IWnd_webview_impl::OnLoaded));
	m_pImpl->Connect(wxEVT_WEBVIEW_TITLE_CHANGED,wxWebViewEventHandler(IWnd_webview_impl::OnTitleChanged));
	m_pImpl->Connect(wxEVT_WEBVIEW_ERROR,wxWebViewEventHandler(IWnd_webview_impl::OnError));

}

void IWnd_webview::RunScript(const String& s)
{
	m_pImpl->RunScript(s.c_str());
	return;
}

bool IWnd_webview::LoadURL(const String& s)
{
	m_pImpl->LoadURL(s.c_str());
	return true;
}

String IWnd_webview::GetURL()
{
	return wx2str(m_pImpl->GetCurrentURL());
}

template<> 
class ValidatorW<IWnd_webview> : public Validator
{
public:
	LitePtrT<IWnd_webview> pWindow;
	ValidatorW(IWnd_webview* w):pWindow(w){}
};


template<>
class WndInfoT<IWnd_webview> : public WndInfoBaseT<IWnd_webview> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_webview>(s)
	{

	}

	Validator* CreateValidator(wxWindow* w)
	{
		return new ValidatorW<IWnd_webview>((IWnd_webview*)w);
	}

	void Update(WndMaker& wk,wxWindow* pw,EvtBase* pv)
	{
		wk.vd_set(CreateValidatorEx(pw,pv));
	}

};

template<>
void WndInfoManger_Register<IWnd_webview>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_webview> info(name);
	imgr.Register(&info);
}

EW_LEAVE
