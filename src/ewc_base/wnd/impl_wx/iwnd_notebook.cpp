#include "ewc_base/wnd/impl_wx/iwnd_notebook.h"


EW_ENTER


IWnd_notebook::IWnd_notebook(wxWindow* p,const WndPropertyEx& h)
:wxNotebook(p,h.id(),h,h,h.flag_nb_dir())
{
	this->Connect(wxEVT_NOTEBOOK_PAGE_CHANGING,wxBookCtrlEventHandler(IWnd_notebook::OnNBChanging));
	this->Connect(wxEVT_NOTEBOOK_PAGE_CHANGED,wxBookCtrlEventHandler(IWnd_notebook::OnNBChanged));
	flags.set(ValidatorGroupBook::FLAG_PAGE_ONLY,h.flags().get(IDefs::IWND_PAGEONLY));
}

void IWnd_notebook::OnNBChanging(wxBookCtrlEvent& evt)
{
	if(!m_pVald) return;

	if(!m_pVald->WndExecuteEx(IDefs::ACTION_SELECTION_CHANGING,evt.GetSelection(),evt.GetOldSelection()))
	{
		evt.Veto();
	}
}

void IWnd_notebook::OnNBChanged(wxBookCtrlEvent& evt)
{
	if(!m_pVald) return;
	m_pVald->WndExecuteEx(IDefs::ACTION_SELECTION_CHANGED,evt.GetSelection(),evt.GetOldSelection());
	m_pVald->ISelPage(this->GetPage(evt.GetSelection()));
}


bool IWnd_notebook::IAddPage(wxWindow *w, const WndProperty& wp)
{
	if (m_pVald) m_pVald->IAddPage(w, wp);
	return wxNotebook::AddPage(w, str2wx(wp.page()));
}

template<>
class ValidatorW<IWnd_notebook> : public ValidatorGroupBook
{
public:

	LitePtrT<IWnd_notebook> pWindow;

	ValidatorW(IWnd_notebook* w_):pWindow(w_)
	{
		flags = pWindow->flags;
		pageid = flags.get(FLAG_PAGE_ONLY) ? 0 : -1;
		pWindow->m_pVald.reset(this);
	}

	virtual bool WndExecute(IWndParam& cmd)
	{
		if(cmd.action==IDefs::ACTION_SELECTION_CHANGED && pageid>=0) pageid=cmd.param1;
		return true;
	}

	virtual bool DoSetValue(int32_t v)
	{
		pWindow->SetSelection(v);
		if(pageid>=0 && v>=0) pageid=v;
		return true;
	}

	virtual bool DoGetValue(int32_t& v)
	{
		v=pWindow->GetSelection();
		return true;
	}

};



template<>
class WndInfoT<IWnd_notebook> : public WndInfoBaseT<IWnd_notebook> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_notebook>(s)
	{

	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<int32_t>* p)
	{
		return CreateValidatorBaseT(w,p);
	}

	Validator* CreateValidator(wxWindow* w)
	{
		return new ValidatorW<IWnd_notebook>((IWnd_notebook*)w);
	}

	void Update(WndMaker& wk,wxWindow* pw,EvtBase* pv)
	{
		wk.vd_set(CreateValidatorEx(pw,pv));
	}

	//void Update(WndBuilder& builder,wxWindow* pwin)
	//{
	//	String name=wx2str(pwin->GetName());
	//	DataPtrT<Validator> p(CreateValidatorEx(pwin,EvtManager::current().get(name)));
	//	ValidatorW<IWnd_notebook>* vbk=dynamic_cast<ValidatorW<IWnd_notebook>*>(p.get());

	//	if(vbk)
	//	{
	//		builder.add(vbk);

	//		for(size_t i=0;i<vbk->pWindow->GetPageCount();i++)
	//		{
	//			DataPtrT<ValidatorGroup> page_group(new ValidatorGroup);
	//			builder.push(page_group.get());

	//			WndInfoManger::current().Update(builder,vbk->pWindow->GetPage(i));
	//			if(page_group->size()==0)
	//			{
	//				vbk->append(new Validator);
	//			}
	//			else if(page_group->size()==1)
	//			{
	//				vbk->append((*page_group)[0]);
	//			}
	//			else
	//			{
	//				vbk->append(page_group.get());
	//			}
	//			builder.pop();
	//		}	

	//	}
	//	else
	//	{
	//		System::LogTrace("Create Validator failed!");
	//	}			
	//}
};


template<>
void WndInfoManger_Register<IWnd_notebook>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_notebook> info(name);
	imgr.Register(&info);
}


EW_LEAVE
