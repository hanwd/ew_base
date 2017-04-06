#include "ewc_base/wnd/impl_wx/iwnd_bookbase.h"


EW_ENTER

IWnd_bookbase::IWnd_bookbase(wxWindow* p,const WndPropertyEx& h)
	:wxWindow(p,h.id(),h,h)
{
	SetSizer(new ::wxBoxSizer(::wxVERTICAL));
}

IWnd_bookbase::IWnd_bookbase(wxWindow *p,int f)
	:basetype(p,wxID_ANY)
{
	flags.clr(f);
	SetSizer(new ::wxBoxSizer(::wxVERTICAL));
}

IWnd_bookbase::~IWnd_bookbase()
{
	win.reset(NULL);
	def.reset(NULL);
}

wxSize IWnd_bookbase::DoGetBestSize() const
{
	wxSize size1(basetype::DoGetBestSize());
	if(m_pVald) m_pVald->IGetBestSize(size1.x, size1.y);
	return size1;
}


wxWindow* IWnd_bookbase::GetPage()
{
	return win.get();
}

wxWindow* IWnd_bookbase::GetDefault()
{
	return def.get();
}


bool IWnd_bookbase::IAddPage(wxWindow *w, const WndProperty& wp)
{
	if (m_pVald) m_pVald->IAddPage(w, wp);
	if (!SelPage(w)) return false;
	return true;

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

	//parent->Freeze();

	wxSizer *sizer=this->GetSizer();

	if(win)
	{
		win->Show(false);
		sizer->Detach(win.get());
	}
	win=w;

	if(win)
	{
		sizer->Add(win.get(),1,wxEXPAND);
		win->SetSize(this->GetSize());
		win->Show(true);
	}

	if(m_pVald) m_pVald->ISelPage(win);

	//parent->Thaw();

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
class ValidatorW<IWnd_bookbase> : public ValidatorGroupBook
{
public:

	LitePtrT<IWnd_bookbase> pWindow;

	ValidatorW(IWnd_bookbase* w):pWindow(w)
	{
		flags = pWindow->flags;
		pWindow->m_pVald.reset(this);
	}

	wxWindow* GetWindow()
	{
		return pWindow;
	}

	virtual bool DoSetValue(const String&)
	{
		return false;
	}
	virtual bool DoGetValue(String&)
	{
		return false;
	}
	virtual bool DoSetValue(int32_t)
	{
		return false;
	}
	virtual bool DoGetValue(int32_t&)
	{
		return false;
	}

	virtual bool WndExecute(IWndParam& cmd)
	{
		if (!basetype::WndExecute(cmd))
		{
			return false;
		}

		return true;
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
