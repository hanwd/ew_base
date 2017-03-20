
#include "ewc_base/wnd/impl_wx/window.h"
#include "ewc_base/wnd/wnd_info.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE IWnd_stc;

void WndInfo::Update(WndMaker& wk,wxWindow* pw,EvtBase* pv)
{
	if(pv)
	{
		Validator* vp=pv->CreateValidator(pw);
		wk.vd_set(vp);
	}
	else
	{
		wk.vd_set(NULL);
	}
}

WndInfoManger::WndInfoManger()
{

	WndInfoManger_Register<IWnd_dialog>(*this,"dialog");
	WndInfoManger_Register<IWnd_frame>(*this,"frame");
	WndInfoManger_Register<IWnd_container>(*this,"container");
	WndInfoManger_Register<IWnd_bookbase>(*this,"bookbase");

	WndInfoManger_Register<IWnd_button>(*this,"button");
	WndInfoManger_Register<IWnd_label>(*this,"label");
	WndInfoManger_Register<IWnd_textctrl>(*this,"textctrl");
	WndInfoManger_Register<IWnd_stc>(*this,"stc");
	WndInfoManger_Register<IWnd_password>(*this,"password");
	WndInfoManger_Register<IWnd_search>(*this,"searchctrl");

	WndInfoManger_Register<IWnd_notebook>(*this,"notebook");
	WndInfoManger_Register<IWnd_checkbox>(*this,"checkbox");
	WndInfoManger_Register<IWnd_hline>(*this,"hline");
	WndInfoManger_Register<IWnd_vline>(*this,"vline");
	WndInfoManger_Register<IWnd_cmdlink_button>(*this,"commandlink");
	WndInfoManger_Register<IWnd_gauge>(*this,"gauge");
	WndInfoManger_Register<IWnd_slider>(*this,"slider");
	WndInfoManger_Register<IWnd_spinctrl>(*this,"spin");
	WndInfoManger_Register<IWnd_hyperlink>(*this,"hyperlink");
	WndInfoManger_Register<IWnd_datectrl>(*this,"date");
	WndInfoManger_Register<IWnd_timectrl>(*this,"time");
	WndInfoManger_Register<IWnd_bitmap>(*this,"bitmap");
	WndInfoManger_Register<IWnd_combo>(*this,"combo");
	WndInfoManger_Register<IWnd_dataview>(*this,"dataview");

}


WndInfoManger& WndInfoManger::current()
{
	static WndInfoManger gInstance;
	return gInstance;
}

void WndInfoManger::Register(WndInfo* wi)
{
	aValue[wi->GetType()].reset(wi);
}

wxWindow* WndInfoManger::Create(const String& s,wxWindow* w,const WndProperty& h)
{
	if(!aValue[s]) return NULL;
	const WndPropertyEx& x((const WndPropertyEx&)h);

	wxWindow* p= aValue[s]->Create(w,x);
	if(p)
	{
		aVptrTable[*(void**)p]=aValue[s];
		if(x.name()!="")
		{
			p->SetName(x.name());
		}
		if(x.tooltip()!="")
		{
			p->SetToolTip(x.tooltip());
		}
	}
	return p;
}

WndInfo* WndInfoManger::GetWndInfo(wxWindow* w)
{
	if(!w) return NULL;
	return aVptrTable[*(void**)w];
}



EW_LEAVE
