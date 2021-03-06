#include "ewc_base/mvc/mvc_view.h"
#include "ewc_base/mvc/mvc_model.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/wnd/wnd_manager.h"
#include "ewc_base/wnd/wnd_manager.h"

#include "ewc_base/wnd/impl_wx/iwnd_bookbase.h"
#include "ewc_base/data/data_model.h"

EW_ENTER

class MvcViewProxy : public wxEvtHandler
{
public:
	MvcView& view;
	MvcViewProxy(MvcView& view_):view(view_){}

    virtual bool TryBefore(wxEvent& evt)
    {
		if(view.ProcessEventLocally(evt))
		{
			return true;
		}
        return wxEvtHandler::TryBefore(evt);
    }
};

MvcView::MvcView(MvcModel& tar):Target(tar),fn(tar.fn)
{
	Target.AttachView(this);
	Target.IncRef();
}

MvcView* MvcView::ms_pActiveView=NULL;

MvcView::~MvcView()
{
	Target.DetachView(this);
	Target.DecRef();
}

MvcView* MvcView::GetViewByCanvas(wxWindow* w)
{
	if(!w) return NULL;
	MvcViewProxy* k=dynamic_cast<MvcViewProxy*>(w->GetEventHandler());
	return k? &k->view:NULL;
}

MvcView* MvcView::GetViewByHandler(wxEvtHandler* h)
{
	MvcViewProxy* k=dynamic_cast<MvcViewProxy*>(h);
	return k? &k->view:NULL;
}

void MvcView::OnDestroy()
{
	delete this;
}


void MvcView::SetCanvas(wxWindow* w)
{
	if(m_pCanvas==w)
	{
		return;
	}

	if(m_pCanvas)
	{
		m_pCanvas->PopEventHandler(true);
	}

	m_pCanvas=w;

	if(m_pCanvas)
	{
		m_pCanvas->PushEventHandler(new MvcViewProxy(*this));
	}
}

wxWindow* MvcView::CreateCanvas(wxWindow* w)
{
	return new wxWindow(w,wxID_ANY);
}


bool MvcView::IsActive()
{
	return flags.get(FLAG_ACTIVE);
}

bool MvcView::OnCreated()
{
	return true;
}

bool MvcView::OnClose(WndManager& wm)
{
	if(Target.TestId(CmdProc::CP_DIRTY,NULL))
	{
		String fp;
		Target.TestId(CmdProc::CP_SAVEAS, fp);
		String msg = fp.empty() ? "File modified, save?" : String::Format("File \"%s\" modified, save?",fp);

		int ret=Wrapper::MsgsDialog(msg,IDefs::DLG_YES_NO_CANCEL|IDefs::ICON_QUESTION);

		ICmdParam cmd;
		for (cmd.param1 = CmdProc::CP_SAVE; ret == IDefs::BTN_YES; cmd.param1 = CmdProc::CP_SAVEAS)
		{
			if (Target.DoExecId(cmd)) break;
			ret=Wrapper::MsgsDialog("File save failed, try another?",IDefs::DLG_YES_NO_CANCEL|IDefs::ICON_QUESTION);
		}

		if(ret==IDefs::BTN_CANCEL)
		{
			return false;
		}
	}

	if(!DoClose(wm))
	{
		return false;
	}

	if(flags.get(FLAG_ACTIVE))
	{
		if(!OnActivate(wm,-2))
		{
			System::LogMessage("OnActivate(-2) return false!");
		}
	}

	return true;
}

bool MvcView::OnActivate(WndManager& wm,int v)
{
	EW_ASSERT(v!=0);

	if(v>0 && flags.get(FLAG_ACTIVE)) return true;
	if(v<0 &&!flags.get(FLAG_ACTIVE)) return true;

	if(v>0)
	{
		wm.cmdptr.SetData(&Target);
		flags.set(FLAG_ACTIVE,true);
	}

	if(!DoActivate(wm,v))
	{
		return false;
	}

	if(v<0)
	{
		flags.set(FLAG_ACTIVE,false);
		wm.cmdptr.SetData(NULL);
	}
	else if (v > 0)
	{
		UpdateStatus();
	}

	return true;
}

bool MvcView::DoClose(WndManager&)
{
	return true;
}

bool MvcView::DoActivate(WndManager& wm,int v)
{
	if (v>0 && !flags.get(FLAG_INITED))
	{
		if (!DoInitialize())
		{
			return false;
		}

		flags.add(FLAG_INITED);
		Target.ExecId(CmdProc::CP_INIT);
	}

	for (size_t i = 0; i < arr_items.size(); i++)
	{
		if (arr_items[i]->DoActivate(wm, v)) continue;

		if (v != 0)
		{
			System::LogError("invalid DoActivate State");
			return true;
		}

		while (i > 0)
		{
			arr_items[--i]->DoActivate(wm, -v);
		}

		return false;


	}

	return true;
}

void MvcView::UpdateView()
{

}

String MvcView::GetName()
{
	return _hT("view");
}


String MvcView::GetTitle()
{
	String fs=fn.GetFilename();
	if (fs != "")
	{
		fs.replace('\\', '/');
		return string_split(fs, "/").back();
	}
	return _hT("unnamed");
}

bool MvcView::DoInitialize()
{
	return true;
}

void MvcView::Refresh()
{
	if (!IsActive()) return;
	if (m_nRequestFresh++) return;

	WndManager::current().evtmgr.pending("View.Refresh");

}

void MvcView::DoRefresh()
{
	if (!m_pCanvas) return;
	m_pCanvas->Refresh();
	m_nRequestFresh = 0;
}

void MvcView::CaptureMouse()
{
	if(m_pCanvas) m_pCanvas->CaptureMouse();
}
void MvcView::ReleaseMouse()
{
	if(m_pCanvas) m_pCanvas->ReleaseMouse();
}

void MvcView::SetFocus()
{
	if(m_pCanvas) m_pCanvas->SetFocus();
}

MvcViewEx::MvcViewEx(MvcModel& tar):MvcView(tar)
{
	fn.UnShare();
}

bool MvcViewEx::DoActivate(WndManager& wm,int v)
{
	if(v>0) Target.SetData(m_pCmdProc.get());
	if(v<0) Target.SetData(NULL);
	return MvcView::DoActivate(wm,v);
}

class MvcItemBookData : public MvcItemData
{
public:

	String sbook;
	IWnd_bookbase* pbook;
	AutoPtrT<wxWindow> pview;

	bool DoActivate(WndManager& wm, int v)
	{
		if (pbook)
		{
			pbook->SelPage(v > 0 ? pview.get() : NULL);
			wm.evtmgr[sbook].CmdExecuteEx(v>0 ? 2 : -2);
		}
		return true;
	}
};

bool MvcView::LinkBookData(const String& sbook, DataModel* pmodel)
{
	DataPtrT<MvcItemBookData> pdata(new MvcItemBookData);

	pdata->pbook = dynamic_cast<IWnd_bookbase*>(WndManager::current().evtmgr[sbook].GetWindow());
	if (!pdata->pbook) return false;

	pdata->sbook = sbook;
	pdata->pview.reset(pmodel->CreateDataView(pdata->pbook));

	arr_items.push_back(pdata);
	return true;
}


EW_LEAVE
