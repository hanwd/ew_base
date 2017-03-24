#include "ewc_base/evt/evt_command.h"
#include "evt_ctrlimpl.h"

#include "ewc_base/evt/validator.h"
#include "ewc_base/app/res_manager.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/wnd/wnd_model.h"
#include "ewc_base/wnd/wnd_manager.h"
#include "ewc_base/wnd/wnd_updator.h"

EW_ENTER


IEW_CtrlData::IEW_CtrlData(EvtCommand* p) :pevt(p)
{
	pevt->m_setAttachedControls.insert(this);
}

IEW_CtrlData::~IEW_CtrlData()
{
	pevt->m_setAttachedControls.erase(this);
}

bool IEW_Ctrl::AddCtrlItem(EvtCommand*)
{
	return false;
}

bool IEW_Ctrl::AddCtrlItem(EvtCommand*, wxControl*)
{
	return false;
}

bool IEW_Ctrl::AddCtrlItem(EvtGroup*)
{
	return false;
}




const BitmapBundle& EvtCommand::GetBundle(int w,int t)
{
	if (!m_bmpParam)
	{
		m_bmpParam=ResManager::current().icons.get(m_sId);
	}
	return m_bmpParam.GetBundle(w,t);
}

bool EvtCommandFunctor::CmdExecute(ICmdParam& cmd)
{
	if(!func)
	{
		return false;
	}

	return func(cmd);
}

bool EvtCommand::DoCmdExecute(ICmdParam& cmd)
{
	if (flags.get(FLAG_CHECK))
	{
		flags.set(FLAG_CHECKED, cmd.param1 != 0);
	}
	return true;
}

String EvtCommand::MakeLabel(int hint) const
{
	String txt=m_sLabel;

	if(txt=="")
	{
		txt=m_sText;
	}

	if(txt=="")
	{
		txt=m_sId;
	}

	if((hint==LABEL_MENU||hint==LABEL_MENUBAR) && m_sHotkey!="")
	{
		txt=txt+"(&"+m_sHotkey+")";
	}

	if(hint==LABEL_MENUBAR)
	{
		return txt;
	}

	if(m_sExtra!="")
	{
		txt=txt+" "+m_sExtra;
	}

	if(flags.get(FLAG_DOTDOT))
	{
		txt+=(" ..");
	}

	if(hint==LABEL_MENU && m_sAccel!="")
	{
		txt+="\t"+m_sAccel;
	}

	return txt;
}

void EvtCommand::CreateCtrlItem(IEW_Ctrl* pctrl)
{
	pctrl->AddCtrlItem(this);
}

void EvtCommand::DoUpdateCtrl(IUpdParam& upd)
{

	if (flags.get(FLAG_HIDE_UI))
	{
		bst_set<IEW_CtrlData*> ctrls(m_setAttachedControls);
		for (auto it = ctrls.begin(); it != ctrls.end(); it++)
		{
			(*it)->UpdateCtrl();
		}
	}
	else
	{
		bst_set<IEW_CtrlData*>& ctrls(m_setAttachedControls);
		for (auto it = ctrls.begin(); it != ctrls.end(); it++)
		{
			(*it)->UpdateCtrl();
		}
	}

	
	EvtBase::DoUpdateCtrl(upd);
}


IWindowPtr EvtCommand::CreateWndsItem(IWindowPtr pw)
{
	if(flags.get(FLAG_SEPARATOR)) return NULL;

	WndProperty wp;
	wp.id(m_nId);
	wp.label(m_sId);

	wxWindow* ctrl= WndInfoManger::current().Create("button",pw,wp);
	if(!ctrl) return NULL;

	CreateValidator(ctrl);
	return ctrl;

}

bool EvtCommandWindow::DoCmdExecute(ICmdParam& cmd)
{
	if(!m_pWindow) return true;

	if(cmd.param1>=+2)
	{
		flags.del(FLAG_DISABLE|FLAG_HIDE_UI);
	}
	else if(cmd.param1==-2)
	{
		flags.add(FLAG_DISABLE|FLAG_HIDE_UI);
	}
	else if(cmd.param1<-2)
	{
		flags.add(FLAG_DISABLE|FLAG_HIDE_UI);
	}
	else
	{
		flags.set(FLAG_CHECKED,cmd.param1!=0);
	}

	bool show=flags.get(FLAG_CHECKED)&&!flags.get(FLAG_DISABLE);

	OnWindow(show?IDefs::WND_SHOW:IDefs::WND_HIDE);

	UpdateCtrl();

	return true;
}

bool EvtCommandWindow::OnWindow(int flag)
{
	if(m_pWindow)
	{
		WndModel::current().OnChildWindow(m_pWindow,flag);
	}
	return true;
}

