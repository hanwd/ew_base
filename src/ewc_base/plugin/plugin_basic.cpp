#include "ewc_base/plugin/plugin_basic.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/wnd/wnd_model.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/mvc/mvc_book.h"
#include "ewc_base/evt/evt_base.h"
#include "ewc_base/evt/evt_cmdproc.h"
#include "ewc_base/evt/evt_command.h"
#include "ewc_base/evt/evt_option.h"
#include "ewc_base/wnd/wnd_updator.h"
#include "ewc_base/mvc/mvc_view.h"
#include <wx/aui/aui.h>
#include <wx/frame.h>

EW_ENTER


class EvtLanguages : public EvtGroup
{
public:

	EvtLanguages():EvtGroup(_kT("Languages")),wm(WndManager::current())
	{
		InitLanguages();
		AttachEvent("Config");
	}

	 bool OnCfgEvent(int lv)
	 {
		wm.conf.CfgUpdate(lv,"/basic/language",sLanguage_temp);

		if(lv>0)
		{
			StdExecuteEx(sLanguage_temp);
		}
		 return true;
	 }


	// 定义StdExecute为选择语言，参数为语言名称或id。
	virtual bool DoStdExecute(IStdParam& pm)
	{
		if(pm.extra1!="")
		{
			return SetLanguage(pm.extra1);
		}
		else if(pm.param1>=0)
		{
			return SetSelection(pm.param1);
		}
		else
		{
			pm.extra1=sLanguage;
			return true;
		}
	}

	bool SetSelection(size_t n)
	{
		if(n>=aLanguages.size()) return false;
		SetLanguage(aLanguages[n]);
		return true;
	}

	// 切换语言
	bool SetLanguage(const String& s)
	{

		sLanguage_temp=s;
		if(sLanguage_temp=="")
		{
			if(aLanguages.empty()) return false;
			sLanguage_temp=aLanguages[0];
		}

		if(sLanguage==sLanguage_temp)
		{
			return true;
		}
		sLanguage=sLanguage_temp;

		OnCfgEvent(-1);

		wm.lang.SetLanguage(sLanguage);

		//wm.lang.Clear();
		//wm.lang.AddCatalog("languages\\"+sLanguage+"\\default.po");

		wxWindow* pwin=wm.model.GetWindow();

		if(pwin) pwin->Freeze();
		try
		{
			wm.evtmgr.langup();
			wm.model.mu_set("");
			wm.wup.gp_add("Languages");
			wm.wup.gp_add("Layout");
		}
		catch(...)
		{

		}

		if(pwin) pwin->Thaw();

		return true;
	}



	void InitLanguages();

	virtual IValueOptionData* GetComboArray()
	{
		return opt_data.get();
	}

	WndManager& wm;
	arr_1t<String> aLanguages;

	String sLanguage_temp;
	String sLanguage;

	DataPtrT<IValueOptionDataEx> opt_data;
};



class EvtLanguageItem : public EvtBaseT<EvtLanguages,EvtCommand>
{
public:
	typedef EvtBaseT<EvtLanguages,EvtCommand> basetype;

	String sIndex;

	EvtLanguageItem(EvtLanguages& t,size_t n)
		:basetype(String::Format("Language.%d",(int)n),t)
		,sIndex(t.aLanguages[n])
	{
		flags.add(FLAG_CHECK);
	}

	void DoUpdateCtrl(IUpdParam& upd)
	{
		flags.set(FLAG_CHECKED,sIndex==Target.sLanguage);
		m_sLabel=sIndex;
		basetype::DoUpdateCtrl(upd);
	}

	bool DoCmdExecute(ICmdParam&)
	{
		Target.SetLanguage(sIndex);
		return true;
	}
};


// languages 目录下，每个文件夹认为是一种语言
void EvtLanguages::InitLanguages()
{
	//FileAllFiles("languages",aLanguages);

	aLanguages=Language::GetLanguages();

	EvtManager& ec(wm.evtmgr);	
	ec.gp_beg(this);
	for(size_t i=0;i<aLanguages.size();i++)
	{
		ec.gp_add(new EvtLanguageItem(*this,i));
	}
	ec.gp_end();

	opt_data.reset(new IValueOptionDataEx);
	for(size_t i=0;i<aLanguages.size();i++)
	{
		opt_data->add(aLanguages[i]);
	}

}

class EvtCommandOpen : public EvtCommand
{
public:

