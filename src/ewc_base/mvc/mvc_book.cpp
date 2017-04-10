#include "ewc_base/mvc/mvc_book.h"
#include "ewc_base/mvc/mvc_view.h"
#include "ewc_base/mvc/mvc_model.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/wnd/wnd_updator.h"
#include <wx/aui/auibook.h>

EW_ENTER


class MvcBookImpl : public wxAuiNotebook
{
public:
	typedef wxAuiNotebook basetype;

	WndManager& wm;

	MvcBookImpl(wxWindow* p,int id,WndManager& app_):wxAuiNotebook(p,id),wm(app_)
	{
		this->Connect(id,wxEVT_AUINOTEBOOK_PAGE_CHANGING,wxAuiNotebookEventHandler(MvcBookImpl::OnCanvasChanging));
		this->Connect(id,wxEVT_AUINOTEBOOK_PAGE_CHANGED,wxAuiNotebookEventHandler(MvcBookImpl::OnCanvasChanged));
		this->Connect(id,wxEVT_AUINOTEBOOK_PAGE_CLOSE,wxAuiNotebookEventHandler(MvcBookImpl::OnCanvasClosing));
		this->Connect(id,wxEVT_AUINOTEBOOK_PAGE_CLOSED,wxAuiNotebookEventHandler(MvcBookImpl::OnCanvasClosed));
		this->Connect(wxID_ANY,wxEVT_CHILD_FOCUS,wxChildFocusEventHandler(MvcBookImpl::OnFocus));
	}

	void OnFocus(wxChildFocusEvent& evt)
	{
		CmdProc* p1=wm.cmdptr.GetData();
		CmdProc* p2=m_pActiveView?&m_pActiveView->Target:NULL;
		if(p1!=p2)
		{
			wm.cmdptr.SetData(p2);
			wm.wup.gp_add("CmdProc");
		}
		evt.Skip(true);
	}

	bool CloseCanvas(wxWindow* w=NULL)
	{
		ICmdParam cmd(-1);
		return CloseCanvas(cmd,w);
	}

	bool CloseCanvas(ICmdParam& cmd,wxWindow* w=NULL)
	{
		wxWindow* pPage=w?w:m_pCurrentCanvas;
		if(!pPage) return false;

		int nSelection=this->GetPageIndex(pPage);
		if(nSelection<0) return false;

		LockGuard<WndUpdator> lock(wm.wup);

		MvcView* pView=MvcView::GetViewByCanvas(pPage);
		if(pView)
		{
			if(!pView->OnClose(wm))
			{
				return false;
			}
			wm.book.DetachView(pView);
		}

		if(m_pCurrentCanvas==pPage)
		{
			m_pCurrentCanvas.reset(NULL);
			SetActiveView(NULL);
		}

		DeletePage(nSelection);
		wm.wup.gp_add("CmdProc");

		return true;
	}

	wxWindow* GetCurrentCanvas(){return m_pCurrentCanvas;}

	bool SetCurrentCanvas(wxWindow* w)
	{
		if(m_pCurrentCanvas.get()==w) return true;

		EW_ASSERT(MvcView::GetViewByCanvas(m_pCurrentCanvas)==m_pActiveView);

		if(m_pActiveView && !m_pActiveView->OnActivate(wm,-1))
		{
			return false;
		}

		m_pCurrentCanvas.reset(w);

		SetActiveView(MvcView::GetViewByCanvas(w));

		if(m_pActiveView)
		{
			m_pActiveView->OnActivate(wm,+2);
		}

		return true;
	}

	void OnCanvasChanging(wxAuiNotebookEvent& evt)
	{
		wm.wup.lock();
		if(!SetCurrentCanvas(NULL))
		{
			evt.Veto();
			wm.wup.unlock();
		}
	}

	void OnCanvasChanged(wxAuiNotebookEvent& evt)
	{

		int sel=evt.GetSelection();
		wxWindow* page=GetPage(sel);
		SetCurrentCanvas(page);

		wm.wup.gp_add("CmdProc");
		wm.wup.unlock();
	}

	void OnCanvasClosed(wxAuiNotebookEvent& evt)
	{
		(void)&evt;

		if(this->GetPageCount()==0)
		{
			m_pCurrentCanvas.reset(NULL);
			SetActiveView(NULL);
			wm.wup.gp_add("CmdProc");
		}

		wm.wup.unlock();
	}