void EvtCommandWindow::SetWindow(wxWindow* w)
{
	if(m_pWindow)
	{
		OnWindow(IDefs::WND_DETACH);
		flags.del(FLAG_CHECK|FLAG_CHECKED);
		m_sText="";
	}
	m_pWindow=w;
	if(m_pWindow)
	{
		flags.add(FLAG_CHECK);	
		flags.set(FLAG_CHECKED,w->IsShown());
		m_sText=Translate(wx2str(w->GetName()));
		OnWindow(IDefs::WND_ATTACH);
	}
}


EvtCommandCtrl::EvtCommandCtrl(const String& id,const String& type,const WndProperty& w)
	:basetype(id)
	,m_sCtrlType(type)
	,wp(w)
{

}

bool EvtCommandCtrl::DoStdExecute(IStdParam& cmd)
{
	if(m_pWindow) return false;

	wxWindow* pw=CreateWndsItem(NULL);

	if(!pw) return false;
	if(cmd.param1>0) flags.add(cmd.param1);
	if(flags.get(FLAG_HIDE_UI)||!flags.get(FLAG_CHECKED)) pw->Hide();

	SetWindow(pw);
	return true;
}

void EvtCommandCtrl::DoUpdateCtrl(IUpdParam& upd)
{
	basetype::DoUpdateCtrl(upd);
}

EvtCommandExtraWindow::EvtCommandExtraWindow(const String& id,const String& type,int h,const WndProperty& w)
	:EvtCommandCtrl(id,type,w)
{
	StdExecuteEx(h);
}

void EvtCommandCtrl::CreateCtrlItem(IEW_Ctrl* pctrl)
{
	if(m_pWindow)
	{
		return;
	}

	wxWindow* parent=pctrl->GetWindow();
	if(!parent)
	{
		return;
	}

	//wp.height(tb->GetToolBitmapSize().y);
	wxWindow* pwin=WndInfoManger::current().Create(m_sCtrlType,parent,wp);
	if(!pwin) 
	{
		System::LogMessage("cannot create toolctrl %s",m_sCtrlType);
		return;
	}

	wxControl* ctrl=dynamic_cast<wxControl*>(pwin);
	if(!ctrl||!pctrl->AddCtrlItem(this, ctrl))
	{
		delete pwin;
	}
	else
	{
		m_pWindow=pwin;
	}



}


IWindowPtr EvtCommandCtrl::CreateWndsItem(IWindowPtr pw)
{
	if(m_pWindow)
	{
		return NULL;
	}

	if(!pw)
	{
		pw=WndModel::current().GetWindow();
	}
	
	wxWindow* pwin=WndInfoManger::current().Create(m_sCtrlType,pw,wp);
	if(!pwin) return NULL;

	pwin->SetName(str2wx(m_sText));
	CreateValidator(pwin);
	m_pWindow=pwin;
	return pwin;
}

Validator* EvtCommandCtrl::CreateValidator(wxWindow* w)
{
	WndInfo* wi=WndInfoManger::current().GetWndInfo(w);
	if(!wi) return NULL;
	return wi->CreateValidator(w,this);	
}

bool EvtCommandText::DoStdExecute(IStdParam& cmd)
{
	if(cmd.param1<=0)
	{
		value=cmd.extra1;
		return WndExecuteEx(IDefs::ACTION_TRANSFER2WINDOW);
	}
	else if(WndExecuteEx(IDefs::ACTION_TRANSFER2MODEL))
	{
		cmd.extra1=value;
		return true;
	}
	else
	{
		return false;
	}
}

bool EvtCommandText::DoWndExecute(IWndParam& cmd)
{

	Validator* pv=cmd.iwvptr;
	if(!pv)
	{
		if(m_aAttachedListeners.empty()) return true;
		pv=dynamic_cast<Validator*>(m_aAttachedListeners[0]);
		if(!pv) return false;
	}

	if(cmd.action==IDefs::ACTION_VALUE_CHANGED||cmd.action==IDefs::ACTION_TRANSFER2MODEL)
	{
		pv->DoGetValue(value);
	}
	else if(cmd.action==IDefs::ACTION_TRANSFER2WINDOW)
	{
		pv->DoSetValue(value);
	}

	return true;
}

EvtCommandWindow::EvtCommandWindow(wxWindow* pw)
	:basetype(wx2str(pw->GetName()))
{
	SetWindow(pw);
}

EvtCommandWindow::EvtCommandWindow(const String& s,wxWindow* w)
	:basetype(s)
{
	SetWindow(w);
}

IWindowPtr EvtCommandWindow::CreateWndsItem(IWindowPtr pw)
{
	return NULL;
}

void EvtCommandWindow::DoUpdateCtrl(IUpdParam& upd)
{
	EvtCommand::DoUpdateCtrl(upd);
}