	EvtCommandOpen():EvtCommand(_kT("Open"))
	{
		this->m_sHelp=_hT("open a file");
	}

	bool DoCmdExecute(ICmdParam& cmd)
	{
		WndManager& wm(WndManager::current());

		PluginEditor* _pEditor=NULL;
		PluginManager& ipm(wm.plugin);

		if(cmd.extra1=="")
		{
			int index(0);
			arr_1t<String> exts;

			arr_1t<int> aEditors;
			for (size_t i = 0; i < ipm.plugin_editor.size(); i++)
			{
				PluginEditor* p = ipm.plugin_editor[i].get();
				if (!p->GetExts(exts)) continue;

				if (i == cmd.param1)
				{
					index = aEditors.size()-1;
				}
				aEditors.push_back(i);
			}

			exts.push_back("All Files (*.*)|*.*");

			arr_1t<String> files;
			if (Wrapper::FileDialog(files, IDefs::FD_OPEN, "", string_join(exts.begin(), exts.end(), "|"), &index) != IDefs::BTN_OK || files.size() != 1)
			{
				return false;
			}
			cmd.extra1=files[0];

			if (index >= 0 && index<(int)aEditors.size())
			{
				cmd.param1 = aEditors[index];
			}
		}
		
		if ((size_t)cmd.param1<ipm.plugin_editor.size())
		{
			_pEditor = ipm.plugin_editor[cmd.param1].get();
		}

		// 判断这个文件是否已经打开了。
		if(wm.book.Activate(cmd.extra1))
		{
			return true;
		}

		if(!_pEditor)
		{
			int _nIndex=0;
			for(size_t i=0;i<ipm.plugin_editor.size();i++)
			{
				int n=ipm.plugin_editor[i]->MatchIndex(cmd.extra1);
				if(n>_nIndex)
				{
					_nIndex=n;
					_pEditor=ipm.plugin_editor[i].get();
				}
			}
		}

		if(!_pEditor||!_pEditor->Open(cmd.extra1))
		{
			this_logger().LogError(_hT("OpenFile %s FAILED!"),cmd.extra1);
			return false;
		}
		else
		{
			this_logger().LogMessage(_hT("OpenFile %s OK."),cmd.extra1);
			return true;
		}
	}
};

class EvtCommandCloseFrame : public EvtCommand
{
public:

	EvtCommandCloseFrame():EvtCommand("CloseFrame"){}
	bool DoCmdExecute(ICmdParam& cmd)
	{
		return MvcBook::current().CloseAll();
	}
};

// 启动Frame的事件，需要在启动时进行某些处理的时候，可监听这个事件。
class EvtCommandStartFrame : public EvtCommand
{
public:
	EvtCommandStartFrame():EvtCommand("StartFrame"){}
	bool DoCmdExecute(ICmdParam&)
	{
		return true;
	}
};


class EvtCommandFullScreen : public EvtCommand
{
public:
	EvtCommandFullScreen():EvtCommand(_kT("FullScreen"))
	{
		//flags.add(FLAG_CHECK);
	}

	wxTopLevelWindow* GetTopLevelWindow()
	{
		wxApp* app=dynamic_cast<wxApp*>(wxApp::GetInstance());
		if(!app) return NULL;
		wxTopLevelWindow* top=dynamic_cast<wxTopLevelWindow*>(app->GetTopWindow());
		if(!top) return NULL;
		return top;
	}

	void DoUpdateCtrl(IUpdParam& upd)
	{
		wxTopLevelWindow* top=GetTopLevelWindow();
		//flags.set(FLAG_CHECKED,top && top->IsFullScreen());
		EvtCommand::DoUpdateCtrl(upd);
	}

	bool DoCmdExecute(ICmdParam&)
	{
		WndManager& wm(WndManager::current());
		wxTopLevelWindow* top=GetTopLevelWindow();
		if(!top) return false;
		bool flag=!top->IsFullScreen();
		top->ShowFullScreen(flag);
		wm.wup.gp_add("FullScreen");
		return true;
	}
};

class EvtProxyCommandExit : public EvtCommand
{
public:

	EvtProxyCommandExit():EvtCommand(_kT("Exit")){}
	bool DoCmdExecute(ICmdParam&)
	{
		WndModel::current().Close();
		return true;
	}
};

