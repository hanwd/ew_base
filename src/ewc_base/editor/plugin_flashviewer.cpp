#include "ewc_base/editor/plugin_flashviewer.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/mvc/mvc_model.h"
#include "ewc_base/mvc/mvc_view.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/wnd/impl_wx/iwnd_flash.h"
#include "ewc_base/wnd/wnd_updator.h"

EW_ENTER

class ICmdProcTextFlash : public CmdProc
{
public:
	IWnd_flash& Target;

	ICmdProcTextFlash(IWnd_flash& t):Target(t){}


	virtual bool DoExecId(ICmdParam&)
	{
		return false;
	}

	virtual bool DoTestId(ICmdParam&)
	{
		return false;
	}

};


class MvcViewFlash : public MvcViewEx
{
public:
	typedef MvcViewEx basetype;

	MvcViewFlash(MvcModel& tar):basetype(tar){}

	wxWindow* CreateCanvas(wxWindow* w)
	{
		IWnd_flash* pCanvas= new IWnd_flash(w);
		pCanvas->LoadFile(Target.fn.GetFilename());
		m_pCmdProc.reset(new ICmdProcTextFlash(*pCanvas));
		return pCanvas;	

	}

	bool DoActivate(WndManager& wm,int v)
	{
		if(!basetype::DoActivate(wm,v)) return false;
		if(v>0) wm.evtmgr["StatusBar"].StdExecuteEx(_hT("flashplayer activated"),0);
		return true;

	}
};


PluginFlashViewer::PluginFlashViewer(WndManager& w):basetype(w,"Plugin.FlashViewer")
{
	m_aExtension.insert(".swf");

	EvtManager& ec(wm.evtmgr);
	ec.gp_beg("New");
		ec.gp_add("NewFlashViewer");
	ec.gp_end();
}

DataPtrT<MvcModel> PluginFlashViewer::CreateSampleModel()
{
	return new MvcModelT<MvcViewFlash>;
}


IMPLEMENT_IPLUGIN(PluginFlashViewer)

EW_LEAVE
