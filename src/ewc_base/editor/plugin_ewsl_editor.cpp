#include "ewc_base/editor/plugin_ewsl_editor.h"
#include "ewc_base/plugin/plugin_manager.h"


#include "ewc_base/mvc/mvc_book.h"
#include "ewc_base/mvc/mvc_model.h"
#include "ewc_base/mvc/mvc_view.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/wnd/wnd_updator.h"
#include "ewc_base/wnd/impl_wx/iwnd_stc.h"
#include "ewc_base/wnd/impl_wx/iwnd_bookbase.h"
#include "ewc_base/evt/evt_option.h"


#include "plugin_ewsl_caret.h"

EW_ENTER


class ICmdProcTextCtrlEwsl : public ICmdProcTextEntryStc
{
public:
	typedef ICmdProcTextEntryStc basetype;

	IWnd_stc& Target;
	MvcView& IView;

	ICmdProcTextCtrlEwsl(IWnd_stc& t,MvcView& v):basetype(t),Target(t),IView(v){}

	bool DoTestId(ICmdParam& cmd)
	{
		switch(cmd.param1)
		{
		case CP_SAVEAS:
			return false;
		case CP_DIRTY:
			return false;
		default:
			return basetype::DoTestId(cmd);
		}
	}

};


class MvcViewEwsl : public MvcViewEx
{
public:
	typedef MvcViewEx basetype;

	EwslIface iface;

	AutoPtrT<wxWindow> m_pMyView;

	MvcViewEwsl(MvcModel& tar):basetype(tar),iface(tar)
	{

	}

	void OnStcChanged()
	{
		WndManager& wm(WndManager::current());

		wm.evtmgr["CmdProc"].UpdateCtrl();
		int line=m_pCanvas->GetCurrentLine();
		int lpos=m_pCanvas->GetCurrentPos();
		int lcol=m_pCanvas->GetColumn(lpos);

		wm.evtmgr["StatusBar"].StdExecuteEx(String::Format("Ln %-5d",line+1),1);		
		wm.evtmgr["StatusBar"].StdExecuteEx(String::Format("Ch %-5d",lcol+1),2);
		//wm.evtmgr["StatusBar"].StdExecuteEx(String::Format("Ps %-5d",lpos+1),3);
	}

	virtual bool OnWndEvent(IWndParam&,int)
	{
		return true;
	}

	LitePtrT<IWnd_bookbase> m_pBook;
	LitePtrT<IWnd_stc> m_pCanvas;

	bool DoActivate(WndManager& wm,int v)
	{
		if(!basetype::DoActivate(wm,v)) return false;
		if(v==0) return true;


		wm.evtmgr["Wnd.Variable"].CmdExecuteEx(v>0?2:-2);
		if(m_pBook) m_pBook->SelPage(v>0?m_pMyView.get():NULL);


		wm.wup.mu_set("MenuBar.default");
		wm.wup.gp_add("OtherWindow");
	
		wm.wup.sb_set(v>0?"StatusBar.texteditor":"StatusBar.default");


		return true;
	}




	bool OnCreated()
	{
		m_pBook=dynamic_cast<IWnd_bookbase*>(WndManager::current().evtmgr["Wnd.Variable"].GetWindow());
		if(m_pBook)
		{
			m_pMyView.reset(iface.model->CreateDataView(m_pBook));
		}
		return true;
	}

	void OnDestroy()
	{
		basetype::OnDestroy();
	}

	wxWindow* CreateCanvas(wxWindow* w)
	{
		WndProperty wp;
		m_pCanvas=new EwslCaret(w,wp,iface);
		m_pCanvas->func.bind(&MvcViewEwsl::OnStcChanged,this);

		m_pCanvas->UpdateStyle(".ews");
		m_pCmdProc.reset(new ICmdProcTextCtrlEwsl(*m_pCanvas,*this));
		return m_pCanvas;
	}
};



DataPtrT<MvcModel> PluginEwslEditor::CreateSampleModel()
{
	return new MvcModelT<MvcViewEwsl>;
}


IWnd_stc* PluginEwslEditor::GetStc()
{
	MvcView* p=wm.book.GetActiveView();
	if(!p) return NULL;
	return dynamic_cast<IWnd_stc*>(p->GetCanvas());
}