class EvtProxyCommandClose : public EvtCommand
{
public:
	EvtProxyCommandClose():EvtCommand(_kT("Close")){}
	void DoUpdateCtrl(IUpdParam& upd)
	{
		flags.set(FLAG_DISABLE,WndManager::current().book.GetPageCount()==0);
		EvtCommand::DoUpdateCtrl(upd);
	}

	bool DoCmdExecute(ICmdParam&)
	{
		WndManager::current().book.Close(0);
		return true;
	}
};

class EvtProxyCommandCloseAll : public EvtCommand
{
public:

	EvtProxyCommandCloseAll():EvtCommand(_kT("CloseAll")){}

	void DoUpdateCtrl(IUpdParam& upd)
	{
		flags.set(FLAG_DISABLE,WndManager::current().book.GetPageCount()==0);
		EvtCommand::DoUpdateCtrl(upd);
	}

	bool DoCmdExecute(ICmdParam&)
	{
		WndManager::current().book.CloseAll();
		return true;
	}
};

class EvtProxyCommandSaveAll : public EvtCommand
{
public:

	EvtProxyCommandSaveAll():EvtCommand(_kT("SaveAll")){}

	void DoUpdateCtrl(IUpdParam& upd)
	{
		flags.set(FLAG_DISABLE,!WndManager::current().book.AnyDirty());
		EvtCommand::DoUpdateCtrl(upd);
	}

	bool DoCmdExecute(ICmdParam&)
	{		
		MvcBook::current().SaveAll();
		return true;
	}
};





class tagLayoutInfo
{
public:
	tagLayoutInfo(const String& s=""):name(s){}
	String name;
	String pers;
	bst_set<String> wnds;
};

class tagAuiPaneLess
{
public:
	bool operator()(wxAuiPaneInfo* lhs,wxAuiPaneInfo* rhs){return lhs->dock_pos<rhs->dock_pos;}
};

class EvtViewLayout : public EvtGroup
{
public:
	WndManager& wm;

	EvtViewLayout(WndManager& w);


	int m_nSelection;
	int m_nUser;

	arr_1t<tagLayoutInfo> aInfos;

	bool OnCfgEvent(int lv)
	{
		int n = aInfos.size() - 2;
		wm.conf.CfgUpdate(lv, "/basic/layout_count", n);

		if (n < 0 || n>20)
		{
			return true;
		}


		aInfos.resize(n + 2);


		for (int i = 0; i < n; i++)
		{
			String pre = String::Format("/basic/layouts/item_%d/", i);

			auto& item(aInfos[i+2]);

			wm.conf.CfgUpdate(lv, pre+"name", item.name);
			wm.conf.CfgUpdate(lv, pre+"pers", item.pers);

			arr_1t<String> wnds;
			if (lv < 0)
			{
				wnds.assign(item.wnds.begin(), item.wnds.end());
			}

			wm.conf.CfgUpdate(lv, pre+"wnds", wnds);

			if (lv > 0)
			{
				item.wnds.clear();
				item.wnds.insert(wnds.begin(), wnds.end());
			}

		}

		return true;
	}


	bool DoStdExecute(IStdParam& cmd)
	{
		return LoadPerspective(cmd.param1);
	}

	wxAuiManager* GetAuiManager()
	{
		wxWindow* pwin=WndModel::current().GetWindow();
		return wxAuiManager::GetManager(pwin);
	}

	bool LoadPerspective(int id)
	{
		if(id<0||id>(int)aInfos.size()) return false;

		wxAuiManager* pmgr=GetAuiManager();
		if(!pmgr)
		{
			return false;
		}

		wxAuiManager& auimgr(*pmgr);
		tagLayoutInfo& info(aInfos[id]);

		if(info.pers!="")
		{
			m_nSelection=id;
		
			auimgr.LoadPerspective(str2wx(info.pers),false);

			wxAuiPaneInfoArray& arr(auimgr.GetAllPanes());
			for(size_t i=0;i<arr.size();i++)
			{
				String pn=wx2str(arr[i].name);
				EvtBase* ib=EvtManager::current().get_command(pn);
				if(!ib) continue;

				if(info.wnds.find(pn)!=info.wnds.end())
				{
					ib->flags.add(EvtCommand::FLAG_CHECKED);
					arr[i].Show(!ib->flags.get(EvtCommand::FLAG_HIDE_UI));
				}
				else
				{
					ib->flags.del(EvtCommand::FLAG_CHECKED);
					arr[i].Show(false);
				}
			}

			auimgr.GetPane("Centerpane").Show(true);

			wm.wup.lock();
			wm.wup.gp_add("ToolBars");
			wm.wup.gp_add("OtherWindow");
			wm.wup.gp_add("View");
			wm.wup.gp_add("Layout");

			wm.wup.update();
			wm.wup.unlock();
		}

		return true;
	}

