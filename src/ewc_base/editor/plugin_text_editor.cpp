#include "ewc_base/plugin/plugin_plugineditor.h"
#include "ewc_base/editor/plugin_text_editor.h"
#include "ewc_base/plugin/plugin_manager.h"

#include "ewc_base/mvc/mvc_book.h"
#include "ewc_base/mvc/mvc_model.h"
#include "ewc_base/mvc/mvc_view.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/wnd/wnd_updator.h"
#include "ewc_base/wnd/impl_wx/iwnd_stc.h"
#include "ewc_base/wnd/impl_wx/iwnd_bookbase.h"
#include "ewc_base/evt/evt_option.h"

EW_ENTER

class ICmdProcTextCtrl : public ICmdProcTextEntryStc
{
public:
	typedef ICmdProcTextEntryStc basetype;

	IWnd_stc& Target;
	MvcView& IView;

	ICmdProcTextCtrl(IWnd_stc& t,MvcView& v):basetype(t),Target(t),IView(v)
	{
		fn = v.fn;
	}

	bool DoLoad(const String& fp)
	{
		if (fp == "")
		{
			return false;
		}

		if (!Target.LoadFile(str2wx(fp)))
		{
			return false;
		}

		return true;
	}

	bool DoSave(const String& fp)
	{
		if (fp == "")
		{
			return false;
		}
		wxString text = Target.GetValue();

		StringBuffer<char> buff;
		buff = wx2str(text);

		return buff.save(fp);

		//if (!Target.SaveFile(str2wx(fp)))
		//{
		//	return false;
		//}
		//return true;
	}


	bool DoExecId(ICmdParam& cmd)
	{
		if(cmd.param1==CP_SAVE_POST)
		{
			if (!basetype::DoExecId(cmd))
			{
				return false;
			}
			Target.SetSavePoint();
			return true;
		}
		else
		{
			return basetype::DoExecId(cmd);
		}
	}
};

class MvcViewText : public MvcViewEx
{
public:
	typedef MvcViewEx basetype;
	MvcViewText(MvcModel& tar):basetype(tar)
	{

	}

	void OnStcChanged()
	{
		WndManager& wm(WndManager::current());

		wm.evtmgr["CmdProc"].UpdateCtrl();
		int line=m_pCanvas->GetCurrentLine();
		int lpos=m_pCanvas->GetColumn(m_pCanvas->GetCurrentPos());

		wm.evtmgr["StatusBar"].StdExecuteEx(String::Format("Ln %-5d",line+1),1);		
		wm.evtmgr["StatusBar"].StdExecuteEx(String::Format("Ch %-5d",lpos+1),2);
	}

	virtual bool OnWndEvent(IWndParam&,int)
	{
		return true;
	}

	bool DoActivate(WndManager& wm,int v)
	{
		if(!basetype::DoActivate(wm,v)) return false;
		if(v==0) return true;

		wm.evtmgr["Text"].flags.set(EvtBase::FLAG_HIDE_UI,v<0);

		wm.wup.mu_set("MenuBar.default");
		wm.wup.gp_add("OtherWindow");
		wm.wup.gp_add("Text");

		wm.wup.sb_set(v>0?"StatusBar.texteditor":"StatusBar.default");

		return true;
	}


	LitePtrT<IWnd_stc> m_pCanvas;

	void OnDestroy()
	{
		basetype::OnDestroy();
	}

	wxWindow* CreateCanvas(wxWindow* w)
	{

		WndProperty wp;
		m_pCanvas=new IWnd_stc(w,wp);
		//m_pCanvas->style.add(IWnd_stc::STYLE_CAN_FIND|IWnd_stc::STYLE_CAN_REPLACE);
		
		//String fn=Target.fn.GetFilename();
		//if(fn!="")
		//{
		//	m_pCanvas->LoadFile(str2wx(fn));
		//}

		fn.SetExts(_hT("Text Files")+"(*.txt) | *.txt");

		m_pCanvas->tempp=IWnd_stc::ms_param;
		m_pCanvas->UpdateStyle(Target.fn.GetFilename());

		m_pCanvas->func.bind(&MvcViewText::OnStcChanged,this);
		m_pCmdProc.reset(new ICmdProcTextCtrl(*m_pCanvas,*this));
		m_pCmdProc->fn = fn;

		return m_pCanvas;
	}
};



class EvtStcLang : public EvtCommand
{
public:
	typedef EvtCommand basetype;

	PluginTextEditor& Target;

