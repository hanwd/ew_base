#include "ewc_base/plugin/plugin_statusbar.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/wnd/impl_wx/impl_wx.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/evt/evt_base.h"
#include "ewc_base/evt/evt_command.h"
#include "ewc_base/wnd/wnd_updator.h"

EW_ENTER

class IWndStatusBar;
class EvtCommandWindowStatusBar;

class EvtStatusBar : public EvtBaseT<EvtCommandWindowStatusBar,EvtGroup>
{
public:

	typedef EvtBaseT<EvtCommandWindowStatusBar,EvtGroup> basetype;

	friend class IWndStatusBar;

	EvtStatusBar(const String& s,EvtCommandWindowStatusBar& t):basetype(s,t){}

	virtual bool DoStdExecute(IStdParam&);

	virtual void DoUpdateCtrl(IUpdParam& upd);
	virtual void PrepareItems();	

protected:
	arr_1t<int> aWidths;
	arr_1t<wxWindow*> aWindows;
	arr_1t<String> aStatusTexts;
};


class IWndStatusBar : public wxStatusBar
{
public:

	IWndStatusBar(wxWindow* w):wxStatusBar(w)
	{
		this->Connect(wxEVT_SIZE, wxSizeEventHandler(IWndStatusBar::OnSize));
		this->SetFieldsCount(1);
	}

	void OnSize(wxSizeEvent& evt)
	{
		UpdateStatusBar();
		evt.Skip();
	}

	void SetStatusBar(EvtStatusBar* w)
	{
		if(w==m_pActive.get()) return;
		if(m_pActive) 
		{
			for(size_t i=0;i<m_pActive->aWindows.size();i++)
			{
				wxWindow* p=m_pActive->aWindows[i];
				if(p) p->Show(false);
			}
		}
		m_pActive.reset(w);
		if(m_pActive)
		{
			m_pActive->PrepareItems();
			UpdateStatusBar();
		}
	}

	String m_sLogtext0;

	void UpdateStatusBar()
	{
		if(!m_pActive) return;
		if(m_pActive->aWidths.empty()) return;

		bool fc1=this->GetFieldsCount()==m_pActive->aWidths.size();

		if(!fc1) SetFieldsCount(m_pActive->aWidths.size());
		SetStatusWidths(m_pActive->aWidths.size(),&m_pActive->aWidths[0]);

		for(size_t i=0;i<m_pActive->aWidths.size();i++)
		{
			wxWindow* w=m_pActive->aWindows[i];
			if(!w) continue;
			w->Show(true);
			wxRect rect;
			if(GetFieldRect(i,rect))
			{
				w->SetSize(rect);
			}
		}

		if(!fc1) for(size_t i=1;i<m_pActive->aWidths.size();i++)
		{
			if(!m_pActive->aWindows[i]) SetStatusText(str2wx(m_pActive->aStatusTexts[i]),i);
		}

	}

	void SetStatusTextEx(const String& s,int d)
	{
		if(d<=0)
		{
			SetStatusText(str2wx(s),0);
		}
		else if(m_pActive&&d<(int)m_pActive->aStatusTexts.size())
		{
			m_pActive->aStatusTexts[d]=s;
			SetStatusText(str2wx(s),d);
		}
	}

	DataPtrT<EvtStatusBar> m_pActive;

};

class EvtCommandWindowStatusBar : public EvtCommandWindow
{
public:

	EvtCommandWindowStatusBar(const String& n=_kT("StatusBar")):EvtCommandWindow(n),m_pStatusBar(NULL)
	{
		m_pStatusBar=new IWndStatusBar(WndModel::current().GetWindow());
		m_pStatusBar->SetName(str2wx(m_sId));
		SetWindow(m_pStatusBar);
	}

	bool StdExecute(IStdParam& cmd)
	{
		GetWindow()->SetStatusTextEx(cmd.extra1,cmd.param1);
		return true;
	}

	IWndStatusBar* GetWindow()
	{
		return m_pStatusBar;
	}

	EvtGroup* CreateGroup(const String& s)
	{
		return new EvtStatusBar(s,*this);
	}

protected:

	LitePtrT<IWndStatusBar> m_pStatusBar;

};


bool EvtStatusBar::DoStdExecute(IStdParam& cmd)
{
	Target.GetWindow()->SetStatusBar(cmd.param1?this:NULL);
	return true;
}

void EvtStatusBar::DoUpdateCtrl(IUpdParam& upd)
{
	PrepareItems();
	if(this!=Target.GetWindow()->m_pActive.get()) return;
	Target.GetWindow()->UpdateStatusBar();
}

void EvtStatusBar::PrepareItems()
{

	if(flags.get(FLAG_G_DYNAMIC)||!flags.get(FLAG_G_LOADED))
	{
		flags.add(FLAG_G_LOADED);

		size_t sz=m_aItems.size();

		aWidths.resize(sz);
		aWindows.resize(sz);
		aStatusTexts.resize(sz);

		for(size_t i=0;i<sz;i++)
		{
			aWidths[i]=m_aItems[i].flags.val();
			EvtBase* pevt=EvtManager::current().get(m_aItems[i].sname);
			aWindows[i]=pevt?pevt->CreateWndsItem(Target.GetWindow()):NULL;
			if(aWindows[i])
			{
				aWindows[i]->Show(false);
			}
		}
	}
}

PluginStatusBar::PluginStatusBar(WndManager& w):basetype(w,"Plugin.StatusBar")
{

}

bool PluginStatusBar::OnAttach()
{

	EvtManager& ec(EvtManager::current());

	ec.gp_beg("MainWindow");
		ec.gp_add(new EvtCommandWindowStatusBar("StatusBar"));
	ec.gp_end();
	
	ec.gp_beg("StatusBar.default","StatusBar");
		ec.gp_add("",-1);
	ec.gp_end();

	return true;
}

IMPLEMENT_IPLUGIN(PluginStatusBar)

EW_LEAVE