	bool SavePerspective(int n=-1,const String& s="")
	{
		if(n>(int)aInfos.size()) return false;
		if(n<0)
		{
			n=(int)aInfos.size();
			aInfos.push_back(tagLayoutInfo(String::Format("Layout.%d",n)));
		}
		if(s!="") aInfos[n].name=s;
		m_nSelection=n;

		return SavePerspective(aInfos[n]);

	}

	bool SavePerspective(tagLayoutInfo& info)
	{
		wxAuiManager* pmgr=GetAuiManager();
		if(!pmgr)
		{
			return false;
		}

		wxAuiManager& auimgr(*pmgr);

		info.pers=wx2str(auimgr.SavePerspective());
		info.wnds.clear();

		wxAuiPaneInfoArray& arr(auimgr.GetAllPanes());
		for(size_t i=0;i<arr.size();i++)
		{
			String pn=wx2str(arr[i].name);
			EvtBase* ib=wm.evtmgr.get_command(pn);
			if(!ib) continue;

			if(ib->flags.get(EvtCommand::FLAG_CHECKED))
			{
				info.wnds.insert(pn);
			}			
		}

		return true;
	}

	void UpdateToolbarPosition(wxAuiManager& auimgr)
	{
		typedef bst_set<wxAuiPaneInfo*,tagAuiPaneLess> mp_row;
		typedef bst_map<int,mp_row> mp_rows;
		mp_rows mp_infos;

		wxAuiPaneInfoArray& arr(auimgr.GetAllPanes());
		for(size_t i=0;i<arr.size();i++)
		{
			wxAuiPaneInfo& pane(arr[i]);
			if(!pane.IsToolbar()||!pane.IsDocked()) continue;
			mp_infos[pane.dock_row].insert(&pane);
		}

		for(mp_rows::iterator it=mp_infos.begin();it!=mp_infos.end();++it)
		{
			mp_row &mp_info((*it).second);
			int dpos=0;

			for(mp_row::iterator ri=mp_info.begin();ri!=mp_info.end();++ri)
			{
				wxAuiPaneInfo& pane(**ri);
				if(!pane.IsShown())
				{
					(**ri).dock_pos=dpos-1;
					continue;
				}

				pane.dock_pos=dpos;
				wxSize sz=static_cast<wxToolBar*>(pane.window)->GetBestSize();
				if(sz.x!=pane.rect.x)
				{
					pane.rect.width=sz.x;
					pane.best_size.x=sz.x;
					pane.window->SetSize(sz.x,sz.y);
				}

				pane.rect.x=dpos;
				dpos+=pane.best_size.x;
			}
		}

		auimgr.Update();
	}
};

class EvtViewLayoutSave : public EvtBaseT<EvtViewLayout,EvtCommand>
{
public:
	typedef EvtBaseT<EvtViewLayout,EvtCommand> basetype;
	EvtViewLayoutSave(EvtViewLayout& t):basetype(_kT("Layout.Save"),t){}

	bool DoStdExecute(IStdParam& cmd)
	{

		wxWindow* pwin=WndModel::current().GetWindow();
		wxAuiManager* pmgr=wxAuiManager::GetManager(pwin);
		if(!pmgr) return false;
		wxAuiManager& auimgr(*pmgr);

		typedef bst_set<wxAuiPaneInfo*,tagAuiPaneLess> mp_row;

		if (Target.aInfos.size() < 3)
		{
			Target.aInfos.resize(3);
			Target.SavePerspective(2);
		}

		Target.aInfos[2].name = _kT("LastRun");

		Target.SavePerspective(0,_kT("StartUp.Default"));


		mp_row mp_alltoolbars;
		mp_row mp_allwindows;

		int tb_row=0;
		wxAuiPaneInfoArray& arr(auimgr.GetAllPanes());
		for(size_t i=0;i<arr.size();i++)
		{
			wxAuiPaneInfo& pane(arr[i]);
			if(!pane.IsDocked()) continue;
			if(pane.IsToolbar())
			{
				mp_alltoolbars.insert(&pane);
				pane.dock_row=tb_row++;
			}
		}

		Target.UpdateToolbarPosition(auimgr);
		Target.SavePerspective(1,_kT("StartUp.Failsafe"));
		Target.LoadPerspective(2);

		EvtManager::current()["Layout"].UpdateCtrl();

		return true;
	}

