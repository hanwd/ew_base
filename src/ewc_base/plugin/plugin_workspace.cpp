#include "ewc_base/plugin/plugin_workspace.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/wnd/wnd_updator.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/wnd/wnd_model.h"
#include "ewc_base/wnd/impl_wx/window.h"
#include "ewc_base/mvc/mvc_book.h"
#include "ewc_base/wnd/impl_wx/iwnd_treectrl.h"
#include "ewc_base/app/res_manager.h"

EW_ENTER

class ObjectNodeFile : public ObjectNode
{
public:

	typedef ObjectNode basetype;

	ObjectNodeFile(const String& s):ObjectNode(s)
	{

	}

	String GetLabel()
	{
		return m_sName;
	}

	virtual bool OnTreeEvent(enumTreeEventType t,ITreeEventData& d)
	{
		if(t==TREE_ACTIVATE)
		{
			if(MvcBook::current().Activate(m_sName))
			{
				return true;
			}
			EvtManager::current()["Open"].CmdExecuteEx(m_sName,-1);
			return true;
		}
		return basetype::OnTreeEvent(t,d);
	}

	virtual String GetContextMenuName()
	{
		return "Wksp.file.menu";
	}
};

class ObjectNodeFolder : public ObjectNodeGroup
{
public:

	typedef ObjectNodeGroup basetype;

	ObjectNodeFolder(const String& s):ObjectNodeGroup(s)
	{
		flags.add(FLAG_EXPANDED);
	}

	virtual String GetContextMenuName()
	{
		return "Wksp.folder.menu";
	}


	bool OnTreeEvent(enumTreeEventType t,ITreeEventData& d)
	{
		if(!basetype::OnTreeEvent(t,d)) return false;

		return true;
	}

};

class ObjectNodeWorkspace : public ObjectNodeFolder
{
public:
	ObjectNodeWorkspace():ObjectNodeFolder("Workspace")
	{

	}

	virtual String GetContextMenuName()
	{
		return "Wksp.menu";
	}
};


class EvtCommandTreeCtrl : public EvtCommand
{
public:
	EvtCommandTreeCtrl(const String& s):EvtCommand(s){}

	ObjectNodeGroup* GetGroup()
	{
		ObjectNode* x=IWnd_treectrl::m_tData.pnode.get();
		return x?x->cast_group():NULL;
	}

	ObjectNodeGroup* GetParent()
	{
		ObjectNode* x=IWnd_treectrl::m_tData.pnode.get();
		if(!x) return NULL;
		if(!x->GetId().IsOk()) return NULL;
		if(x->GetId()==IWnd_treectrl::m_tData.window->GetRootItem()) return NULL;
		return IWnd_treectrl::m_tData.window->ExGetItemParent(x);
	}

};

class EvtCommandWorkspaceAddFile : public EvtCommandTreeCtrl
{
public:
	EvtCommandWorkspaceAddFile():EvtCommandTreeCtrl(_kT("Wksp.AddFile")){}

	bool DoCmdExecute(ICmdParam& cmd)
	{

		ObjectNodeGroup* g=GetGroup();
		if(!g) return true;

		arr_1t<String> files;
		if(Wrapper::FileDialog(files,IDefs::FD_OPEN|IDefs::FD_MUST_EXIST|IDefs::FD_MULTI)!=IDefs::BTN_OK)
		{
			return true;
		}

		for(size_t i=0;i<files.size();i++)
		{
			if(IWnd_treectrl::m_tData.window->ExGetRoot()->FindNodeEx(files[i])) continue;
			IWnd_treectrl::m_tData.window->ExAppendNode(new ObjectNodeFile(files[i]),g);
		}

		return true;
	}
};

class EvtCommandWorkspaceRemove : public EvtCommandTreeCtrl
{
public:
	EvtCommandWorkspaceRemove():EvtCommandTreeCtrl(_kT("Wksp.Remove")){}

	bool DoCmdExecute(ICmdParam& cmd)
	{
		ObjectNodeGroup* p=GetParent();
		if(!p) return true;

		ObjectNode* x=IWnd_treectrl::m_tData.pnode.get();
		IWnd_treectrl::m_tData.window->Delete(x->GetId());
		p->remove(x);

		return true;
	}
};


class EvtCommandWorkspaceRemoveChildren : public EvtCommandTreeCtrl
{
public:
	EvtCommandWorkspaceRemoveChildren():EvtCommandTreeCtrl(_kT("Wksp.RemoveChildren")){}

	bool DoCmdExecute(ICmdParam& cmd)
	{
		arr_1t<String> files;
		ObjectNodeGroup* p=GetGroup();
		if(!p) return true;
		IWnd_treectrl::m_tData.window->DeleteChildren(p->GetId());
		p->clear();
		return true;
	}
};


class EvtCommandWorkspaceAddFolder: public EvtCommandTreeCtrl
{
public:
	EvtCommandWorkspaceAddFolder():EvtCommandTreeCtrl(_kT("Wksp.AddFolder")){}