	void SetActiveView(MvcView* pview)
	{
		m_pActiveView=pview;
		MvcView::ms_pActiveView = pview;
	}


	void OnCanvasClosing(wxAuiNotebookEvent& evt)
	{
		wm.wup.lock();

		wxWindow* pPage=GetPage(evt.GetSelection());
		EW_ASSERT(pPage!=NULL);

		MvcView* pView=MvcView::GetViewByCanvas(pPage);
		if(!pView)
		{
			return;
		}

		ICmdParam cmd(-1);
		if(!pView->OnClose(wm))
		{
			evt.Veto();
			wm.wup.unlock();
			return;
		}

		if(pPage==m_pCurrentCanvas)
		{
			m_pCurrentCanvas.reset(NULL);
			SetActiveView(NULL);
		}

		wm.book.DetachView(pView);

	}

	LitePtrT<wxWindow> m_pCurrentCanvas;
	LitePtrT<MvcView> m_pActiveView;
};



class EvtListenerCurrentView : public EvtListener
{
public:

	WndManager& wm;
	EvtListenerCurrentView(WndManager& w):wm(w){}

	virtual bool OnCmdEvent(ICmdParam& cmd,int phase)
	{
		MvcView* pv=wm.book.GetActiveView();
		return pv?pv->OnCmdEvent(cmd,phase):true;
	}

	virtual bool OnStdEvent(IStdParam& cmd,int phase)
	{
		MvcView* pv=wm.book.GetActiveView();
		return pv?pv->OnStdEvent(cmd,phase):true;
	}

	virtual bool OnWndEvent(IWndParam& cmd,int phase)
	{
		MvcView* pv=wm.book.GetActiveView();
		return pv?pv->OnWndEvent(cmd,phase):true;
	}
};

MvcBook::MvcBook()
{
	WndManager& wm(WndManager::current());
	m_pViewListener.reset(new EvtListenerCurrentView(wm));
}

wxAuiNotebook* MvcBook::CreateBook(wxWindow* p)
{
	WndManager& wm(WndManager::current());
	MvcBookImpl* book=new MvcBookImpl(p,wm.evtmgr.id_new(),wm);
	book->SetName("centerpane");
	m_pBook.reset(book);
	return book;
}


bool MvcBook::IsActivated(const String& fn)
{
	for(view_map::iterator it=m_aOpenedViews.begin();it!=m_aOpenedViews.end();++it)
	{
		MvcView* pView = (*it).first;
		if(pView->fn.GetFilename()!=fn) continue;
		wxWindow* w=pView->GetCanvas();
		if(w)
		{
			int sel=m_pBook->GetPageIndex(w);
			if(sel>=0)
			{
				return true;
			}
		}
	}
	return false;
}

bool MvcBook::Activate(const String& fn)
{
	for (view_map::iterator it = m_aOpenedViews.begin(); it != m_aOpenedViews.end(); ++it)
	{
		MvcView* pView = (*it).first;
		if(pView->fn.GetFilename()!=fn) continue;
		wxWindow* w=pView->GetCanvas();
		if(w)
		{
			int sel=m_pBook->GetPageIndex(w);
			if(sel>=0) m_pBook->SetSelection(sel);
		}
		return true;
	}
	return false;
}

MvcView* MvcBook::GetActiveView()
{
	return m_pBook->m_pActiveView;
}



bool MvcBook::AttachView(MvcView* pView)
{
	if(!pView)
	{
		System::LogDebug("MvcBook::AttachView: pView is NULL");
		return false;
	}


	String title = pView->GetTitle();

	wxWindow* pCanvas=pView->CreateCanvas(m_pBook.get());
	pView->SetCanvas(pCanvas);

	bool flag=m_pBook->AddPage(pCanvas,str2wx(title),true,-1);

	if(!flag)
	{
		System::LogMessage("MvcBook::AttachView: m_pBook->AddPage failed");
	}

	return flag;
}



bool MvcBook::DetachView(MvcView* pView)
{
	m_aOpenedViews.erase(pView);

	pView->SetCanvas(NULL);
	pView->OnDestroy();

	return true;
}