	int nlang;
	EvtStcLang(PluginTextEditor& t,int nlang_)
		:basetype("Lang."+StcManager::current().langs[nlang_].name)
		,nlang(nlang_)
		,Target(t)
	{
		flags.add(FLAG_CHECK);
	}

	void DoUpdateCtrl(IUpdParam& upd)
	{
		IWnd_stc* p=Target.GetStc();
		flags.set(FLAG_CHECKED,p?p->tempp.nlang==nlang:false);
		flags.set(FLAG_DISABLE,!p);

		basetype::DoUpdateCtrl(upd);
	}

	bool DoCmdExecute(ICmdParam& cmd)
	{
		IWnd_stc* p=Target.GetStc();
		if(!p) return false;
		p->tempp.nlang=nlang;
		p->UpdateStyle();
		Target.wm.wup.gp_add("TE.Lang");
		return true;
	}
};

template<int N>
class EvtStcFlagT : public EvtCommand
{
public:
	typedef EvtCommand basetype;

	PluginTextEditor& Target;

	EvtStcFlagT(PluginTextEditor& t,const String& s)
		:EvtCommand(s)
		,Target(t)
	{
		flags.add(FLAG_CHECK);
	}

	void DoUpdateCtrl(IUpdParam& upd)
	{
		IWnd_stc* p=Target.GetStc();

		flags.set(FLAG_CHECKED,p?p->tempp.flags.get(N):false);
		flags.set(FLAG_DISABLE,!p);

		basetype::DoUpdateCtrl(upd);
	}

	bool DoCmdExecute(ICmdParam& cmd)
	{
		IWnd_stc* p=Target.GetStc();
		if(!p) return false;
		p->tempp.flags.inv(N);
		p->UpdateStyle();
		return true;
	}
};


template<int N>
class EvtStcFontSizeT : public EvtCommand
{
public:
	typedef EvtCommand basetype;

	PluginTextEditor& Target;

	EvtStcFontSizeT(PluginTextEditor& t)
		:EvtCommand(String::Format("Text.Fontsize.%d",N))
		,Target(t)
	{
		flags.add(FLAG_CHECK);
	}

	void DoUpdateCtrl(IUpdParam& upd)
	{
		IWnd_stc* p=Target.GetStc();

		flags.set(FLAG_CHECKED,p&&p->param.nsize==N);
		flags.set(FLAG_DISABLE,!p);

		basetype::DoUpdateCtrl(upd);
	}

	bool DoCmdExecute(ICmdParam& cmd)
	{
		IWnd_stc* p=Target.GetStc();
		if(!p) return false;
		p->SetFontSize(N);

		Target.wm.wup.gp_add("Text.FontSize");
		return true;
	}
};


DataPtrT<MvcModel> PluginTextEditor::CreateSampleModel()
{
	return new MvcModelT<MvcViewText>;
}


IWnd_stc* PluginTextEditor::GetStc()
{
	MvcView* p=wm.book.GetActiveView();
	if(!p) return NULL;
	return dynamic_cast<IWnd_stc*>(p->GetCanvas());
}




class EvtOptionTextEditor : public EvtOptionPage
{
public:
	typedef EvtOptionPage basetype;

	EvtOptionTextEditor(const String& s):basetype(s)
	{
	
	}
	
