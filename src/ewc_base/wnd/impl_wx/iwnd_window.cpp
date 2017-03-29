

#include "ewc_base/app/app.h"
#include "ewc_base/app/res_manager.h"
#include "ewc_base/wnd/impl_wx/topwindow.h"
#include "ewc_base/wnd/wnd_model.h"


EW_ENTER


bst_set<wxWindow*> g_setTopWindow;
int32_t g_numTopWindow=0;
int32_t g_numTopFrame=0;

template<typename T>
bool IWnd_topwindow<T>::Show(bool f)
{

	if(f==this->IsShown())
	{
		return true;
	}

	if(!flags.get(IDefs::IWND_WINDOW_INITED))
	{

		if(!UpdateModel())
		{
			return false;
		}
		if(!m_pModel->WndExecuteEx(IDefs::ACTION_WINDOW_INIT))
		{
			return false;
		}

		flags.add(IDefs::IWND_WINDOW_INITED);
	}
	else if(!m_pModel)
	{
		return T::Show(f);
	}


	if(!m_pModel->WndExecuteEx(f?IDefs::ACTION_WINDOW_BEFORE_SHOW:IDefs::ACTION_WINDOW_BEFORE_HIDE))
	{
		return false;
	}

	if(!T::Show(f))
	{
		return false;
	}

	g_numTopWindow+=(f?+1:-1);
	if(tl::is_same_type<T,wxFrame>::value)
	{
		g_numTopFrame+=(f?+1:-1);
	}

	m_pModel->WndExecuteEx(f?IDefs::ACTION_WINDOW_AFTER_SHOW:IDefs::ACTION_WINDOW_AFTER_HIDE);

	return true;
}

template<typename T>
void IWnd_topwindow<T>::OnActivate(wxActivateEvent &evt)
{
	//if(m_pModelTop)
	//{
	//	m_pModelTop->wm.Activate();
	//}
	evt.Skip(true);
}


template<typename T>
void IWnd_topwindow<T>::OnDropDown(wxCommandEvent& evt)
{

	int evtid=evt.GetId();
	if(EvtBase* pevt=m_pModel->local_evtmgr.chained_get(evtid))
	{
		AutoPtrT<IEW_Ctrl> pctrl(pevt->CreateCtrl(ICtlParam("menu")));
		if (!pctrl || !pctrl->WndIsOk() || !AppData::current().popup_dropdown_menu) return;
		AppData::current().popup_dropdown_menu(this, pctrl->GetMenu());
		AppData::current().popup_dropdown_menu.clear();
	}
}

template<typename T>
void IWnd_topwindow<T>::OnCloseFrame(wxCloseEvent& evt)
{

	if(flags.get(IDefs::IWND_WINDOW_INITED))
	{
		if(!m_pModel->WndExecuteEx(IDefs::ACTION_WINDOW_FINI))
		{
			return;
		}
	}

	if(amgr.GetManagedWindow()!=NULL)
	{
		amgr.UnInit();
	}
	
	if(this->IsShown() && !App::TestDestroy() && (tl::is_same_type<T,wxFrame>::value?g_numTopFrame==1:g_numTopWindow==1))
	{
		App::ReqExit();

		bst_set<wxWindow*> _aPendingDestroy;
		for(bst_set<wxWindow*>::iterator it=g_setTopWindow.begin();it!=g_setTopWindow.end();++it)
		{
			wxWindow* pwin=(*it);
			if(pwin==this) continue;
			if(pwin->GetParent()!=NULL) continue;

			_aPendingDestroy.insert(pwin);
		}
		for(bst_set<wxWindow*>::iterator it=_aPendingDestroy.begin();it!=_aPendingDestroy.end();++it)
		{
			wxWindow* pwin=*it;
			wxCloseEvent evt(wxEVT_CLOSE_WINDOW, pwin->GetId());
			evt.SetEventObject(pwin);
			evt.SetCanVeto(false);
			pwin->GetEventHandler()->AddPendingEvent(evt);
		}
	}

	SetValidatorTop(NULL);	
	T::Show(false);

	Destroy();

}

wxWindow* IWnd_get_topwindow(wxWindow* w)
{
	return w;
}


