
#include "ewc_base/evt/evt_group.h"
#include "ewc_base/wnd/wnd_manager.h"
#include "ewc_base/wnd/wnd_updator.h"

EW_ENTER


EvtGroup::EvtGroup(const String& name):basetype(name)
{
	flags.add(FLAG_GROUP|FLAG_DOTDOT);
}

void EvtGroup::DoUpdateCtrl(IUpdParam& upd)
{
	PrepareItems();

	if(flags.get(FLAG_CHECK))
	{
		basetype::DoUpdateCtrl(upd);
	}
	else
	{
		for(size_t i=0;i<size();i++)
		{
			(*this)[i]->UpdateCtrl(upd);
		}
		basetype::DoUpdateCtrl(upd);
	}

	for(bst_set<ICtl_object*>::iterator it=m_aCtrls.begin();it!=m_aCtrls.end();++it)
	{
		(*it)->StdExecuteEx("update");
	}

}

void EvtGroup::UnLink()
{
	basetype::UnLink();
	impl.clear();
}

void EvtGroup::set(const arr_1t<EvtItem>& a)
{
	flags.del(FLAG_G_LOADED);
	m_aItems=a;
}

void EvtGroup::add(const String& s,const String& e,int f)
{
	flags.del(FLAG_G_LOADED);
	m_aItems.push_back(EvtItem(s,e,f));
}
void EvtGroup::add(const String& s,int f)
{
	flags.del(FLAG_G_LOADED);
	m_aItems.push_back(EvtItem(s,f));
}

void EvtGroup::get(arr_1t<EvtItem>& a)
{
	a=m_aItems;
}


void EvtGroup::PrepareItems()
{
	if(flags.get(FLAG_G_DYNAMIC)||!flags.get(FLAG_G_LOADED))
	{
		DoPrepareItems(m_aItems);
	}
}


void EvtGroup::DoAppendItem(const arr_1t<EvtItem>& a)
{
	EvtManager& ec(EvtManager::current());

	for(size_t i=0;i<a.size();i++)
	{
		const EvtItem& item(a[i]);
		EvtBase* vp=ec.get(item.sname);
		if(!vp) continue;
		
		if(item.flags.val()==0)
		{
			impl.append(vp->cast_command());
		}
		else
		{
			EvtGroup* gp=vp->cast_group();
			if(!gp) continue;
			DoAppendItem(gp->m_aItems);
		}
	}
}

void EvtGroup::DoPrepareItems(const arr_1t<EvtItem>& items)
{
	flags.add(FLAG_G_LOADED);
	impl.clear();
	DoAppendItem(items);
}



void EvtGroup::CreateCtrlItem(ICtl_object* pctrl)
{
	if(pctrl->GetWindow()||!flags.get(FLAG_CHECK))
	{
		pctrl->AddCtrlItem(this);
	}
	else
	{
		return EvtCommand::CreateCtrlItem(pctrl);
	}
}


ICtl_object* EvtGroup::CreateCtrl(const ICtlParam& param)
{
	PrepareItems();
	return ICtl_object::WndCreateCtrl(param,this);
}


bool EvtGroup::CmdExecute(ICmdParam& cmd)
{
	if(flags.get(FLAG_RECURSIVE))
	{
		PrepareItems();
		for(size_t i=0;i<size();i++)
		{
			(*this)[i]->CmdExecute(cmd);
		}
		return true;
	}
	return basetype::CmdExecute(cmd);
}

bool EvtGroup::StdExecute(IStdParam& cmd)
{
	if(flags.get(FLAG_RECURSIVE))
	{
		PrepareItems();
		for(size_t i=0;i<size();i++)
		{
			(*this)[i]->StdExecute(cmd);
		}
		return true;
	}
	return basetype::StdExecute(cmd);
}

bool EvtGroup::WndExecute(IWndParam& cmd)
{
	if(flags.get(FLAG_RECURSIVE))
	{
		PrepareItems();
		for(size_t i=0;i<size();i++)
		{
			cmd.iwvptr=NULL;
			EvtBase* pevt=(*this)[i].get();
			pevt->WndExecute(cmd);
		}
		return true;
	}
	return basetype::WndExecute(cmd);
}


EvtRadio::EvtRadio(const String& id) :basetype(id)
{
	this->AttachEvent("StartFrame");
}

void EvtRadio::DoUpdateCtrl(IUpdParam& upd)
{
	basetype::DoUpdateCtrl(upd);
}

void EvtRadio::DoAppendItem(const arr_1t<EvtItem>& a)
{
	basetype::DoAppendItem(a);
	for (size_t i = 0; i < impl.size(); i++)
	{
		(*this)[i]->flags.add(FLAG_CHECK|FLAG_RADIO);
		AttachEvent((*this)[i].get());
	}

}

bool EvtRadio::OnCmdEvent(ICmdParam& cmd, int phase)
{
	if (cmd.evtptr->m_sId == "StartFrame")
	{
		if (phase == IDefs::PHASE_PRECALL)
		{
			PrepareItems();
		}

		return true;
	}


	if (!phase == IDefs::PHASE_POSTCALL)
	{
		return true;
	}

	for (size_t i = 0; i < impl.size(); i++)
	{
		EvtBase* pevt = (*this)[i].get();
		pevt->flags.set(FLAG_CHECKED,pevt==cmd.evtptr);
	}

	WndManager::current().wup.gp_add(m_sId);	
	return true;
}

EW_LEAVE
