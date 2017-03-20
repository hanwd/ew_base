#include "ewc_base/plugin/plugin_option.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/evt/evt_option.h"
#include "ewc_base/wnd/wnd_updator.h"
#include "ewc_base/wnd/wnd_model.h"
#include "ewc_base/wnd/impl_wx/window.h"
#include "ewc_base/wnd/impl_wx/iwnd_treectrl.h"
#include "ewc_base/wnd/impl_wx/iwnd_bookbase.h"
#include "ewc_base/app/res_manager.h"

EW_ENTER
	
class WndOptionPage : public ObjectNode
{
public:
	EvtOptionPage& Target;

	WndOptionPage(EvtOptionPage& t,IWnd_bookbase* b)
		:ObjectNode(t.MakeLabel())
		,Target(t)
		,m_pBook(b)

	{

	}

	typedef ObjectNode basetype;

	IWnd_bookbase* m_pBook;
	LitePtrT<wxWindow> m_pPage;

	virtual bool OnTreeEvent(enumTreeEventType t,ITreeEventData& d)
	{
		if(!basetype::OnTreeEvent(t,d)) return false;
		if(t==TREE_SELECTED)
		{
			if(!m_pPage)
			{
				m_pPage=Target.CreatePage(m_pBook);
				if(Target.m_pVald)
				{
					Target.m_pVald->WndExecuteEx(IDefs::ACTION_TRANSFER2WINDOW);
					Target.m_pVald->WndExecuteEx(IDefs::ACTION_UPDATECTRL);
				}
			}
			m_pBook->SelPage(m_pPage);

			return true;
		}
		return true;
	}

};


class WndOptionFolder : public ObjectNodeGroup
{
public:

	WndOptionFolder(const String& s):ObjectNodeGroup(s){}

	typedef ObjectNodeGroup basetype;
	virtual bool OnTreeEvent(enumTreeEventType t,ITreeEventData& d)
	{
		if(!basetype::OnTreeEvent(t,d)) return false;
		return true;
	}

};


class WndModelOption : public WndModel
{
public:
	typedef WndModel basetype;

	DataPtrT<ValidatorGroup> m_pValdGroup;
	WndManager& wm;
	WndModelOption(WndManager& wm_):basetype("Model.Option"),wm(wm_)
	{
		flags.add(FLAG_NO_CLOSE);
		m_pValdGroup.reset(new ValidatorGroup);
	}

	IWnd_bookbase* option_book;
	IWnd_treectrl* option_tree;

	void LoadPages(const String& s)
	{
		EvtGroup* gp=wm.evtmgr.get_group(s);
		if(!gp) return;
		LoadChildren(gp);
	}

	void LoadChildren(EvtGroup* gp,ObjectNodeGroup* pd=NULL)
	{
		gp->PrepareItems();
		ObjectNodeGroup* nd=new WndOptionFolder(gp->MakeLabel());
		if(pd)
		{
			option_tree->ExAppendNode(nd,pd);
		}
		else
		{
			nd->flags.add(ObjectNode::FLAG_EXPANDED);
			option_tree->ExSetRoot(nd);
		}


		for(size_t i=0;i<gp->size();i++)
		{
			EvtBase* pe=(*gp)[i].get();
			EvtGroup* sg=dynamic_cast<EvtGroup*>(pe);
			if(sg)
			{
				LoadChildren(sg,nd);
			}
			else
			{
				EvtOptionPage* pg=dynamic_cast<EvtOptionPage*>(pe);
				if(pg)
				{
					option_tree->ExAppendNode(new WndOptionPage(*pg,option_book),nd);
					pg->m_pValdGroup=m_pValdGroup;
				}
			}
		}
	}

	bool WndExecute(IWndParam& cmd)
	{
		if(!basetype::WndExecute(cmd)) return false;
		if(cmd.action==IDefs::ACTION_APPLY)
		{
			wm.LoadConfig();
		}
		return true;
	}


	bool Create()
	{

		WndMaker km(this);

		km.flags(IDefs::IWND_EXPAND).sv(1);
		km.propotion(1).flags(IDefs::IWND_EXPAND).sv(2);

		km.win("dialog",WndProperty().label(_hT("options")).size(800,480));

			option_tree=new IWnd_treectrl(km.icur.hwnd,km.ld(1).size(200,500));		
			option_book=new IWnd_bookbase(km.icur.hwnd,km.ld(2).size(280,500));

			km.row(km.ld(0).flags(IDefs::IWND_EXPAND));
				km.col(km.ld(2));
					km.add(option_tree	,km.ld(1).size(280,500).name("option.page"));
					km.add(option_book	,km.ld(2).size(280,500).name("option.book"));
				km.end();
				km.add("hline",km.ld(0).flags(IDefs::IWND_EXPAND));
				km.col(km.ld(0).flags(IDefs::IWND_EXPAND));
					km.add("space",km.propotion(1));
					km.add("button",km.ld(0).name(_kT("Btn.Ok")));
					km.add("button",km.ld(0).name(_kT("Btn.Apply")));
					km.add("button",km.ld(0).name(_kT("Btn.Reset")));
					km.add("button",km.ld(0).name(_kT("Btn.Cancel")));
				km.end();
			km.end();
		km.end();

		vald_grp->append(m_pValdGroup.get());

		LoadPages("Option.pages");

		return true;
	}

};



PluginOption::PluginOption(WndManager& w):PluginCommon(w,"Plugin.Option")
{

}

bool PluginOption::OnCmdEvent(ICmdParam& cmd,int phase)
{
	return true;
}


bool PluginOption::OnAttach()
{

	wm.evtmgr.append(new WndModelOption(wm));
	wm.evtmgr.append(new EvtCommandShowModel(_kT("Option"),"Model.Option"));

	wm.evtmgr["Option"].flags.del(FLAG_CHECK);

	return true;
}

IMPLEMENT_IPLUGIN(PluginOption)

EW_LEAVE
