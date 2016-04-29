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
	case CP_LOAD:
		if(DoLoad())
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
		if(DoSave(cmd.extra==""?m_sFilename:cmd.extra))
		{
			nLast=nIndex;
			return true;
		}
		else
		{
			return false;
		}
	default:
		return false;
	};
}


bool CmdProcStk::DoTestId(ICmdParam& cmd)
{
	switch(cmd.param1)
	{
	case CP_UNDO:
		if(nIndex>0)
		{
			cmd.extra=aCommands[nIndex-1]->GetName();
			return true;
		}
		else
		{
			cmd.extra="";
			return false;
		}
	case CP_REDO:
		if(nIndex<aCommands.size())
		{
			cmd.extra=aCommands[nIndex]->GetName();
			return true;
		}
		else
		{
			cmd.extra="";
			return false;
		}
	case CP_DIRTY:
		return nIndex!=nLast;
	case CP_SAVE:
		cmd.extra=m_sFilename;
		return true;
	case CP_LOAD:
		return true;
	default:
		return false;
	};
}


bool CmdProcStk::add_cmd(CmdBase* c)
{
	if(!c->Redo())
	{
		c->IncRef();
		c->DecRef();
		return false;
	}

	aCommands.resize(nIndex);
	aCommands.push_back(c);
	++nIndex;

	return true;
}

EW_LEAVE