bool PluginEwslEditor::OnCmdEvent(ICmdParam& cmd,int phase)
{
	if(phase>0 && cmd.evtptr && cmd.evtptr->m_sId=="Ewsl.Done")
	{
		MvcView* p=wm.book.GetActiveView();
		MvcViewEwsl* plab=dynamic_cast<MvcViewEwsl*>(p);
		if(!p) return true;

		plab->iface.OnExecuteDone(cmd.param1);
	}

	if(phase>0 && cmd.evtptr && cmd.evtptr->m_sId=="Ewsl.Exec")
	{
		MvcViewEwsl* plab=NULL;

		plab=dynamic_cast<MvcViewEwsl*>(wm.book.GetActiveView());
		if(!plab)
		{			
			EvtManager::current()["New.Ewsl"].CmdExecuteEx(1);
			plab=dynamic_cast<MvcViewEwsl*>(wm.book.GetActiveView());
			if(!plab) return true;
		}



		wm.evtmgr["Ewsl.Script"].WndExecuteEx(IDefs::ACTION_TRANSFER2MODEL);
		String val;
		wm.evtmgr.table["Ewsl.Script"].get<String>(val);
		plab->iface.PendingExecute(val);
	}

	if(phase>0 && cmd.evtptr && cmd.evtptr->m_sId=="Ewsl.Clear")
	{
		wm.evtmgr.table["Ewsl.Script"].reset<String>("");
		wm.evtmgr["Ewsl.Script"].WndExecuteEx(IDefs::ACTION_TRANSFER2WINDOW);
	}


	return true;
}



class WndModelEwslScript: public WndModel
{
public:
	typedef WndModel basetype;
	
	WndModelEwslScript(WndManager& wm):WndModel("Model.EwslScript")
	{
		flags.add(FLAG_NO_CLOSE);
	}

	bool Create()
	{

		WndMaker km(this);

		km.propotion(1).flags(km.IWND_EXPAND).size(540,280).sv(1);
		km.width( 96).sv(2);

		km.win("dialog"			,km.label("Ewsl.Script").flags(km.IWND_NO_CLOSE|km.IWND_AUTO_FIT).sprops("icon","Find"));
			km.win("row");
				km.add("stc"	,km.ld(1).name("Ewsl.Script"));
				km.win("col"	,km.flags(km.IWND_EXPAND));
					km.add("space"	,km.propotion(1));
					km.add("button"	,km.ld(2).name("Ewsl.Exec"));
					km.add("button"	,km.ld(2).name("Ewsl.Clear"));
				km.end();
			km.end();
		km.end();		

		return true;
	}
};


bool PluginEwslEditor::OnAttach()
{

	EvtManagerTop& ec(wm.evtmgr);

	ec.link_a<String>("Ewsl.Script");
	ec.table["Ewsl.Script"].reset<String>("");

	ec.append(new EvtCommand("Ewsl.Exec"));
	ec.append(new EvtCommand("Ewsl.Clear"));

	AttachEvent("Ewsl.Exec");
	AttachEvent("Ewsl.Clear");

	WndModelEwslScript* pmdl=new WndModelEwslScript(wm);
	ec.append(pmdl);

	ec.append(new EvtCommand("Ewsl.Done"));
	AttachEvent("Ewsl.Done");

	ec.gp_beg("OtherWindow");
		ec.gp_add(new EvtCommandExtraWindow("Wnd.Variable","bookbase"));
		ec.gp_add(new EvtCommandShowModel("Ewsl.Script",pmdl));
	ec.gp_end();

	ec["Wnd.Variable"].flags.add(EvtBase::FLAG_CHECKED);

	ec.gp_beg("Menu.New");
		ec.gp_add(new EvtCommandNewEditorFile(*this,"New.Ewsl"));
	ec.gp_end();

	return true;
}


bool PluginEwslEditor::OnCfgEvent(int lv)
{

	return true;
}

PluginEwslEditor::PluginEwslEditor(WndManager& w):basetype(w,"Plugin.Ewsl")
{

}

IMPLEMENT_IPLUGIN(PluginEwslEditor)

EW_LEAVE
