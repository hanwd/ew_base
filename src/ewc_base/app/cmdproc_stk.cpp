#include "ewc_base/app/cmdproc_stk.h"
#include "ewc_base/app/data_defs.h"

EW_ENTER


bool CmdGroup::Undo()
{
	for(arr_1t<DataPtrT<CmdBase> >::reverse_iterator it=aCommands.rbegin();it!=aCommands.rend();++it)
	{
		if((*it)->Undo()) continue;
		for(;it!=aCommands.rbegin();--it)
		{
			if(!(*it)->Redo())
			{
				System::LogTrace("CmdGroup::Undo failed!!");
				return false;
			}
		}
		return false;
	}

	return true;
}

bool CmdGroup::Redo()
{
	for(arr_1t<DataPtrT<CmdBase> >::iterator it=aCommands.begin();it!=aCommands.end();++it)
	{
		if((*it)->Redo()) continue;

		for(;it!=aCommands.begin();--it)
		{
			if(!(*it)->Undo())
			{
				System::LogTrace("CmdGroup::Redo failed!!");
				return false;
			}
	
		return false;
		}
	}
	return true;		
}


bool CmdProcStk::DoExecId(ICmdParam& cmd)
{
	switch(cmd.param1)
	{
	case CP_UNDO:
		{
			if(nIndex==0) return false;

			DataPtrT<ObjectData> locker(CreateLockerObject());
			if(!aCommands[nIndex-1]->Undo())
			{
				return false;
			}
			--nIndex;
			return true;
		}
	case CP_REDO:
		{
			if(nIndex==aCommands.size()) return false;

			DataPtrT<ObjectData> locker(CreateLockerObject());
			if(!aCommands[nIndex]->Redo())
			{
				return false;
			}
			++nIndex;
			return true;
		}
	case CP_DIRTY:
		nLast=-1;
		return true;
	case CP_LOAD_FILE:
		if (basetype::DoExecId(cmd))
		{
			aCommands.clear();
			nIndex=nLast=0;
			return true;
		}
		else
		{
			return false;
		}
	case CP_SAVE:
	case CP_SAVEAS:
		if (basetype::DoExecId(cmd))
		{
			nLast=nIndex;
			return true;
		}
		else
		{
			return false;
		}
	default:
		return basetype::DoExecId(cmd);
	};
}


bool CmdProcStk::DoTestId(ICmdParam& cmd)
{
	switch(cmd.param1)
	{
	case CP_UNDO:
		if(nIndex>0)
		{
			cmd.extra1=aCommands[nIndex-1]->GetName();
			return true;
		}
		else
		{
			cmd.extra1="";
			return false;
		}
	case CP_REDO:
		if(nIndex<aCommands.size())
		{
			cmd.extra1=aCommands[nIndex]->GetName();
			return true;
		}
		else
		{
			cmd.extra1="";
			return false;
		}
	case CP_DIRTY:
		return nIndex!=nLast;
	//case CP_SAVE:
	//	cmd.extra1=m_sFilename;
	//	return true;
	case CP_LOAD:
		return true;
	default:
		return basetype::DoTestId(cmd);
	};
}


bool CmdProcStk::add_cmd(DataPtrT<CmdBase> c)
{
	if(!c->Redo())
	{
		return false;
	}

	aCommands.resize(nIndex);
	aCommands.push_back(c);
	++nIndex;

	return true;
}

EW_LEAVE
