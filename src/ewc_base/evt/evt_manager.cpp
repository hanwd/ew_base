#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/wnd/impl_wx/impl_wx.h"

EW_ENTER

EvtManager::~EvtManager()
{
	for(map_events::iterator it=m_aEvents.begin();it!=m_aEvents.end();++it)
	{
		EvtBase* p=(*it).second.get();
		if(!p) continue;
		p->UnLink();
	}
}

void EvtManager::link(const String& s,double& v){link(s,new EvtProxyRefT<double>(v));}
void EvtManager::link(const String& s,float& v){link(s,new EvtProxyRefT<double,float>(v));}
void EvtManager::link(const String& s,String& v){link(s,new EvtProxyRefT<String>(v));}
void EvtManager::link(const String& s,int32_t& v){link(s,new EvtProxyRefT<int32_t>(v));}
void EvtManager::link(const String& s,int64_t& v){link(s,new EvtProxyRefT<int32_t,int64_t>(v));}
void EvtManager::link(const String& s,bool& v){link(s,new EvtProxyRefT<int32_t,bool>(v));}
void EvtManager::link(const String& s,BitFlags& v,int n){link(s,new EvtProxyFlags(v,n));}

void EvtManager::link(const String& s,String& v,IValueOptionData* d)
{
	EvtProxyRefT<String>* vp=new EvtProxyRefT<String>(v);
	vp->opt_data.reset(d);
	link(s,vp);
}

void EvtManager::link(const String& s,EvtBase* p)
{
	p->m_sId=s;
	gp_add(p);
}

int EvtManager::id_new()
{
	TimePoint tp=Clock::now();
	String s="__newid@";
	s<<tp.val;
	return gp_add(new EvtBase(s));

}

int EvtManager::append(EvtBase* v)
{
	const String& s(v->m_sId);

	int id=m_aEvents.find1(s);
	if(id>=0)
	{
		map_events::value_type& p(m_aEvents.get(id));
		if(p.second.get()&&p.second.get()!=v)
		{
			System::LogTrace("%s is already in evtcenter",s);
		}
		p.second.reset(v);

	}
	else
	{
		id=m_aEvents.insert(std::make_pair(s,v));
		EW_ASSERT(v->m_nId<0);
	}

	int eventid=m_nIdMin+id;
	v->m_nId=eventid;
	return eventid;
}

int EvtManager::gp_add(EvtBase* v)
{
	gp_add(v->m_sId);
	return append(v);
}



EvtBase* EvtManager::get(const String& s)
{
	int id=m_aEvents.find1(s);
	return id>=0?m_aEvents.get(id).second.get():NULL;
}

EvtGroup* EvtManager::get_group(const String& s)
{
	EvtBase* gp=get(s);
	return gp?gp->cast_group():NULL;
}

EvtCommand* EvtManager::get_command(const String& s)
{
	EvtBase* gp=get(s);
	return gp?gp->cast_command():NULL;
}

EvtBase& EvtManager::operator[](const String& s)
{
	DataPtrT<EvtBase>& v(m_aEvents[s]);
	if(!v)
	{
		v.reset(new EvtBase(s));
	}
	return *v;
}


EvtBase* EvtManager::get(int evtid)
{
	size_t id=evtid-m_nIdMin;
	if(id>=m_aEvents.size())
	{
		return NULL;
	}
	return m_aEvents.get(id).second.get();
}


void EvtManager::get_item(arr_1t<EvtItem>& v)
{
	if(m_aGroups.empty())
	{
		v.clear();
		return;
	}
	m_aGroups.back()->get(v);
}

void EvtManager::gp_set(const arr_1t<EvtItem>& v)
{
	if(m_aGroups.empty())
	{
		return;
	}

	m_aGroups.back()->set(v);
}

void EvtManager::item_get(const String& s,arr_1t<EvtItem>& v)
{
	EvtGroup* gp=get_group(s);
	if(gp!=NULL)
	{
		gp->get(v);
	}
	else
	{
		v.clear();
	}
}

void EvtManager::item_set(const String& s,const arr_1t<EvtItem>& v)
{
	EvtGroup* gp=get_group(s);
	if(gp!=NULL)
	{
		gp->set(v);
	}
}

void EvtManager::gp_beg(EvtGroup* g)
{
	gp_add(g);
	gp_beg(g->m_sId);
}