template<typename T>
IWnd_topwindow<T>::IWnd_topwindow(wxWindow* w,const WndPropertyEx& h,int f)
	:T(
	IWnd_get_topwindow(w),
	h.id(),
	h.label(),
	h,
	h,
	f
	),
	flags(h.m_tFlags)
{

	String icon_id=h.WndProperty::icon();
	if(icon_id=="") icon_id="AppIcon";

	wxIconBundle iconbundle;
	if(ResManager::current().icons.get(icon_id).update(iconbundle))
	{
		SetIcons(iconbundle);
	}
	

	g_setTopWindow.insert(this);
	this->Connect(wxID_ANY,wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(IWnd_topwindow::OnCommandEvent));
	this->Connect(wxID_ANY,wxEVT_MENU,wxCommandEventHandler(IWnd_topwindow::OnCommandEvent));
	this->Connect(wxID_ANY,wxEVT_TOOL,wxCommandEventHandler(IWnd_topwindow::OnCommandEvent));
	this->Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,wxEVT_CLOSE_WINDOW,wxCloseEventHandler(IWnd_topwindow::OnCloseFrame));
	this->Connect(wxID_ANY,wxEVT_ACTIVATE,wxActivateEventHandler(IWnd_topwindow::OnActivate));
	this->Connect(wxID_ANY,AppData::current().evt_user_dropdown_menu, wxCommandEventHandler(IWnd_topwindow::OnDropDown));

}


template<typename T>
IWnd_topwindow<T>::~IWnd_topwindow()
{
	
	if(m_pVald)
	{
		SetValidatorTop(NULL);
	}

	g_setTopWindow.erase(this);

	if(g_setTopWindow.empty())
	{
		App::ExitLoop();
	}

}


template<typename T>
bool IWnd_topwindow<T>::UpdateModel()
{
	if(m_pModel)
	{
		System::LogMessage("IWnd_topwindow<T>::UpdateModel: m_pModel already set");
		return true;
	}

	if(!m_pVald)
	{
		System::LogMessage("IWnd_topwindow<T>::UpdateModel: m_pVald is not set");
		return false;
	}

	m_pModel=m_pVald->GetModel();
	m_pModelTop=dynamic_cast<WndModelTop*>(m_pModel.get());

	if(!m_pModel)
	{
		System::LogMessage("IWnd_topwindow<T>::UpdateModel: m_pVald->GetModel() is NULL");
		return false;
	}

	if(flags.get(IDefs::IWND_AUTO_FIT))
	{
		m_pModel->flags.add(IDefs::IWND_AUTO_FIT);
	}
	if(flags.get(IDefs::IWND_NO_CLOSE))
	{
		m_pModel->flags.add(IDefs::IWND_NO_CLOSE);
	}

	return true;
}


template<typename T>
void IWnd_topwindow<T>::OnCommandEvent(wxCommandEvent& evt)
{
	if(!m_pModel) return;
	m_pModel->OnCmdExecute(evt.GetId(),evt.GetInt());
}

template<typename T>
bool IWnd_topwindow<T>::TransferDataFromWindow()
{
	if(!T::TransferDataFromWindow())
	{
		return false;
	}
	if(!m_pVald) return true;
	return m_pVald->WndExecuteEx(IDefs::ACTION_TRANSFER2MODEL);
}

template<typename T>
bool IWnd_topwindow<T>::TransferDataToWindow()
{
	if(!T::TransferDataToWindow())
	{
		return false;
	}
	if(!m_pVald) return true;
	return m_pVald->WndExecuteEx(IDefs::ACTION_TRANSFER2WINDOW);
}

template<typename T>
bool IWnd_topwindow<T>::Validate()
{
	if(!T::Validate())
	{
		return false;
	}
	if(!m_pVald) return true;
	return m_pVald->WndExecuteEx(IDefs::ACTION_VALIDATE);
}


template<typename T>
bool IWnd_topwindow<T>::IsModified()
{
	if(!m_pVald) return false;
	return !m_pVald->WndExecuteEx(IDefs::ACTION_TEST_NOT_DIRTY);
}

template<typename T>
void IWnd_topwindow<T>::SetValidatorTop(ValidatorTop* p)
{
	if(m_pVald)
	{
		m_pVald->pWindow=NULL;
	}

	m_pVald.reset(p);
	if(!p)
	{
		m_pModel.reset(NULL);
		m_pModelTop.reset(NULL);
	}
}


template class DLLIMPEXP_EWC_BASE IWnd_topwindow<wxDialog>;
template class DLLIMPEXP_EWC_BASE IWnd_topwindow<wxFrame>;


EW_LEAVE

