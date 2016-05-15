#include "evt_impl.h"

#include "ewc_base/evt/evt_base.h"
#include "ewc_base/evt/validator.h"
#include "ewc_base/app/data_defs.h"
EW_ENTER

EvtBase::EvtBase()
{
	m_nId=-1;
}

EvtBase::EvtBase(const String& s):m_sId(s)
{
	m_nId=-1;
}

void EvtBase::UpdateCtrl(IUpdParam& upd)
{
	if(upd.test(this)) DoUpdateCtrl(upd);
}

void EvtBase::UpdateCtrl()
{
	IUpdParam upd;DoUpdateCtrl(upd);
}

bool EvtBase::CmdExecute(ICmdParam& cmd)
{
	size_t num;
	for(num=0;num<m_aAttachedListeners.size();num++)
	{
		EvtListener* lis=m_aAttachedListeners[num];
		if(!lis->OnCmdEvent(cmd,IDefs::PHASE_PRECALL))
		{
			break;
		}
	}

	cmd.flags.set(IDefs::CMD_HANDLED_OK,DoCmdExecute(cmd));

	while(num>0)
	{
		EvtListener* lis=m_aAttachedListeners[--num];
		lis->OnCmdEvent(cmd,IDefs::PHASE_POSTCALL);
	}

	return cmd.flags.get(IDefs::CMD_HANDLED_OK);
}

bool EvtBase::StdExecute(IStdParam& cmd)
{
	size_t num;
	for(num=0;num<m_aAttachedListeners.size();num++)
	{
		EvtListener* lis=m_aAttachedListeners[num];
		if(!lis->OnStdEvent(cmd,IDefs::PHASE_PRECALL))
		{
			break;
		}
	}

	cmd.flags.set(IDefs::CMD_HANDLED_OK,DoStdExecute(cmd));

	while(num>0)
	{
		EvtListener* lis=m_aAttachedListeners[--num];
		lis->OnStdEvent(cmd,IDefs::PHASE_POSTCALL);
	}

	return cmd.flags.get(IDefs::CMD_HANDLED_OK);
}


bool EvtBase::WndExecute(IWndParam& cmd)
{
	if(cmd.action>=IDefs::ACTION_UPDATECTRL)
	{
		size_t num;
		for(num=0;num<m_aAttachedListeners.size();num++)
		{
			EvtListener* lis=m_aAttachedListeners[num];
			if(!lis->OnWndEvent(cmd,0))
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		size_t num;
		for(num=0;num<m_aAttachedListeners.size();num++)
		{
			EvtListener* lis=m_aAttachedListeners[num];
			if(!lis->OnWndEvent(cmd,IDefs::PHASE_PRECALL))
			{
				break;
			}
		}

		cmd.flags.set(IDefs::CMD_HANDLED_OK,DoWndExecute(cmd));

		while(num>0)
		{
			EvtListener* lis=m_aAttachedListeners[--num];
			lis->OnWndEvent(cmd,IDefs::PHASE_POSTCALL);
		}

		return cmd.flags.get(IDefs::CMD_HANDLED_OK);
	}
}


bool EvtBase::WndExecuteEx(int action,int p1,int p2)
{
	IWndParam cmd(this,action,p1,p2);
	return WndExecute(cmd);
}

bool EvtBase::StdExecuteEx(int p1,int p2,wxWindow* pw)
{
	IStdParam cmd(this,p1,p2,pw);
	return StdExecute(cmd);
}

bool EvtBase::StdExecuteEx(const String& s1,int p1,int p2,wxWindow* pw)
{
	IStdParam cmd(this,s1,p1,p2,pw);
	return StdExecute(cmd);
}

bool EvtBase::CmdExecuteEx(int p1,int p2,WndModel* pwm)
{
	ICmdParam cmd(this,p1,p2,pwm);
	return CmdExecute(cmd);
}

bool EvtBase::CmdExecuteEx(const String& s1,int p1,int p2)
{
	ICmdParam cmd(this,s1,p1,p2);
	return CmdExecute(cmd);
}

void EvtBase::DoUpdateCtrl(IUpdParam& upd)
{
	if(m_aAttachedListeners.empty()) return;

	IWndParam cmd(this,IDefs::ACTION_UPDATECTRL);
	for(arr_1t<EvtListener*>::iterator it=m_aAttachedListeners.begin();it!=m_aAttachedListeners.end();++it)
	{
		(*it)->OnWndEvent(cmd,0);
	}
}

EvtBase::~EvtBase()
{
	EW_ASSERT(m_aAttachedListeners.empty());
}




void EvtBase::DetachAllListeners()
{
	for(EvtListenerGroup::iterator it=m_aAttachedListeners.begin();it!=m_aAttachedListeners.end();++it)
	{
		(*it)->m_aAttachedEvents.remove(this);
	}
	m_aAttachedListeners.clear();
}

void EvtBase::UnLink()
{
	basetype::UnLink();
	DetachAllListeners();
}

void EvtBase::AttachListener(EvtListener* pListener)
{
	if(m_aAttachedListeners.append(pListener))
	{
		pListener->m_aAttachedEvents.append(this);
	}
}

void EvtBase::DetachListener(EvtListener* pListener)
{
	if(m_aAttachedListeners.remove(pListener))
	{
		pListener->m_aAttachedEvents.remove(this);
	}
}


bool EvtBase::PopupMenu(IWindowPtr pw)
{
	HeMenuImpl mu;
	if(CreateMenu(&mu)==NULL)
	{
		return false;
	}

	if(!pw)
	{
		pw=WndManager::current().model.GetWindow();
		if(!pw) return false;
	}

	pw->PopupMenu(&mu);
	return true;
}

Validator* EvtBase::CreateValidator(wxWindow*)
{
	return NULL;
}

EW_LEAVE