void MvcBook::UpdateTitle()
{
	MvcView* pView=m_pBook->m_pActiveView;
	if(!pView) return;


	ViewInfo& vinfo(m_aOpenedViews[pView]);

	String title = pView->GetTitle();
	if (vinfo.title != title)
	{
		vinfo.title = title;
		vinfo.index = -1;
	}

	vinfo.dirty=pView->Target.TestId(CmdProc::CP_DIRTY);

	
	class MyViewInfoLess
	{
	public:
		bool operator()(ViewInfo* lhs, ViewInfo* rhs)
		{
			if (lhs->index < rhs->index) return true;
			if (lhs->index > rhs->index) return false;
			return lhs->timestamp < rhs->timestamp;
		}
	};

	bst_map<String, bst_set<ViewInfo*, MyViewInfoLess> > ainfo;
	for (auto it = m_aOpenedViews.begin(); it != m_aOpenedViews.end(); ++it)
	{
		ainfo[(*it).second.title].insert(&(*it).second);
	}
	for (auto it = ainfo.begin(); it != ainfo.end(); it++)
	{
		bst_set<ViewInfo*, MyViewInfoLess>& ainfo((*it).second);
		if (ainfo.size() == 1)
		{
			(*ainfo.begin())->index = -1;
			continue;
		}

		int n = (*ainfo.rbegin())->index;
		for (auto it = ainfo.begin(); it != ainfo.end() && (*it)->index < 0; ++it)
		{
			(*it)->index = ++n;
		}

	}


	for (view_map::iterator it = m_aOpenedViews.begin(); it != m_aOpenedViews.end(); ++it)
	{
		ViewInfo& vinfo((*it).second);
		String title;
		if (vinfo.dirty) title << "* ";
		else  title << "  ";

		title << vinfo.title;
		if (vinfo.index >= 0)
		{
			title << ":" << vinfo.index;
		}
		if (title == vinfo.title_n) continue;
		vinfo.title_n = title;

		wxWindow* pCanvas = (*it).first->GetCanvas();
		int id = m_pBook->GetPageIndex(pCanvas);
		if (id >= 0)
		{
			m_pBook->SetPageText(id, str2wx(title));
		}
		else
		{
			System::LogTrace("canvas not found");
		}
	}

}

bool MvcBook::OpenModel(MvcModel* pModel)
{
	
	if(!pModel)
	{
		System::LogMessage("MvcBook::OpenModel: pModel is NULL");
		return false;
	}

	if(!m_pBook)
	{
		System::LogMessage("MvcBook::OpenModel: m_pBook is NULL");
		return false;
	}

	
	LockGuard<WndUpdator> lock(WndManager::current().wup);

	AutoPtrT<MvcView> pView;

	pView.reset(pModel->CreateView());
	if(!pView)
	{
		System::LogMessage("MvcBook::OpenModel: pModel->CreateView return NULL");
		return false;
	}

	if(!pView->OnCreated())
	{
		System::LogMessage("MvcBook::OpenModel: pView->Create return false");
		return false;
	}

	if(!AttachView(pView.get())) return false;

	pView.release();
	return true;

}

size_t MvcBook::GetPageCount()
{
	return m_pBook?m_pBook->GetPageCount():0;
}

bool MvcBook::AnyDirty()
{
	size_t n=m_pBook->GetPageCount();
	for(size_t i=0;i<n;i++)
	{
		wxWindow* w=m_pBook->GetPage(i);
		MvcView* v=MvcView::GetViewByCanvas(w);
		if(!v) continue;
		if(v->Target.TestId(CmdProc::CP_DIRTY)) return true;
	}
	return false;
}

void MvcBook::SaveAll()
{
	WndManager& wm(WndManager::current());
	size_t n=m_pBook->GetPageCount();
	for(size_t i=0;i<n;i++)
	{
		wxWindow* w=m_pBook->GetPage(i);
		MvcView* v=MvcView::GetViewByCanvas(w);
		if(!v) continue;
		v->Target.ExecId(CmdProc::CP_DIRTY);
	}

	wm.wup.gp_add("Save");
	wm.wup.gp_add("SaveAll");
}

bool MvcBook::CloseAll()
{
	WndManager& wm(WndManager::current());

	ICmdParam cmd(-1);
	LockGuard<WndUpdator> lock(wm.wup);

	while(1)
	{
		wxWindow* w=m_pBook->GetCurrentCanvas();
		if(!w) break;

		if(!m_pBook->CloseCanvas(cmd,w))
		{
			return false;
		}
	}

	return true;
}

bool MvcBook::Close(int)
{
	return m_pBook->CloseCanvas();
}

EW_LEAVE