	bool DoCmdExecute(ICmdParam& cmd)
	{
		LockGuard<WndUpdator> lock(WndUpdator::current());

		Target.SavePerspective(cmd.param1,cmd.extra1);
		WndUpdator::current().gp_add("Layout");

		return true;
	}

};

class EvtViewLayoutDelete : public EvtBaseT<EvtViewLayout,EvtCommand>
{
public:
	typedef EvtBaseT<EvtViewLayout,EvtCommand> basetype;
	EvtViewLayoutDelete(EvtViewLayout& t):basetype(_kT("Layout.Delete"),t){}

	bool DoCmdExecute(ICmdParam&)
	{
		if(size_t(Target.m_nSelection)>=Target.aInfos.size()) return true;

		LockGuard<WndUpdator> lock(WndUpdator::current());
		Target.aInfos.erase(Target.aInfos.begin()+Target.m_nSelection);
		Target.m_nSelection=-1;

		WndUpdator::current().gp_add("Layout");
		return true;
	}

	void DoUpdateCtrl(IUpdParam& upd)
	{
		flags.set(FLAG_DISABLE,size_t(Target.m_nSelection)>=Target.aInfos.size()||Target.m_nSelection<Target.m_nUser);
		basetype::DoUpdateCtrl(upd);
	}

};

class EvtViewLayoutItem : public EvtBaseT<EvtViewLayout,EvtCommand>
{
public:
	typedef EvtBaseT<EvtViewLayout,EvtCommand> basetype;

	EvtViewLayoutItem(EvtViewLayout& t,size_t n):basetype(String::Format("Layout.Item%d",(int)n),t),m_nItem(n)
	{
		flags.add(FLAG_CHECK);
	}

	bool DoCmdExecute(ICmdParam&)
	{
		LockGuard<WndUpdator> lock(WndUpdator::current());
		Target.LoadPerspective(m_nItem);
		Target.wm.wup.gp_add("Layout");
		return true;
	}

	void DoUpdateCtrl(IUpdParam& upd)
	{
		if(m_nItem<Target.aInfos.size())
		{
			flags.del(FLAG_HIDE_UI);
			flags.set(FLAG_CHECKED,m_nItem==Target.m_nSelection);
			if(m_nItem<3)
			{
				m_sLabel=Translate(Target.aInfos[m_nItem].name);
			}
			else
			{
				m_sLabel=Target.aInfos[m_nItem].name;
			}
		}
		else
		{
			flags.add(FLAG_HIDE_UI);
		}
		basetype::DoUpdateCtrl(upd);
	}

	size_t m_nItem;
};

EvtViewLayout::EvtViewLayout(WndManager& w):EvtGroup(_kT("Layout")),wm(w)
{
	EvtManager& ec(wm.evtmgr);
	m_nUser=3;

	ec.gp_beg(this);
		ec.gp_add(new EvtViewLayoutSave(*this));
		ec.gp_add(new EvtViewLayoutDelete(*this));	
		ec.gp_add("");
		for(int i=0;i<20;i++)
		{
			if(i==m_nUser) ec.gp_add("");
			ec.gp_add(new EvtViewLayoutItem(*this,i));
		}
	ec.gp_end();
	m_nSelection=-1;

	AttachEvent("Config");
}



class IEvtCommandButton : public EvtCommand
{
public:

	IEvtCommandButton(const String& s):EvtCommand(s)
	{

	}

	bool DoCmdExecute(ICmdParam& cmd)
	{
		if(!cmd.iwmptr) return false;

		WndModel& Target(*cmd.iwmptr);
		if(m_sId=="Btn.Ok")
		{
			if (Target.WndExecuteEx(IDefs::ACTION_APPLY))
			{
				Target.Show(false);
			}
		}
		else if(m_sId=="Btn.Apply")
		{
			Target.WndExecuteEx(IDefs::ACTION_APPLY);
		}
		else if(m_sId=="Btn.Reset")
		{
			Target.WndExecuteEx(IDefs::ACTION_TRANSFER2WINDOW);
		}
		else if(m_sId=="Btn.Cancel")
		{
			Target.Show(false);
		}
		return true;
	}

};


