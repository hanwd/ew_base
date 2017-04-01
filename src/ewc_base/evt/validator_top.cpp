#include "ewc_base/evt/validator_top.h"
#include "ewc_base/wnd/impl_wx/topwindow.h"

EW_ENTER

wxTopLevelWindow* win2top(IWindowPtr w)
{
	return dynamic_cast<wxTopLevelWindow*>(w);
}

ValidatorTop::ValidatorTop(IWindowPtr w):pWindow(w)
{

}

ValidatorTop::~ValidatorTop()
{

}

bool ValidatorTop::DoSetLabel(const String& v)
{
	wxTopLevelWindow* pw=win2top(pWindow);
	pw->SetLabel(str2wx(v));
	return true;
}

bool ValidatorTop::DoGetLabel(String& v)
{
	wxTopLevelWindow* pw=win2top(pWindow);
	if(!pw) return false;
	v=wx2str(pw->GetTitle());
	return true;
}


void ValidatorTop::Close()
{
	wxTopLevelWindow* pw=win2top(pWindow);
	if(pw) pw->Close();
}

bool ValidatorTop::IsShown()
{
	wxTopLevelWindow* pw=win2top(pWindow);
	return pw && pw->IsShown();
}


bool ValidatorTop::Show(bool f)
{
	wxTopLevelWindow* pw=win2top(pWindow);
	if(!pw)
	{
		return false;
	}
	return pw->Show(f);
}

int ValidatorTop::ShowModal()
{
	wxTopLevelWindow* pw=win2top(pWindow);
	wxDialog* dlg=dynamic_cast<wxDialog*>(pw);
	if(!dlg)
	{
		return IDefs::BTN_CANCEL;
	}

	int ret=dlg->ShowModal();
	return Wrapper::wxid_to_btn(ret);

}

void ValidatorTop::EndModal(int h)
{
	if (h == 0) h = IDefs::BTN_CANCEL;

	wxTopLevelWindow* pw=win2top(pWindow);
	wxDialog* dlg=dynamic_cast<wxDialog*>(pw);
	if(!dlg)
	{
		return;
	}

	int ret = Wrapper::btn_to_wxid(h);
	dlg->EndModal(ret);
}

void ValidatorTop::OnChildWindow(IWindowPtr w,int a)
{

}

void ValidatorTop::SetModel(WndModel* pwm)
{
	DataPtrT<ObjectData> lock(this);
	m_pModel.reset(pwm);
}

WndModel* ValidatorTop::GetModel()
{
	return m_pModel.get();
}

EW_LEAVE