	void DoCreatePage(WndMaker& km)
	{

		km.border(2).sv(0);
		km.width(120).flags(IDefs::IWND_WITHCOLON).align(IDefs::ALIGN_RIGHT).sv(1);
		km.propotion(1).sv(2);
		km.flags(IDefs::IWND_EXPAND).sv(3);

		km.win("container");
			km.row();
				km.col(km.ld(3));
					km.add("label"		,km.ld(1).label(_hT("fontsize")));
					km.add("textctrl"	,km.ld(2).name("/texteditor/fontsize"));
				km.end();
				km.col(km.ld(3));
					km.add("label"		,km.ld(1).label(_hT("tab_size")));
					km.add("textctrl"	,km.ld(2).name("/texteditor/tab_size"));
				km.end();
				km.col(km.ld(3));
					km.add("label"		,km.ld(1).label(_hT("flags")));
					km.col();
						km.add("checkbox"	,km.name("/texteditor/fold").label("fold"));
						km.add("checkbox"	,km.name("/texteditor/wrap").label("wrap"));
					km.end();
				km.end();
			km.end();
		km.end();
	}

};
bool PluginTextEditor::OnAttach()
{

	EvtManager& ec(wm.evtmgr);

	ec.gp_beg("StatusBar.texteditor","StatusBar");
		ec.gp_add("",-1);
		ec.gp_add("",60);
		ec.gp_add("",60);
		ec.gp_add("",60);
	ec.gp_end();

	ec.gp_beg("Menu.New");
		ec.gp_add(new EvtCommandNewEditorFile(*this,_kT("New.TextFile")));
	ec.gp_end();


	ec.gp_beg(_kT("TE.Lang"));
	for(size_t i=0;i<StcManager::current().langs.size();i++)
	{
		ec.gp_add(new EvtStcLang(*this,i));
	}
	ec.gp_end();


	ec.gp_beg(_kT("Text"));
		ec.gp_add("TE.Lang");

		ec.gp_add("");
		ec.gp_add(new EvtStcFlagT<IWnd_stc::FLAG_SYNTAX>(*this,_kT("TE.Syntax")));
		ec.gp_add(new EvtStcFlagT<IWnd_stc::FLAG_FOLD>(*this,_kT("TE.Fold")));
		ec.gp_add(new EvtStcFlagT<IWnd_stc::FLAG_WRAPMODE>(*this,_kT("TE.Wrap")));
		ec.gp_add("");
		ec.gp_add(new EvtStcFlagT<IWnd_stc::FLAG_LINENUM>(*this,_kT("TE.LineNum")));
		ec.gp_add(new EvtStcFlagT<IWnd_stc::FLAG_INDENTGUIDE>(*this,_kT("TE.IndentGuide")));
		ec.gp_add(new EvtStcFlagT<IWnd_stc::FLAG_LONGLINE>(*this,_kT("TE.LongLine")));
		ec.gp_add(new EvtStcFlagT<IWnd_stc::FLAG_WHITESPACE>(*this,_kT("TE.WhiteSpace")));
		ec.gp_add(new EvtStcFlagT<IWnd_stc::FLAG_SHOWEOL>(*this,_kT("TE.ShowEOL")));
		ec.gp_add("");
		ec.gp_add(new EvtStcFlagT<IWnd_stc::FLAG_READONLY>(*this,_kT("TE.ReadOnly")));
		ec.gp_add(new EvtStcFlagT<IWnd_stc::FLAG_OVERTYPE>(*this,_kT("TE.OverWrite")));
		//ec.gp_add(new EvtStcFlagT<IWnd_stc::FLAG_INDENT>(*this,_kT("TE.Indent")));
	ec.gp_end();

	ec.gp_beg("Menu.Extra");
		ec.gp_add("Text");
	ec.gp_end();

	ec["Text"].flags.add(EvtCommand::FLAG_HIDE_UI);

	wm.evtmgr.link_c<int64_t>("/texteditor/fontsize");
	wm.evtmgr.link_c<int64_t>("/texteditor/tab_size");

	wm.evtmgr.link_c<int64_t>("/texteditor/wrap");
	wm.evtmgr.link_c<int64_t>("/texteditor/fold");

	wm.evtmgr.gp_beg(_kT("Option.pages"));
		//wm.evtmgr.gp_add(new EvtOptionTextEditor(_kT("Option.TextEditor")));
		wm.evtmgr.gp_add(new EvtOptionPageScript(_kT("Option.TextEditor"), "scripting/ui/option_text.ewsl"));
	wm.evtmgr.gp_end();

	AttachEvent("Config");

	return true;
}


bool PluginTextEditor::OnCfgEvent(int lv)
{
	wm.app.conf.CfgUpdate(lv,"/texteditor/fontsize",IWnd_stc::ms_param.nsize,3,40);
	wm.app.conf.CfgUpdate(lv,"/texteditor/tab_size",IWnd_stc::ms_param.wdtab,1,12);

	wm.app.conf.CfgUpdate(lv,"/texteditor/fold",IWnd_stc::ms_param.flags,IWnd_stc::FLAG_FOLD);
	wm.app.conf.CfgUpdate(lv,"/texteditor/wrap",IWnd_stc::ms_param.flags,IWnd_stc::FLAG_WRAPMODE);

	return true;
}

PluginTextEditor::PluginTextEditor(WndManager& w):basetype(w,"Plugin.TextEditor")
{
	m_aExtension.insert(".txt");
	m_aExtension.insert(".log");
	m_aExtension.insert(".cpp");
	m_aExtension.insert(".hpp");
	m_aExtension.insert(".sql");
	m_aExtension.insert(".ewsl");
	m_aExtension.insert(".script");

}

IMPLEMENT_IPLUGIN(PluginTextEditor)

EW_LEAVE