	bool DoCmdExecute(ICmdParam& cmd)
	{
		ObjectNodeGroup* g=GetGroup();
		if(!g) return true;

		String s=::wxGetTextFromUser("folder name").c_str().AsChar();
		if(s=="") return true;

		IWnd_treectrl::m_tData.window->ExAppendNode(new ObjectNodeFolder(s),g);
		return true;
	}
};

class EvtCommandWorkspaceOpenFile : public EvtCommand
{
public:
	EvtCommandWorkspaceOpenFile():EvtCommand(_kT("Wksp.Open")){}

	void DoUpdateCtrl(IUpdParam& upd)
	{
		ObjectNodeFile *fp=dynamic_cast<ObjectNodeFile*>(IWnd_treectrl::m_tData.pnode.get());
		flags.set(FLAG_DISABLE,!fp);
		if(fp)
		{
			m_sLabel=MvcBook::current().IsActivated(fp->GetLabel())?_hT("Activate"):_hT("Open");
		}

		EvtCommand::DoUpdateCtrl(upd);
	}

	bool DoCmdExecute(ICmdParam& cmd)
	{
		ObjectNodeFile *fp=dynamic_cast<ObjectNodeFile*>(IWnd_treectrl::m_tData.pnode.get());
		if(!fp) return false;
		fp->OnTreeEvent(ObjectNode::TREE_ACTIVATE,IWnd_treectrl::m_tData);
		return true;
	}
};


class EvtCommandWindowWorkspace : public EvtCommandWindow
{
public:

	EvtCommandWindowWorkspace():EvtCommandWindow(_kT("Workspace"))
	{
		WndModel& wm(WndModel::current());
		wxWindow* pw=wm.GetWindow();
		if(!pw)
		{
			return;
		}

		WndProperty wp;
		wp.id(5000);
		wp.size(480,640);
	
		IWnd_treectrl* pWkspTree=new IWnd_treectrl(pw,wp);

		pWkspTree->ExSetRoot(new ObjectNodeWorkspace);
		pWkspTree->ExAppendNode(new ObjectNodeFolder("folder1"));
		pWkspTree->ExAppendNode(new ObjectNodeFolder("folder2"));

		pWkspTree->SetName(m_sId.c_str());
		pWkspTree->SetMinSize(wxSize(120,120));

		SetWindow(pWkspTree);

	}

};

PluginWorkspace::PluginWorkspace(WndManager& w):basetype(w,"Plugin.Workspace")
{

}


bool PluginWorkspace::OnCmdEvent(ICmdParam& cmd,int phase)
{
	if(!cmd.flags.get(IDefs::CMD_HANDLED_OK)) return true;

	if(phase==IDefs::PHASE_POSTCALL)
	{
		if(cmd.evtptr->m_sId=="Open")
		{
			if(cmd.extra=="") return true;

			IWnd_treectrl* pTree=dynamic_cast<IWnd_treectrl*>(EvtManager::current()["Workspace"].GetWindow());
			if(pTree&&!pTree->ExGetRoot()->FindNodeEx(cmd.extra))
			{
				ObjectNodeGroup* gp=pTree->ExGetCurrentFolder();
				pTree->ExAppendNode(new ObjectNodeFile(cmd.extra),gp);
			}
		}
	}
	return true;
}

bool PluginWorkspace::OnAttach()
{

	EvtManager::current()["Open"].AttachListener(this);

	EvtManager& ec(wm.evtmgr);

	ec.gp_beg("MainWindow");
		ec.gp_add(new EvtCommandWindowWorkspace);
	ec.gp_end();

	ec.gp_add(new EvtCommandWorkspaceAddFile);
	ec.gp_add(new EvtCommandWorkspaceAddFolder);
	ec.gp_add(new EvtCommandWorkspaceRemove);
	ec.gp_add(new EvtCommandWorkspaceRemoveChildren);

	ec.gp_beg("Wksp.folder.menu");
		ec.gp_add("Wksp.AddFile");
		ec.gp_add("Wksp.AddFolder");
		ec.gp_add("Wksp.Remove");
		ec.gp_add("Wksp.RemoveChildren");
	ec.gp_end();

	ec.gp_beg("Wksp.menu");
		ec.gp_add("Wksp.AddFile");
		ec.gp_add("Wksp.AddFolder");
		ec.gp_add("Wksp.RemoveChildren");
		ec.gp_add("");
		ec.gp_add(new EvtCommand("Wksp.Load"));
		ec.gp_add(new EvtCommand("Wksp.Save"));
	ec.gp_end();

	ec.gp_beg("Wksp.file.menu");
		ec.gp_add(new EvtCommandWorkspaceOpenFile);
		ec.gp_add("Wksp.Remove");
	ec.gp_end();

	wm.wup.gp_add("View",1);

	return true;
}

IMPLEMENT_IPLUGIN(PluginWorkspace)

EW_LEAVE
