#include "ewc_base/wnd/impl_wx/iwnd_combo.h"

EW_ENTER

IWnd_combo::IWnd_combo(wxWindow* p,const WndPropertyEx& h)
:wxOwnerDrawnComboBox(p,h.id(),h.value(),h,h,wxArrayString(),h.flag_text())
{
	this->SetHint(h.hint());
	this->Connect(wxEVT_COMBOBOX,wxCommandEventHandler(IWnd_controlT<IWnd_combo>::OnCommandIntChanged));
}



template<>
class ValidatorW<IWnd_combo> : public Validator
{
public:
	LitePtrT<IWnd_combo> pWindow;
	ValidatorW(IWnd_combo* w):pWindow(w)
	{
		pWindow->m_pVald.reset(this);
	}

	wxArrayString aOptions;

	wxArrayString MakeOption(EvtBase* vp)
	{
		wxArrayString aString;
		IValueOptionData* d=vp->GetComboArray();
		if(!d) return aString;
		for(size_t i=0;i<d->size();i++)
		{
			aString.Add(str2wx(d->item(i)));
		}
		return aString;
	}

	bool OnUpdateWindow(wxWindow* pwin,EvtBase* proxy)
	{
		if(pWindow!=pwin)
		{
			return false;
		}

		wxArrayString opt_new=MakeOption(proxy);
		if(opt_new==aOptions)
		{
			return true;
		}

		wxString val=pWindow->GetValue();
		aOptions.swap(opt_new);
		pWindow->Clear();
		pWindow->Append(aOptions);

		pWindow->ChangeValue(val);

		bool _bEnable	=!proxy->flags.get(EvtBase::FLAG_DISABLE);
		bool _bShow		=!proxy->flags.get(EvtBase::FLAG_HIDE_UI);
		pWindow->Enable(_bEnable);
		pWindow->Show(_bShow);

		return true;
	}
	
	virtual bool DoSetValue(int32_t v)
	{
		pWindow->SetSelection(v);
		return true;
	}

	virtual bool DoGetValue(int32_t& v)
	{
		v=pWindow->GetSelection();
		return true;
	}

	virtual bool DoSetValue(const String& v)
	{
		pWindow->SetValue(str2wx(v));
		return true;
	}

	virtual bool DoGetValue(String& v)
	{
		v=wx2str(pWindow->GetValue());
		return true;
	}
};


template<>
class WndInfoT<IWnd_combo> : public WndInfoBaseT<IWnd_combo> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_combo>(s)
	{

	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<int32_t>* p)
	{
		return CreateValidatorBaseT(w,p);
	}


	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<String>* p)
	{
		return CreateValidatorBaseT(w,p);
	}

	Validator* CreateValidator(wxWindow* w)
	{
		return new ValidatorW<IWnd_combo>((IWnd_combo*)w);
	}

};


template<>
void WndInfoManger_Register<IWnd_combo>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_combo> info(name);
	imgr.Register(&info);
}

EW_LEAVE
