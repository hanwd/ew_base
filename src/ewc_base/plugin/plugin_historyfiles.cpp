#include "ewc_base/plugin/plugin_historyfiles.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/wnd/wnd_updator.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/evt/evt_base.h"
#include "ewc_base/evt/evt_command.h"


EW_ENTER


class EvtCommandClearHistory : public EvtCommand
{
public:
	PluginHistoryFiles& Target;

	EvtCommandClearHistory(PluginHistoryFiles& t)
		:EvtCommand(_kT("ClearHistory"))
		,Target(t)
	{}

	void DoUpdateCtrl(IUpdParam& upd)
	{
		flags.set(FLAG_DISABLE,Target.aHistoryFiles.empty());
		EvtCommand::DoUpdateCtrl(upd);
	}


	virtual bool DoCmdExecute(ICmdParam&)
	{
		Target.aHistoryFiles.clear();
		Target.wm.wup.gp_add("HistoryFiles");
		return true;
	}
};


class EvtCommandHistoryItem : public EvtCommand
{
public:
	uint32_t m_nItemId;

	PluginHistoryFiles& Target;

	EvtCommandHistoryItem(PluginHistoryFiles& t,uint32_t i):m_nItemId(i),Target(t)
	{
		flags.add(FLAG_HIDE_UI);
		m_sId=String::Format("HistoryFile.%d",(int)i);
	}

	void DoUpdateCtrl(IUpdParam& upd)
	{
		if(m_nItemId<Target.aHistoryFiles.size())
		{
			m_sLabel=Target.aHistoryFiles[m_nItemId];
			flags.clr(0);
		}
		else
		{
			flags.clr(FLAG_DISABLE|FLAG_HIDE_UI);
		}

		EvtCommand::DoUpdateCtrl(upd);
	}

	virtual bool DoCmdExecute(ICmdParam&)
	{
		arr_1t<String>& files(Target.aHistoryFiles);
		if(m_nItemId>=files.size())
		{
			return true;
		}

		Target.wm.evtmgr["Open"].CmdExecuteEx(files[m_nItemId]);
		return true;
	}
};

class EvtCommandHistoryFiles : public EvtGroup
{
public:

	PluginHistoryFiles& Target;

	EvtCommandHistoryFiles(PluginHistoryFiles& t)
		:EvtGroup(_kT("HistoryFiles"))
		,Target(t)
	{

	}

	virtual bool StdExecute(IStdParam& cmd)
	{
		if(cmd.param1==1)
		{
			Target.insert(cmd.extra);
			UpdateCtrl();
			return true;
		}
		else if(cmd.param1==-1)
		{
			Target.remove(cmd.extra);
			UpdateCtrl();
			return true;
		}
		return false;
	}

};


bool PluginHistoryFiles::OnCmdEvent(ICmdParam& cmd,int phase)
{
	if(!cmd.flags.get(IDefs::CMD_HANDLED_OK)) return true;

	if(phase==IDefs::PHASE_POSTCALL)
	{
		if(cmd.evtptr->m_sId=="Open")
		{
			insert(cmd.extra);
			wm.wup.gp_add("HistoryFiles");	
		}
	}
	return true;
}

bool PluginHistoryFiles::OnAttach()
{
	EvtManager& ec(wm.evtmgr);
	AttachEvent("Config");

	ec["Open"].AttachListener(this);

	ec.gp_beg(new EvtCommandHistoryFiles(*this));
		ec.gp_add(new EvtCommandClearHistory(*this));
		ec.gp_add("");
		for(size_t i=0;i<40;i++)
		{
			ec.gp_add(new EvtCommandHistoryItem(*this,i));
		}
	ec.gp_end();

	wm.wup.gp_add("File",1);

	wm.evtmgr.link_c<int64_t>("/basic/history_size");

	return true;
}

bool PluginHistoryFiles::OnCfgEvent(int lv)
{
	wm.app.conf.CfgUpdate(lv,"/basic/history_size",nHistorySize,1,40);

	if(lv>0 && aHistoryFiles.size()>(size_t)nHistorySize)
	{
			aHistoryFiles.resize(nHistorySize);
			wm.wup.gp_add("HistoryFiles");
	}

	return true;
}

void PluginHistoryFiles::OnDetach()
{

}


void PluginHistoryFiles::remove(const String& s)
{
	size_t j=0;
	for(size_t i=0;i<aHistoryFiles.size();i++)
	{
		if(aHistoryFiles[i]==s)
		{
			continue;
		}
		
		if(i!=j)
		{
			aHistoryFiles[j]=aHistoryFiles[i];
		}
		j++;
	}

	aHistoryFiles.resize(j);
}


void PluginHistoryFiles::insert(const String& s)
{
	remove(s);
	aHistoryFiles.insert(aHistoryFiles.begin(),s);

	if(aHistoryFiles.size()>(size_t)nHistorySize)
	{
		aHistoryFiles.resize(nHistorySize);
	}
}

PluginHistoryFiles::PluginHistoryFiles(WndManager& w):basetype(w,"Plugin.HistoryFiles")
{
	nHistorySize=5;
}


IMPLEMENT_IPLUGIN(PluginHistoryFiles)

EW_LEAVE