class EvtCommandConfig : public EvtCommand
{
public:
	WndManager& wm;
	EvtCommandConfig(WndManager& wm_):EvtCommand("Config"),wm(wm_){}

	bool StdExecute(IStdParam& cmd)
	{
		size_t num;
		for(num=0;num<m_aAttachedListeners.size();num++)
		{
			EvtListener* lis=m_aAttachedListeners[num];
			if(!lis->OnCfgEvent(cmd.param1))
			{
				break;
			}
		}

		return true;
	}

};



PluginBasic::PluginBasic(WndManager& w):PluginCommon(w,"Plugin.Basic")
{

}


bool PluginBasic::OnCmdEvent(ICmdParam& cmd,int phase)
{
	if(phase==IDefs::PHASE_PRECALL)
	{
		if(cmd.evtptr->m_sId=="StartFrame")
		{

		}	
	}

	if(phase==IDefs::PHASE_POSTCALL)
	{
		if(cmd.evtptr->m_sId=="StartFrame")
		{
			{
				LockGuard<WndUpdator> lock(wm.wup);
				wm.wup.mu_set("MenuBar.default");
				wm.wup.tb_set("ToolBar.default");
				wm.wup.sb_set("StatusBar.default");
				wm.wup.gp_add("MenuBar.default");
			}
			wm.evtmgr["Layout.Save"].StdExecuteEx(-1);

			//wm.evtmgr["Layout.Item2"].CmdExecuteEx(-1);
		}

		if (cmd.evtptr->m_sId == "CloseFrame")
		{
			wm.evtmgr["Layout.Save"].CmdExecuteEx(2);
		}
	}

	return true;
}

class EvtCommandViewRefresh : public EvtCommand
{
public:
	EvtCommandViewRefresh() :EvtCommand("View.Refresh")
	{

	}

	bool DoCmdExecute(ICmdParam& cmd)
	{
		if (MvcView::ms_pActiveView)
		{
			MvcView::ms_pActiveView->DoRefresh();
		}
		return true;
	}
};


