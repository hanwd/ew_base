#include "ewc_base/editor/plugin_webview.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/wnd/impl_wx/iwnd_webview.h"

#include "ewc_base/mvc/mvc_model.h"
#include "ewc_base/mvc/mvc_view.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/wnd/wnd_updator.h"

#include <wx/webview.h>
#include <wx/uri.h>

EW_ENTER



class MvcViewWebView : public MvcViewEx
{
public:
	typedef MvcViewEx basetype;

	LitePtrT<IWnd_webview> m_pCanvas;

	MvcViewWebView(MvcModel& tar):basetype(tar)
	{

	}

	bool DoActivate(WndManager& wm,int v)
	{
		if(!basetype::DoActivate(wm,v)) return false;
		if(v==0) return true;

		wm.evtmgr["Webview.Page"].flags.set(EvtBase::FLAG_HIDE_UI,v<0);

		wm.wup.mu_set("MenuBar.default");
		wm.wup.gp_add("Webview.Page");

		if(v>0) wm.evtmgr["StatusBar"].StdExecuteEx("webview activated",0);

		return true;
	}

	wxWindow* CreateCanvas(wxWindow* w)
	{
		//wxString v1=str2wx();

		m_pCanvas=new IWnd_webview(w,WndProperty());//wxWebView::New(w,wxID_ANY);
		m_pCmdProc.reset(m_pCanvas->GetCmdProc());

		//m_pCmdProc.reset(new ICmdProcWebView(*m_pCanvas,*this));

		//StringBuffer<char> sb;
		//sb.load(this->GetFilename());
		//pCanvas->SetPage(sb.c_str(),"file:///E:/files/");

		m_pCanvas->LoadURL(fn.GetFilename());

		return m_pCanvas;
	}


};

DataPtrT<MvcModel> PluginWebView::CreateSampleModel()
{
	return new MvcModelT<MvcViewWebView>;
}



IWnd_webview* PluginWebView::GetActiveWebView()
{
	if(!MvcView::ms_pActiveView) return NULL;
	return dynamic_cast<IWnd_webview*>(MvcView::ms_pActiveView->GetCanvas());
}

void PluginWebView::LoadURL(const String& url)
{
	if(url=="") return;

	IWnd_webview* wv=GetActiveWebView();
	if(wv)
	{
		wv->LoadURL(url);
	}
	else
	{
		DataPtrT<MvcModel> pModel=CreateModel(url);
		if(!pModel) return;
		this->OpenModel(m_pSampleModel.get());
	}
}



bool PluginWebView::OnCmdEvent(ICmdParam& cmd,int phase)
{
	if(!cmd.evtptr||phase==IDefs::PHASE_POSTCALL) return true;
	if(cmd.evtptr->m_sId=="WebView.Go")
	{
		WndManager::current().evtmgr["WebView.URL"].WndExecuteEx(IDefs::ACTION_TRANSFER2MODEL);
		LoadURL(temp_url);
	}

	if(cmd.evtptr->m_sId=="Webview.Exec")
	{
		IWnd_webview* wv=GetActiveWebView();
		if(!wv) return true;

		wm.evtmgr["Webview.script"].WndExecuteEx(IDefs::ACTION_TRANSFER2MODEL);
		String val;
		wm.evtmgr.table["Webview.script"].get<String>(val);

		wv->RunScript(val);


	}

	if(cmd.evtptr->m_sId=="Webview.Clear")
	{
		wm.evtmgr.table["Webview.script"].reset<String>("");
		wm.evtmgr["Webview.script"].WndExecuteEx(IDefs::ACTION_TRANSFER2WINDOW);
	}

	return true;
}

bool PluginWebView::OnWndEvent(IWndParam& cmd,int phase)
{
	if(phase!=IDefs::PHASE_POSTCALL) return true;
	if(!cmd.iwvptr||!cmd.evtptr||cmd.evtptr->m_sId!="WebView.URL")
	{
		return true;
	}

	if(cmd.action==IDefs::ACTION_TRANSFER2MODEL)
	{
		cmd.iwvptr->DoGetValue(temp_url);
	}
	if(cmd.action==IDefs::ACTION_VALUE_CHANGED)
	{
		cmd.iwvptr->DoGetValue(temp_url);
		LoadURL(temp_url);
	}
	return true;
}



class WndModelWvScript: public WndModel
{
public:
	typedef WndModel basetype;
	
	WndModelWvScript(WndManager& wm):WndModel("Model.WvScript")
	{
		flags.add(FLAG_NO_CLOSE);
	}

	bool Create()
	{

		WndMaker km(this);

		km.propotion(1).flags(km.IWND_EXPAND).size(540,280).sv(1);
		km.width( 96).sv(2);

		km.win("dialog"			,km.label("Webview.Script").flags(km.IWND_NO_CLOSE|km.IWND_AUTO_FIT).sprops("icon","Find"));
			km.win("row");
				km.add("stc"	,km.ld(1).name("Webview.script"));
				km.win("col"	,km.flags(km.IWND_EXPAND));
					km.add("space"	,km.propotion(1));
					km.add("button"	,km.ld(2).name("Webview.Exec"));
					km.add("button"	,km.ld(2).name("Webview.Clear"));
				km.end();
			km.end();
		km.end();		

		return true;
	}
};

bool PluginWebView::OnAttach()
{

	EvtManagerTop& ec(wm.evtmgr);

	ec.link_a<String>("Webview.script");
	ec.table["Webview.script"].reset<String>("");

	ec.append(new EvtCommand("Webview.Exec"));
	ec.append(new EvtCommand("Webview.Clear"));

	AttachEvent("Webview.Exec");
	AttachEvent("Webview.Clear");

	WndModelWvScript* pmdl=new WndModelWvScript(wm);
	ec.append(pmdl);

	ec.gp_beg("OtherWindow");
		ec.gp_add(new EvtCommandShowModel("Webview.Script",pmdl));
	ec.gp_end();

	ec.gp_beg(_kT("Webview.Page"));
		ec.gp_add("Forward");
	ec.gp_end();

	ec.gp_beg("Menu.Extra");
		ec.gp_add("Webview.Page");
	ec.gp_end();

	ec["Webview.Page"].flags.add(EvtCommand::FLAG_HIDE_UI);

	ec.gp_beg(_kT("tb.WebView"));
		ec.gp_add(new EvtCommandCtrl("WebView.URL","searchctrl",WndProperty().width(320)));
		ec.gp_add(new EvtCommand(_kT("WebView.Go")));
	ec.gp_end();

	AttachEvent("WebView.URL");
	AttachEvent("WebView.Go");

	ec.gp_beg("ToolBar.default");
		ec.gp_add("tb.WebView");
	ec.gp_end();

	return true;
}

PluginWebView::PluginWebView(WndManager& w):basetype(w,"Plugin.WebView")
{
	m_aExtension.insert(".htm");
	m_aExtension.insert(".html");
	m_aExtension.insert(".xml");
}

IMPLEMENT_IPLUGIN(PluginWebView)

EW_LEAVE
