
#include "ewc_base/evt/evt_cmdproc.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/mvc/mvc_book.h"

EW_ENTER

EvtCommandCmdProc::EvtCommandCmdProc(WndManager& w,const String& s,int d)
	:EvtCommand(s)
	,wm(w)
	,id(d)
{}

void EvtCommandCmdProc::DoUpdateCtrl(IUpdParam& upd)
{
	if(m_sId=="SaveAs")
	{
		m_sId=m_sId;
	}
	sExtra="";
	bool f1=wm.cmdptr.TestId(id,sExtra);
	flags.set(FLAG_DISABLE,!f1);
		
	if(sExtra=="")
	{
		m_sLabel="";
	}
	else
	{
		m_sLabel=m_sId+" "+sExtra;
	}

	if(id==CmdProc::CP_SAVEAS)
	{
		if(sExtra!="") m_sLabel=String::Format(_hT("Save %s As"),sExtra);
		wm.book.UpdateTitle();
	}

	EvtCommand::DoUpdateCtrl(upd);
}

bool EvtCommandCmdProc::DoCmdExecute(ICmdParam&)
{
	if(!wm.cmdptr.ExecId(id))
	{
		return false;
	}
	wm.evtmgr["CmdProc"].UpdateCtrl();
	return true;
}

EW_LEAVE