bool PluginBasic::OnAttach()
{

	EvtManager& ec(wm.evtmgr);

	ec.append(new EvtCommand(_kT("Empty")));
	ec.append(new EvtCommandViewRefresh());

	ec.append(new IEvtCommandButton(_kT("Btn.Ok")));
	ec.append(new IEvtCommandButton(_kT("Btn.Apply")));
	ec.append(new IEvtCommandButton(_kT("Btn.Cancel")));
	ec.append(new IEvtCommandButton(_kT("Btn.Reset")));

	ec.append(new EvtCommandConfig(wm));

	ec.append(new EvtProxyCommandClose());
	ec.append(new EvtProxyCommandCloseAll());
	ec.append(new EvtProxyCommandSaveAll());
	ec.append(new EvtProxyCommandExit());
	ec.append(new EvtCommandOpen());
	ec.append(new EvtCommandStartFrame());
	ec.append(new EvtCommandCloseFrame());
	ec.append(new EvtViewLayout(wm));
	ec.append(new EvtCommandFullScreen);
	ec.append(new EvtCommand("Website"));
	ec.append(new EvtCommand("Document"));
	ec.append(new EvtCommand("About"));

	ec.append(new EvtLanguages);

	ec.append(new EvtRadio("RadioGroup"));

	ec.gp_beg("Menu.New");

	ec.gp_end();



	ec.gp_beg("MainWindow");

	ec.gp_end();

	ec.gp_beg(_kT("OtherWindow"));

	ec.gp_end();

	ec.gp_beg(_kT("ToolBars"));

	ec.gp_end();


	ec.gp_beg("CmdProc");
		ec.gp_add(new EvtCommandCmdProc(wm,_kT("Cut"),CmdProc::CP_CUT));
		ec.gp_add(new EvtCommandCmdProc(wm,_kT("Copy"),CmdProc::CP_COPY));
		ec.gp_add(new EvtCommandCmdProc(wm,_kT("Paste"),CmdProc::CP_PASTE));
		ec.gp_add(new EvtCommandCmdProc(wm,_kT("Delete"),CmdProc::CP_DELETE));
		ec.gp_add(new EvtCommandCmdProc(wm,_kT("Clear"),CmdProc::CP_CLEAR));
		ec.gp_add(new EvtCommandCmdProc(wm,_kT("Undo"),CmdProc::CP_UNDO));
		ec.gp_add(new EvtCommandCmdProc(wm,_kT("Redo"),CmdProc::CP_REDO));
		ec.gp_add(new EvtCommandCmdProc(wm,_kT("Forward"),CmdProc::CP_NEXT));
		ec.gp_add(new EvtCommandCmdProc(wm,_kT("Backward"),CmdProc::CP_PREV));
		ec.gp_add(new EvtCommandCmdProc(wm,_kT("SelectAll"),CmdProc::CP_SELECTALL));
		ec.gp_add(new EvtCommandCmdProc(wm,_kT("Save"),CmdProc::CP_SAVE));
		ec.gp_add(new EvtCommandCmdProc(wm,_kT("SaveAs"),CmdProc::CP_SAVEAS));
		ec.gp_add("Close");
		ec.gp_add("CloseAll");
		ec.gp_add("SaveAll");
	ec.gp_end();

	ec.gp_beg(_kT("New"));
		ec.gp_add("Menu.New",1);
	ec.gp_end();

	ec.gp_new(_kT("File"));
		ec.gp_add("Open");
		ec.gp_add("New");
		ec.gp_add("");
		ec.gp_add("Save");
		ec.gp_add("SaveAs");
		ec.gp_add("SaveAll");
		ec.gp_add("");
		ec.gp_add("Close");
		ec.gp_add("CloseAll");
		ec.gp_add("");
		ec.gp_add("HistoryFiles");
		ec.gp_add("");
		ec.gp_add("Exit");
	ec.gp_end();


	ec.gp_beg(_kT("Edit"));
		ec.gp_add("Undo");
		ec.gp_add("Redo");
		ec.gp_add("");
		ec.gp_add("Cut");
		ec.gp_add("Copy");
		ec.gp_add("Paste");
		ec.gp_add("Delete");
		ec.gp_add("");
		ec.gp_add("SelectAll");
		ec.gp_add("");
		ec.gp_add("Find");
		ec.gp_add("Replace");
	ec.gp_end();

	ec.gp_beg(_kT("View"));
		ec.gp_add("MainWindow",1);
		ec.gp_add("");
		ec.gp_add("ToolBars");
		ec.gp_add("OtherWindow");
		ec.gp_add("");
		ec.gp_add("Layout");
		ec.gp_add("");
		ec.gp_add("Languages");
		ec.gp_add("");
		ec.gp_add("Option");
		ec.gp_add("");
		ec.gp_add("FullScreen");
	ec.gp_end();

	ec.gp_beg(_kT("Help"));
		ec.gp_add("Website");
		ec.gp_add("Document");
		ec.gp_add("About");
	ec.gp_end();




	ec.gp_beg(_kT("tb.Standard"));
		ec.gp_add("Open");
		ec.gp_add("New");
		ec.gp_add("");
		ec.gp_add("Save");
		ec.gp_add("SaveAs");
		ec.gp_add("");
		ec.gp_add("Cut");
		ec.gp_add("Copy");
		ec.gp_add("Paste");
		ec.gp_add("Delete");
		ec.gp_add("");
		ec.gp_add("Undo");
		ec.gp_add("Redo");
	ec.gp_end();

	ec.gp_beg("ToolBar.default");
		ec.gp_add("tb.Standard");
	ec.gp_end();

	ec.gp_beg(_kT("Edit.Mini"));
		ec.gp_add("Undo");
		ec.gp_add("Redo");
		ec.gp_add("");
		ec.gp_add("Cut");
		ec.gp_add("Copy");
		ec.gp_add("Paste");
		ec.gp_add("Delete");
		ec.gp_add("");
		ec.gp_add("SelectAll");

	ec.gp_end();

	//ec.gp_add(new EvtCommandTimer("timer.idle"));
	//ec["timer.idle"].StdExecuteEx(1000,1);

	EvtManager::current()["StartFrame"].AttachListener(this);
	EvtManager::current()["CloseFrame"].AttachListener(this);

	wm.evtmgr.link_c<String>("/basic/language").opt_data.reset(wm.evtmgr["Languages"].GetComboArray());

	wm.evtmgr.gp_beg(_kT("Option.pages"));
		wm.evtmgr.gp_add(new EvtDynamicPageScript(_kT("Option.Common"), "scripting/ui/option.common.ewsl"));
	wm.evtmgr.gp_end();



	return true;
}


IMPLEMENT_IPLUGIN(PluginBasic)

EW_LEAVE