EvtCommandWindowSharedBook::EvtCommandWindowSharedBook(const String& n)
	:EvtCommandWindow(n)
{
	IWindowPtr p=WndModel::current().GetWindow();
	if(p)
	{
		WndProperty wp;
		m_pWindow=new IWnd_bookbase(p,wp);
		m_pWindow->SetName(str2wx(n));
		SetWindow(m_pWindow);
	}
}

bool EvtCommandWindowSharedBook::OnWindow(int show)
{
	return EvtCommandWindow::OnWindow(show);
}


bool EvtCommandWindowSharedBook::SelPage(const String& s,wxWindow* w)
{
	EvtBase* pevt=EvtManager::current().get(s);
	EvtCommandWindowSharedBook* pevt_book=NULL;

	if(!pevt)
	{
		pevt_book=new EvtCommandWindowSharedBook(s);
		EvtManager::current().gp_beg("OtherWindow");
		EvtManager::current().gp_add(pevt_book);
		EvtManager::current().gp_end();
	}
	else
	{
		pevt_book=dynamic_cast<EvtCommandWindowSharedBook*>(pevt);
		if(!pevt_book)
		{
			return false;
		}
	}

	if(w) w->Reparent(pevt_book->m_pWindow);

	pevt_book->m_pWindow->SelPage(w);
	return true;

}

Validator* EvtCommand::CreateValidator(wxWindow* w)
{
	WndInfo* wi=WndInfoManger::current().GetWndInfo(w);
	if(!wi) return NULL;
	return wi->CreateValidator(w,this);
}

void EvtCommandShowModel::_DoSetModel(WndModel* p)
{
	if(m_pModel.get()==p) return;

	flags.add(FLAG_CHECK);
	DetachEvent(m_pModel.get());
	m_pModel.reset(p);
	AttachEvent(m_pModel.get());
}

EvtCommandShowModel::EvtCommandShowModel(const String& s,WndModel* p)
	:basetype(s)
{
	_DoSetModel(p);
}

EvtCommandShowModel::EvtCommandShowModel(const String& s,const String& p)
	:basetype(s)
{
	_DoSetModel(dynamic_cast<WndModel*>(WndManager::current().evtmgr.get(p)));
}


bool EvtCommandShowModel::OnWndEvent(IWndParam& cmd,int phase)
{
	if(phase>0 && cmd.action<=IDefs::ACTION_WINDOW_FINI)
	{
		UpdateCtrl();
	}
	return true;
}

void EvtCommandShowModel::DoUpdateCtrl(IUpdParam& upd)
{
	bool checked = m_pModel && m_pModel->IsShown();
	flags.set(FLAG_CHECKED,checked);
	basetype::DoUpdateCtrl(upd);
}

bool EvtCommandShowModel::DoCmdExecute(ICmdParam& cmd)
{
	if (m_pModel)
	{
		m_pModel->Show(cmd.param1 != 0);
	}
	return true;
}



class ITimerData : public Object, public wxTimer
{
public:
	EvtCommandTimer& Target;
	ITimerData(EvtCommandTimer& t):Target(t)
	{
		this->Connect(wxID_ANY,wxEVT_TIMER,wxTimerEventHandler(ITimerData::OnTimer));
	}
	void OnTimer(wxTimerEvent&)
	{
		Target.CmdExecuteEx(1);
	}
};

EvtCommandTimer::EvtCommandTimer(const String& s)
	:EvtCommand(s)
{
	AttachEvent("CloseFrame");
}

bool EvtCommandTimer::OnCmdEvent(ICmdParam& cmd,int phase)
{
	if(phase ==IDefs::PHASE_PRECALL && cmd.evtptr && cmd.evtptr->m_sId=="CloseFrame")
	{
		m_pTimerData.reset(NULL);
	}
	return true;
}

bool EvtCommandTimer::DoStdExecute(IStdParam& cmd)
{
	if(cmd.param2==0)
	{
		m_pTimerData.reset(NULL);
		return true;
	}

	if(!m_pTimerData)
	{
		m_pTimerData.reset(new ITimerData(*this));
	}

	ITimerData* ptimer=static_cast<ITimerData*>(m_pTimerData.get());
	if(cmd.param2==-1)
	{
		ptimer->StartOnce(cmd.param1);
	}
	else if(cmd.param2==1)
	{
		ptimer->Start(cmd.param1,false);
	}
	return true;
}


IEW_Ctrl::IEW_Ctrl(EvtGroup* p):m_pGroup(p)
{
	if(m_pGroup) m_pGroup->m_aCtrls.insert(this);
}

IEW_Ctrl::~IEW_Ctrl()
{
	if(m_pGroup) m_pGroup->m_aCtrls.erase(this);
}

EW_LEAVE
