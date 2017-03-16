#include "ewc_base/app/cmdproc.h"
#include "ewc_base/app/data_defs.h"
#include "ewc_base/app/wrapper.h"

EW_ENTER


bool CmdProc::DoLoad(const String&)
{
	return true;
}

bool CmdProc::DoSave(const String&)
{
	return true;
}

bool CmdProc::DoExecId(ICmdParam& cmd)
{
	if (cmd.param1 == CP_LOAD_FILE)
	{
		if(!DoLoad(cmd.extra1))
		{
			this_logger().LogError(_hT("failed_to_load_file %s!"),cmd.extra1);
			return false;
		}
		return true;
	}
	else if (cmd.param1 == CP_SAVE_FILE)
	{
		return DoSave(cmd.extra1);
	}
	else if (cmd.param1 == CP_INIT)
	{
		if (cmd.extra1 != "")
		{

		}
		else if (!TestId(CmdProc::CP_SAVEAS, cmd.extra1))
		{
			return false;
		}

		if (cmd.extra1 == "")
		{
			return true;
		}

		cmd.param1 = CP_LOAD_FILE;
		if (!DoExecId(cmd)) return false;
		return true;
	}
	else if (cmd.param1 == CP_LOAD)
	{

		if (cmd.extra1 != "")
		{

		}
		else if (!TestId(CmdProc::CP_SAVEAS, cmd.extra1))
		{
			return false;
		}

		if (cmd.extra1 == "")
		{
			String exts;
			TestId(CP_FILEEXT, exts);

			if (Wrapper::FileDialog(cmd.extra1, IDefs::FD_OPEN, "", exts) == IDefs::BTN_CANCEL)
			{
				return false;
			}
		}
		cmd.param1 = CP_LOAD_FILE;
		if (!DoExecId(cmd)) return false;
		return true;
	}

	else if(cmd.param1==CP_SAVE_TEMP)
	{
		if (cmd.extra1 != "")
		{

		}
		else if (cmd.extra2 == "")
		{
			return false;
		}
		else
		{
			cmd.extra1 = cmd.extra2 + ".temp";
		}

		cmd.param1 = CP_SAVE_FILE;
		return DoExecId(cmd);
	}

	else if(cmd.param1==CP_SAVE_POST)
	{
		if (cmd.extra1 == ""||cmd.extra2=="")
		{
			return false;
		}

		if (cmd.extra1 != cmd.extra2)
		{
			if (!FSLocal::current().Remove(cmd.extra2))
			{
				this_logger().LogError(_hT("failed_to_save_file %s!"),cmd.extra2);
				return false;
			}

			if (!FSLocal::current().Rename(cmd.extra1, cmd.extra2, 0))
			{
				this_logger().LogError(_hT("failed_to_save_file %s!"),cmd.extra2);
				return false;
			}
		}

		fn.SetFilename(cmd.extra2);

		return true;
	}
	else if(cmd.param1==CP_SAVE||cmd.param1==CP_SAVEAS)
	{

		if(!TestId(CmdProc::CP_SAVEAS,cmd.extra1))
		{
			return false;
		}

		if(cmd.extra1==""||cmd.param1==CP_SAVEAS)
		{
			String exts;
			TestId(CP_FILEEXT,exts);

			if(Wrapper::FileDialog(cmd.extra1,IDefs::FD_SAVE,"",exts)==IDefs::BTN_CANCEL)
			{
				return false;
			}
		}


		if (!FSObject::current().FileExists(cmd.extra1))
		{
			cmd.extra2 = cmd.extra1;
		}
		else
		{
			std::swap(cmd.extra1,cmd.extra2);
		}

		cmd.param1=CP_SAVE_TEMP;
		if(!DoExecId(cmd))
		{
			this_logger().LogError(_hT("failed_to_save_file %s!"),cmd.extra2);
			return false;
		}

		cmd.param1=CP_SAVE_POST;
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
	else if (cmd.param1 == CP_SAVEAS)
	{
		if (!fn.IsSavable())
		{
			return false;
		}

		cmd.extra1 = fn.GetFilename();
		return true;
	}
	else if (cmd.param1 == CP_FILEEXT)
	{
		cmd.extra1 = fn.GetExts();
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
	p2=cmd.extra1;
	return f1;
}

bool CmdProc::TestId(int id,String& p2)
{
	ICmdParam cmd(id);
	bool f1=DoTestId(cmd);
	p2=cmd.extra1;
	return f1;
}



EW_LEAVE
