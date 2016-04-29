#include "ewc_base/app/cmdproc.h"
#include "ewc_base/app/data_defs.h"
#include "ewc_base/app/wrapper.h"

EW_ENTER

bool CmdProc::DoExecId(ICmdParam& cmd)
{
	if(cmd.param1==CP_SAVE_TEMP)
	{
		return true;
	}
	else if(cmd.param1==CP_SAVE||cmd.param1==CP_SAVEAS)
	{

		if(!TestId(CmdProc::CP_SAVEAS,cmd.extra))
		{
			return false;
		}

		if(cmd.extra==""||cmd.param1==CP_SAVEAS)
		{
			String exts;
			TestId(CP_FILEEXT,exts);

			if(Wrapper::FileDialog(cmd.extra,IDefs::FD_SAVE,"",exts)==IDefs::BTN_CANCEL)
			{
				return false;
			}
		}

		cmd.param1=CP_SAVE_TEMP;
		if(!DoExecId(cmd)) return false;
		cmd.param1=CP_SAVE_FILE;
		if(!DoExecId(cmd)) return false;
		return true;
	}
	else
	{
		return false;
	}
}

bool CmdProc::DoTestId(ICmdParam& cmd)
{
	if(cmd.param1==CP_SAVE)
	{
		cmd.param1=CP_SAVEAS;
		if(!DoTestId(cmd)) return false;

		cmd.param1=CP_DIRTY;
		if(!DoTestId(cmd)) return false;

		return true;
	}
	return false;
}

bool CmdProc::ExecId(int id,int p2)
{
	ICmdParam cmd(id,p2);
	return DoExecId(cmd);
}

bool CmdProc::TestId(int id,int p2)
{
	ICmdParam cmd(id,p2);
	return DoTestId(cmd);
}

bool CmdProc::ExecId(int id,String& p2)
{
	ICmdParam cmd(id);
	bool f1=DoExecId(cmd);
	p2=cmd.extra;
	return f1;
}

bool CmdProc::TestId(int id,String& p2)
{
	ICmdParam cmd(id);
	bool f1=DoTestId(cmd);
	p2=cmd.extra;
	return f1;
}



EW_LEAVE
