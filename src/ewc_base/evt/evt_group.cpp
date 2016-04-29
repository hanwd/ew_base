#include "evt_impl.h"
#include "ewc_base/evt/evt_group.h"
#include "ewc_base/app/res_manager.h"

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
	

	for(bst_set<HeTbarImpl*>::iterator it=m_aSubTbars.begin();it!=m_aSubTbars.end();++it)
	{
		(*it)->Enable(!flags.get(FLAG_DISABLE));
	}

	for(bst_set<HeMenuImpl*>::iterator it=m_aSubMenus.begin();it!=m_aSubMenus.end();++it)
	{
		CreateMenu(*it,false);
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
		
		if(item.flags==0)
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



HeToolItemImpl* EvtGroup::CreateToolItem(HeTbarImpl* tb)
{
	HeToolItemImpl* item=new HeToolItemImpl(tb,this);

	if(!m_bmpParam)
	{
		m_bmpParam=ResManager::current().icons.get(m_sId);
	}
	m_bmpParam.update(item);

	wxMenu* menu=this->CreateMenu();
	if(menu)
	{
		item->SetDropdownMenu(menu);
	}

	tb->AddTool(item);
	UpdateToolItem(item);

	return item;
}


HeMenuItemImpl* EvtGroup::CreateMenuItem(HeMenuImpl* mu)
{
	if(flags.get(FLAG_CHECK))
	{
		return EvtCommand::CreateMenuItem(mu);
	}
	else
	{
		HeMenuItemImpl* item=DoCreateMenuImpl(mu,this);
		item->SetSubMenu(CreateMenu());
		mu->Append(item);
		return item;
	}
}

wxMenu* EvtGroup::CreateMenu(HeMenuImpl* mu,bool prepare)
{
	if(prepare)
	{
		PrepareItems();
	}

	if(!mu)
	{
		mu=new HeMenuImpl(this);
	}
	else
	{
		EvtGroup::ClearMenu(mu);
	}

	int last_is_seperator=-1;
	for(size_t i=0;i<impl.size();i++)
	{
		EvtCommand* vp=(*this)[i].get();
		if(vp->flags.get(EvtBase::FLAG_HIDE_UI)) continue;
		if(vp->flags.get(EvtCommand::FLAG_SEPARATOR))
		{
			if(last_is_seperator==0) last_is_seperator=1;
		}
		else
		{
			if(last_is_seperator==1) mu->AppendSeparator();
			last_is_seperator=0;

			vp->CreateMenuItem(mu);
		}
	}

	if(last_is_seperator==-1)
	{
		EvtCommand* vp=EvtManager::current()["Empty"].cast_command();
		if(vp)
		{
			mu->Append(wxID_ANY,vp->MakeLabel().c_str())->Enable(false);
		}
	}

	return mu;
}

wxToolBar* EvtGroup::CreateTbar(wxWindow* pw,int wd)
{

	PrepareItems();

	HeTbarImpl* tb=new HeTbarImpl(this,pw,wd);

	for(size_t i=0;i<size();i++)
	{
		EvtCommand* pCommand=(*this)[i].get();
		if(pCommand->flags.get(EvtBase::FLAG_HIDE_UI)) continue;
		if(pCommand->flags.get(EvtBase::FLAG_SEPARATOR)) {tb->AddSeparator();continue;}

		pCommand->CreateToolItem(tb);
	}

	tb->Realize();

	if(flags.get(FLAG_HIDE_UI)) tb->Show(false);
	if(flags.get(FLAG_DISABLE)) tb->Enable(false);

	flags.set(FLAG_CHECKED,!flags.get(FLAG_HIDE_UI));

	tb->SetName(m_sId.c_str());

	return tb;

}

void EvtGroup::ClearMenu(wxMenu* mu)
{
	int nc=mu->GetMenuItemCount();
	while(--nc>=0)
	{
		wxMenuItem* mi=mu->FindItemByPosition(0);
		mu->Destroy(mi);
	}
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

EW_LEAVE
