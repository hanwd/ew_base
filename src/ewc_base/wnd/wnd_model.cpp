#include "ewc_base/wnd/wnd_model.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/wnd/wnd_info.h"
#include "ewc_base/wnd/wnd_maker.h"
#include "ewc_base/wnd/wnd_manager.h"
#include "ewc_base/evt/validator_group.h"
#include "ewc_base/evt/validator_top.h"
#include "ewc_base/plugin/plugin_manager.h"

#include "ewc_base/wnd/impl_wx/window.h"


EW_ENTER

bool WndModel::DoCmdExecute(ICmdParam&)
{
	Show(true);
	return true;
}

bool WndModel::DoStdExecute(IStdParam& cmd)
{
	if (cmd.param1 == 2)
	{
		cmd.param2=ShowModal();
		return true;
	}
	else if (cmd.param1 != 0)
	{
		Show(true);
	}
	else if (IsModal())
	{
		EndModal(cmd.param2);
		return true;
	}
	else
	{
		Show(false);

	}

	return true;
}


bool WndModel::WndExecuteEx(int action)
{
	IWndParam cmd(this,action);
	return WndExecute(cmd);
}

bool WndModel::OnCmdExecute(int evtid,int p1)
{
	if(evtid<=12000)
	{
		return local_evtmgr.OnCmdExecute(evtid,p1,this);
	}
	else
	{
		return EvtManager::current().OnCmdExecute(evtid,p1,this);
	}
}

bool WndModel::DoWndExecute(IWndParam& cmd)
{
	if(!vald_grp)
	{
		return true;
	}

	if(cmd.action==IDefs::ACTION_APPLY)
	{
		return WndExecuteEx(IDefs::ACTION_TRANSFER2MODEL);
	}

	if(cmd.action==IDefs::ACTION_WINDOW_BEFORE_SHOW)
	{
		vald_grp->WndExecuteEx(IDefs::ACTION_TRANSFER2WINDOW);
		UpdateCtrl();
	}

	if(cmd.action!=IDefs::ACTION_WINDOW_FINI)
	{
		return vald_grp->WndExecute(cmd);
	}

	if(flags.get(FLAG_NO_CLOSE) && !App::TestDestroy())
	{
		WndExecuteEx(IDefs::ACTION_TRANSFER2MODEL);
		Show(false);
		return false;
	}
	else
	{
		bool flag = true;
		if(this==&WndModel::current())
		{
			flag=WndManager::current().evtmgr["CloseFrame"].CmdExecuteEx(-1);
		}

		if (flag)
		{
			DataPtrT<WndModel> lock(this);
			vald_top.reset(NULL);
			vald_grp.reset(NULL);
		}

		return flag;
	}


}

void WndModel::UpdateCtrl()
{
	IUpdParam upd;
	DoUpdateCtrl(upd);
}

void WndModel::DoUpdateCtrl(IUpdParam&)
{
	if(vald_grp) vald_grp->WndExecuteEx(IDefs::ACTION_UPDATECTRL);
	if(flags.get(FLAG_AUTO_FIT))
	{
		wxWindow* p=GetWindow();
		if(p) p->Fit();
	}
}

wxWindow* WndModel::GetWindow()
{
	if(!vald_top) return m_pWindow;
	return vald_top->pWindow;
}

void WndModel::OnChildWindow(wxWindow* w,int a)
{
	if(!vald_top)
	{
		System::LogTrace("WndModel::OnChildWindow: vald_top is NULL!");
		return;
	}
	vald_top->OnChildWindow(w,a);
}


void WndModel::SetSize(int w,int h)
{
	if(vald_top)
	{
		vald_top->pWindow->SetSize(w,h);
	}
}


bool WndModel::EnsureCreated(bool t)
{
	if(!vald_grp && !flags.get(FLAG_CREATED))
	{
		flags.add(FLAG_CREATED);
		Create();
	}

	if(!vald_grp)
	{
		return false;
	}

	return !t||vald_top;
}

