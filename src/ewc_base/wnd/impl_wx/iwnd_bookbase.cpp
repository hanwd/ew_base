#include "ewc_base/wnd/impl_wx/iwnd_bookbase.h"


EW_ENTER

IWnd_bookbase::IWnd_bookbase(wxWindow* p,const WndPropertyEx& h)
	:wxWindow(p,h.id(),h,h)
{
	SetSizer(new ::wxBoxSizer(::wxVERTICAL));
}

IWnd_bookbase::IWnd_bookbase(wxWindow *p,int f)
	:basetype(p,wxID_ANY)
	,flags(f)
{
	SetSizer(new ::wxBoxSizer(::wxVERTICAL));
}

IWnd_bookbase::~IWnd_bookbase()
{
	win.reset(NULL);
	def.reset(NULL);
}
void IWnd_bookbase::SetMinSizeEx(const wxSize& sz)
{
	m_v2MinSize=sz;	
}

wxSize IWnd_bookbase::DoGetBestSize() const
{
	const wxWindowList &list(this->GetChildren());
	wxSize size1(basetype::DoGetBestSize());

	if(win) size1=win->GetBestSize();

	wxSize size2(size1);
	for(wxWindowList::const_iterator it=list.begin();it!=list.end();++it)
	{
		wxSize _wsz=(*it)->GetBestSize();
		if(_wsz.x>size2.x) size2.x=_wsz.x;
		if(_wsz.y>size2.y) size2.y=_wsz.y;
	}

	if(!flags.get(FLAG_DYNAMIC_SIZE_X))
	{
		size1.x=std::max(size1.x,size2.x);
	}

	if(!flags.get(FLAG_DYNAMIC_SIZE_Y))
	{
		size1.y=std::max(size1.y,size2.y);
	}

	if(size1.x<m_v2MinSize.x) size1.x=m_v2MinSize.x;
	if(size1.y<m_v2MinSize.y) size1.y=m_v2MinSize.y;

	return size1;

}

void IWnd_bookbase::EnableDynamicSize(bool bflag)
{
	flags.set(FLAG_DYNAMIC_SIZE_X|FLAG_DYNAMIC_SIZE_Y,bflag);
}

wxWindow* IWnd_bookbase::GetPage()
{
	return win.get();
}

wxWindow* IWnd_bookbase::GetDefault()
{
	return def.get();
}



bool IWnd_bookbase::SelPage(wxWindow *w)
{
	EW_ASSERT(!w||w->GetParent()==this);

	if(w==NULL)
	{
		w=def;
	}

	if(w==win)
	{
		return false;
	}

	wxWindow* parent=GetParent();

	parent->Freeze();

	wxSizer *sizer=this->GetSizer();

	if(win)
	{
		sizer->Detach(win.get());
		win->Show(false);
	}
	win=w;

	if(win)
	{
		sizer->Add(win.get(),1,wxEXPAND);
		win->Show(true);

		if(flags.get(FLAG_DYNAMIC_SIZE_X|FLAG_DYNAMIC_SIZE_Y))
		{
			this->Fit();
			parent->Fit();
			parent->Layout();
			parent->Refresh();
		}
		else if(flags.get(FLAG_FITINSIDE))
		{
			this->FitInside();
		}
		else
		{
			this->Fit();
			parent->Layout();
			parent->Refresh();
		}

	}

	parent->Thaw();

	return true;

}

void IWnd_bookbase::DefPage(wxWindow *w)
{
	if(def) 
	{
		def->Destroy();
	}

	def=w;

	if(def)
	{
		def->Show(false);
	}

	if(!win)
	{
		SelPage(NULL);
	}

}




BEGIN_EVENT_TABLE(IWnd_bookbase,basetype)

END_EVENT_TABLE()


template<> 
class ValidatorW<IWnd_bookbase> : public ValidatorGroup
{
public:
	LitePtrT<IWnd_bookbase> pWindow;
	ValidatorW(IWnd_bookbase* w):pWindow(w)
	{
		pWindow->m_pVald.reset(this);
	}
};


template<>
class WndInfoT<IWnd_bookbase> : public WndInfoBaseT<IWnd_bookbase> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_bookbase>(s)
	{

	}

	Validator* CreateValidator(wxWindow* w)
	{
		return new ValidatorW<IWnd_bookbase>((IWnd_bookbase*)w);
	}

	void Update(WndMaker& wk,wxWindow* pw,EvtBase* pv)
	{
		wk.vd_set(CreateValidatorEx(pw,pv));
	}

};

template<>
void WndInfoManger_Register<IWnd_bookbase>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_bookbase> info(name);
	imgr.Register(&info);
}

EW_LEAVE
