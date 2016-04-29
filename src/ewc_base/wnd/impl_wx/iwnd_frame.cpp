#include "ewc_base/wnd/impl_wx/iwnd_frame.h"
#include "ewc_base/wnd/wnd_updator.h"

EW_ENTER

int make_wnd_frame_flag(const WndPropertyEx& h)
{
	
	BitFlags flags(wxDEFAULT_FRAME_STYLE);
	if(h.flags().get(IDefs::IWND_NO_RESIZABLE)) flags.del(wxRESIZE_BORDER);
	if(h.flags().get(IDefs::IWND_NO_CAPTION)) flags.del(wxCAPTION);
	if(h.flags().get(IDefs::IWND_NO_SYS_MENU)) flags.del(wxSYSTEM_MENU);
	if(h.flags().get(IDefs::IWND_NO_CLOSE_BOX)) flags.del(wxCLOSE_BOX);

	return flags.val();
}


IWnd_frame::IWnd_frame(wxWindow* w,const WndPropertyEx& h):basetype(w,h,make_wnd_frame_flag(h))
{
	this->Connect(wxEVT_AUI_PANE_CLOSE,wxAuiManagerEventHandler(IWnd_frame::OnPaneClose));
}

void IWnd_frame::OnPaneClose(wxAuiManagerEvent& evt)
{
	wxAuiPaneInfo* pPane=evt.GetPane();
	if(!pPane) return;
	String s=pPane->name.c_str().AsChar();
	EvtBase* p=WndManager::current().evtmgr.get_command(s);
	if(p)
	{
		p->flags.del(EvtCommand::FLAG_CHECKED);
		p->UpdateCtrl();
	}
}

bool IWnd_frame::UpdateModel()
{
	if(!basetype::UpdateModel())
	{
		return false;
	}

	return true;
}

IWnd_frame::~IWnd_frame()
{

}


template<> 
class ValidatorW<IWnd_frame> : public ValidatorTop
{
public:
	LitePtrT<IWnd_frame> pWindow;
	AutoPtrT<wxMenuBar> pMenuBar;
	String sMenuBar;

	ValidatorW(IWnd_frame* w)
		:ValidatorTop(w)
		,pWindow(w)
	{
		pWindow->SetValidatorTop(this);
	}

	void OnChildWindow(wxWindow* w,int a)
	{
		wxAuiManager& amgr(pWindow->amgr);

		if(!w)
		{
			amgr.Update();
			return;
		}

		if(a==IDefs::WND_ATTACH)
		{
			wxAuiPaneInfo  info;
			info.Name(w->GetName()).Caption(w->GetName()).Show(w->IsShown());

			if(w->GetName()=="Centerpane")
			{
				amgr.AddPane(w,wxAuiPaneInfo().CenterPane());
			}
			else if(dynamic_cast<wxToolBar*>(w))
			{
				amgr.AddPane(w,info.ToolbarPane().Top());
			}
			else if(dynamic_cast<wxStatusBar*>(w))
			{
				pWindow->SetStatusBar(static_cast<wxStatusBar*>(w));
			}
			else if(wxMenuBar* mb=dynamic_cast<wxMenuBar*>(w))
			{
				if(mb->IsShown())
				{
					pWindow->SetMenuBar(mb);
					pMenuBar.reset(NULL);
				}
				else
				{
					pWindow->SetMenuBar(NULL);
					pMenuBar.reset(mb);
				}
			}
			else
			{
				info.PinButton(true);
				info.MinSize(w->GetMinSize());
				info.BestSize(w->GetSize());

				wxSize sz=w->GetSize();
				if(size_t(sz.y)>size_t(sz.x))
				{
					amgr.AddPane(w,info.Left().Layer(1));
				}
				else
				{
					amgr.AddPane(w,info.Bottom());
				}
			}
			WndUpdator::current().update();
		}
		else if(a==IDefs::WND_DETACH)
		{
			amgr.DetachPane(w);
			WndUpdator::current().update();
		}
		else if(a==IDefs::WND_SHOW)
		{
			if(wxMenuBar* mb=dynamic_cast<wxMenuBar*>(w))
			{
				pWindow->SetMenuBar(pMenuBar.release());
			}
			else if(dynamic_cast<wxStatusBar*>(w))
			{
				w->Show(true);
			}
			else
			{
				amgr.GetPane(w).Show(true);
			}
			WndUpdator::current().update();

		}
		else if(a==IDefs::WND_HIDE)
		{
			if(wxMenuBar* mb=dynamic_cast<wxMenuBar*>(w))
			{
				pMenuBar.reset(mb);
				pWindow->SetMenuBar(NULL);
			}
			else if(dynamic_cast<wxStatusBar*>(w))
			{
				w->Show(false);
			}
			else
			{
				amgr.GetPane(w).Show(false);
			}
			WndUpdator::current().update();
		}
	}


	virtual bool mu_set(const String& s_)
	{
		if(!pWindow)
		{
			return false;
		}

		arr_1t<EvtItem> a;

		String s=s_==""?sMenuBar:s_;
		EvtManager::current().item_get(s,a);

		if(a.empty()) return false;
		sMenuBar=s;

		EvtCtrlGroup m;
		m.gp_add(a.begin(),a.end());

		if(m==pWindow->aMenus && s_!="")
		{
			return true;
		}

		pWindow->aMenus=m;	
		size_t nc;

		wxMenuBar* mb=pMenuBar?pMenuBar.get():pWindow->GetMenuBar();
		if(!mb)
		{
			if(m.empty()) return true;

			mb=new wxMenuBar;
			pWindow->SetMenuBar(mb);
			nc=0;
		}
		else
		{
			nc=mb->GetMenuCount();
		}
		
		size_t ni=0;
		for(size_t i=0;i<m.size();i++)
		{
			EvtGroup* vp=m[i];
			wxMenu* mu=m[i]->CreateMenu();
			if(!mu)
			{
				System::LogTrace("%s CreateMenu failed",m[i]->m_sId);
				continue;
			}

			String lb=m[i]->MakeLabel(EvtCommand::LABEL_MENUBAR);
			if(ni<nc)
			{
				delete mb->Replace(ni,mu,lb.c_str());
			}
			else
			{
				mb->Append(mu,lb.c_str());
			}
			ni++;
		}

		for(size_t i=ni;i<nc;i++)
		{
			delete mb->Remove(ni);
		}

		return true;
	}
};


template<>
class WndInfoT<IWnd_frame> : public WndInfoBaseT<IWnd_frame> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_frame>(s)
	{

	}

	Validator* CreateValidator(wxWindow* w)
	{
		return new ValidatorW<IWnd_frame>((IWnd_frame*)w);
	}

	void Update(WndMaker& wk,wxWindow* pw,EvtBase* pv)
	{
		wk.vd_set(CreateValidatorEx(pw,pv));
	}

};


template<>
void WndInfoManger_Register<IWnd_frame>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_frame> info(name);
	imgr.Register(&info);
}


EW_LEAVE
