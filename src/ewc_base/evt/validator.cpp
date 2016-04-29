#include "ewc_base/evt/validator.h"
#include "ewc_base/evt/evt_base.h"
#include "wx/window.h"

EW_ENTER


Validator::Validator()
{

}


Validator::~Validator()
{

}

bool Validator::WndExecuteEx(int action,int p1,int p2)
{
	IWndParam cmd(this,action,p1,p2);
	return WndExecute(cmd);
}


bool Validator::DoSetLabel(const String&){return false;}
bool Validator::DoGetLabel(String&){return false;}
bool Validator::DoSetValue(const String&){return false;}
bool Validator::DoGetValue(String&){return false;}
bool Validator::DoSetValue(int32_t){return false;}
bool Validator::DoGetValue(int32_t&){return false;}

bool Validator::DoSetValue(const Variant& v)
{
	return false;
}
bool Validator::DoGetValue(Variant& v)
{
	return false;
}

bool Validator::DoSetValue(double){return false;}
bool Validator::DoGetValue(double&){return false;}


bool Validator::OnWndEvent(IWndParam&,int)
{
	return true;
}

bool Validator::WndExecute(IWndParam& cmd)
{
	if(cmd.action==IDefs::ACTION_UPDATECTRL)
	{
		OnWndEvent(cmd,0);
		return true;
	}
	else
	{
		return DoWndExecute(cmd);
	}
}

bool Validator::OnUpdateWindow(wxWindow* pwin,EvtBase* proxy)
{
	bool _bEnable	=!proxy->flags.get(EvtBase::FLAG_DISABLE);
	bool _bShow		=!proxy->flags.get(EvtBase::FLAG_HIDE_UI);
	pwin->Enable(_bEnable);
	pwin->Show(_bShow);
	return true;
}


bool Validator::OnWndUpdate(IWndParam& cmd,EvtBase* proxy)
{
	if(cmd.iwvptr==NULL)
	{
		cmd.iwvptr=this;
	}
	return true;
}

EW_LEAVE