void EvtManager::gp_beg(const String& s,const String& d)
{
	EvtGroup* gp=(*this)[d].CreateGroup(s);
	if(!gp)
	{
		gp=new EvtGroup(s);
	}
	gp_add(gp);

	m_aGroups.push_back(gp);
}

void EvtManager::gp_new(const String& s)
{
	EvtGroup* gp=new EvtGroup(s);
	gp_add(gp);
	m_aGroups.push_back(gp);
}

void EvtManager::gp_beg(const String& s)
{
	EvtGroup* gp=dynamic_cast<EvtGroup*>(get(s));
	if(!gp)
	{
		gp=new EvtGroup(s);
		gp_add(gp);
	}
	m_aGroups.push_back(gp);
}

void EvtManager::gp_end()
{
	EvtGroup* g=m_aGroups.back().get();

	m_aGroups.pop_back();
	if(!m_aGroups.empty())
	{
		m_aGroups.back()->append(g);
	}

	g->flags.del(EvtCommand::FLAG_G_LOADED);
	//g->DoUpdateCtrl(IUpdParam& upd);
}

void EvtManager::gp_add(const String& s,int f)
{
	if(m_aGroups.empty()) return;
	EvtGroup* gp=m_aGroups.back().get();
	gp->add(s,f);
}

void EvtManager::gp_add(const String& s,const String& e,int f)
{
	if(m_aGroups.empty()) return;
	EvtGroup* gp=m_aGroups.back().get();
	gp->add(s,e,f);
}

bool EvtManager::OnCmdExecute(int evtid,int p1,WndModel* pwm)
{
	EvtBase* evtptr=get(evtid);
	if(!evtptr) return false;

	ICmdParam cmd(evtptr,p1,-1,pwm);
	return evtptr->CmdExecute(cmd);
}

//EvtCommand& EvtManager::link(const String& s,functor_type f)
//{
//	DataPtrT<EvtCommandFunctor> vp=new EvtCommandFunctor(s,f);
//	gp_add(vp.get());
//	return *vp;
//}


class EvtCommandUpdateCtrl : public EvtCommand
{
public:

	EvtManager& Target;

	EvtCommandUpdateCtrl(EvtManager& t):EvtCommand("UpdateCtrl"),Target(t){}

	bool CmdExecute(ICmdParam& cmd)
	{
		Target.update("");
		return true;
	}
};

EvtManager::EvtManager(int id)
{
	if(id<0)
	{
		m_nIdMin=m_nIdMax=12000;
	}
	else
	{
		m_nIdMin=m_nIdMax=id;
	}

	gp_add(new EvtCommandSeparator(""));
	gp_add(new EvtCommandSeparator("separator"));
	gp_add(new EvtCommandSeparator("breaker"));
	gp_add(new EvtCommandUpdateCtrl(*this));

}
	
void EvtManager::update(const String& id)
{

	if(id=="")
	{
		m_tMutex.lock();
		arr_1t<String> tmp;
		tmp.swap(m_aPendingUpdate);
		m_tMutex.unlock();

		IUpdParam upd;
		for(size_t i=0;i<tmp.size();i++)
		{
			(*this)[tmp[i]].UpdateCtrl(upd);
		}

	}
	else
	{
		LockGuard<AtomicMutex> lock(m_tMutex);
		m_aPendingUpdate.push_back(id);
		pending("UpdateCtrl");
	}

}

void EvtManager::pending(const String& sid,int v)
{
	int id=(*this)[sid].m_nId;
	if(id<0)
	{
		return;
	}
	wxCommandEvent e(wxEVT_COMMAND_BUTTON_CLICKED,id);
	e.SetInt(v);

	wxWindow* p=GetWindow();
	p->GetEventHandler()->AddPendingEvent(e);
}

 wxWindow* EvtManager::GetWindow()
 {
	 return WndManager::current().model.GetWindow();
 }


 
EvtManagerTop::EvtManagerTop(WndManager& wm_):wm(wm_)
{
}

 wxWindow* EvtManagerTop::GetWindow()
 {
	 return wm.model.GetWindow();
 }
void EvtManagerTop::langup()
{
	for(size_t i=0;i<m_aEvents.size();++i)
	{
		EvtBase* p=m_aEvents.get(i).second.get();
		if(!p) continue;
		EvtCommand* c=p->cast_command();
		if(!c) continue;

		if(c->m_sText!="")
		{
			c->m_sText=wm.lang.Translate(c->m_sId);
		}
	}
}


EW_LEAVE