void WndModel::EndModal()
{
	EndModal(IDefs::BTN_OK);
}

void WndModel::EndModal(int h)
{

	if(!EnsureCreated(true))
	{
		return;
	}
	
	if(!vald_top->IsShown())
	{
		return;
	}
	vald_top->EndModal(h);
}

int WndModel::ShowModal()
{
	if(!EnsureCreated(true))
	{
		return IDefs::BTN_CANCEL;
	}

	if (vald_top->IsShown())
	{
		vald_top->Show(false);
	}

	return vald_top->ShowModal();

}

bool WndModel::IsModal()
{
	wxDialog* d=dynamic_cast<wxDialog*>(GetWindow());
	return d && d->IsModal();
}

bool WndModel::IsShown()
{
	wxWindow* w=GetWindow();
	return w && w->IsShown();
}

bool WndModel::Show(bool f)
{
	if(!EnsureCreated(true))
	{
		return false;
	}

	flags.set(FLAG_CHECKED,f);

	if(f)
	{
		UpdateCtrl();	
	}

	if(!f && IsModal())
	{
		EndModal(IDefs::BTN_CANCEL);
		return true;
	}

	return vald_top->Show(f);

}

void WndModel::Close()
{
	if(!EnsureCreated(true))
	{
		return;
	}

	vald_top->Close();

}


WndModel::WndModel(const String& s):basetype(s),local_evtmgr(wxID_HIGHEST)
{
	local_evtmgr.m_pNextEvtManager=&EvtManager::current();
}

WndModel::~WndModel()
{
	if(vald_top)
	{
		vald_top->Close();
	}
}



EvtBase* WndModel::get_item(const String& s)
{
	if(s=="") return NULL;
	EvtBase* vp=local_evtmgr.get(s);
	if(vp) return vp;

	return EvtManager::current().get(s);	
}

bool WndModel::DoUpdate(WndMaker& km)
{
	wxWindow* w=km.get();
	if(!w) return false;

	DataPtrT<ObjectData> lock(this);
	if(vald_top)
	{
		vald_top->Close();
		vald_top.reset(NULL);
	}

	vald_top.reset(dynamic_cast<ValidatorTop*>(km.vald.get()));
	vald_grp.reset(km.vald.get());

	if(!vald_grp)
	{
		vald_grp.reset(new ValidatorGroup);
	}

	if(vald_top)
	{
		vald_top->SetModel(this);
	}

	vald_grp->WndExecuteEx(IDefs::ACTION_TRANSFER2WINDOW);

	//for(size_t i=0;i<km.aEvtArray.size();i++)
	//{
	//	this->AttachEvent(km.aEvtArray[i]);
	//}

	if(vald_top)
	{
		static_cast<wxTopLevelWindow*>(vald_top->pWindow)->CenterOnScreen();
	}

	flags.add(FLAG_CREATED);

	Show(flags.get(EvtBase::FLAG_CHECKED));
	return true;
}


bool WndModel::mu_set(const String& a)
{
	return vald_top && vald_top->mu_set(a);	
}

void WndModel::SetLabel(const String& s)
{
	if(vald_top) vald_top->DoSetLabel(s);
}

bool WndModelTop::Create()
{
	WndMaker km(this);
	km.add("frame",	km.ld(0).label(m_sId).width(800).height(640).sprops("mgr","aui"));
	return true;
}

WndModelScript::WndModelScript(const String& s,const String& f,int h):WndModel(s),m_sFilename(f)
{
	flags.add(h);
}

bool WndModelScript::Create()
{
	Executor ewsl;

	ewsl.push(new CallableWndModelPtr(this));
	return ewsl.execute_file(m_sFilename,1);
}

void WndModel::AttachEvent(const String& s)
{
	this->basetype::AttachEvent(local_evtmgr.get(s));
}

void WndModel::DetachEvent(const String& s)
{
	this->basetype::DetachEvent(local_evtmgr.get(s));
}


EW_LEAVE
