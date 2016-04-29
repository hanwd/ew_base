#ifndef __H_IMPL_WX_WINDOW__
#define __H_IMPL_WX_WINDOW__

#include "ewc_base/wnd/impl_wx/impl_wx.h"
#include "ewc_base/wnd/wnd_property.h"
#include "ewc_base/evt/evt_element.h"
#include "ewc_base/evt/validator.h"

class wxSearchCtrlBase;
EW_ENTER

template<typename T>
class ValidatorHolderT : public DataPtrT<T>
{
public:

	~ValidatorHolderT()
	{
		if(m_ptr) m_ptr->DetachFromAllEvents();
	}
};

template<>
class LockPolicyDefault<wxWindow>
{
public:
	static void lock(wxWindow& w)
	{
		w.Freeze();
	}

	static void unlock(wxWindow& w)
	{
		w.Thaw();
	}
};

class DLLIMPEXP_EWC_BASE WndPropertyEx : public WndProperty
{
public:

	WndPropertyEx(const WndProperty& o):WndProperty(o){}

	operator wxPoint() const {return wxPoint(x(),y());}
	operator wxSize() const {return wxSize(width(),height());}

	wxString sprops(const String& n) const
	{
		return m_aSProperties[n].c_str();
	}
	
	#define IWINPROPERTY_PROPERTY_EX(X)\
	wxString X() const{return str2wx(m_aSProperties[#X]);}

	IWINPROPERTY_PROPERTY_EX(name);
	IWINPROPERTY_PROPERTY_EX(label);
	IWINPROPERTY_PROPERTY_EX(value);
	IWINPROPERTY_PROPERTY_EX(page);
	IWINPROPERTY_PROPERTY_EX(desc);
	IWINPROPERTY_PROPERTY_EX(hint);
	IWINPROPERTY_PROPERTY_EX(help);
	IWINPROPERTY_PROPERTY_EX(tooltip);

	wxString label1() const;
	wxString label2() const;
	
	int32_t flag_align_h() const
	{
		int v=0;
		if(align()==IDefs::ALIGN_CENTER)
		{
			v|=wxALIGN_CENTER_HORIZONTAL;
		}
		else if(align()==IDefs::ALIGN_RIGHT)
		{
			v|=wxALIGN_RIGHT;
		}
		return v;
	}

	int32_t flag_align_v() const
	{
		int v=0;
		if(valign()==IDefs::ALIGN_MIDDLE)
		{
			v|=wxALIGN_CENTER_VERTICAL;
		}
		else if(valign()==IDefs::ALIGN_BOTTOM)
		{
			v|=wxALIGN_BOTTOM;
		}
		return v;		
	}

	int32_t flag_nb_dir() const;

	int32_t flag_text() const
	{
		int v=flag_align_h();
		if(flags().get(IDefs::IWND_READONLY))
		{
			v|=wxTE_READONLY;
		}
		if(flags().get(IDefs::IWND_MULTILINE))
		{
			v|=wxTE_MULTILINE;
		}
		return v;
	}

	int32_t flag_direction()
	{
		if(flags().get(IDefs::IWND_VERTICAL))
		{
			return wxVERTICAL;
		}
		else
		{
			return wxHORIZONTAL;
		}
	}
};


template<>
class ValidatorW<wxTextEntryBase> : public Validator
{
public:
	LitePtrT<wxTextEntryBase> pWindow;

	ValidatorW(wxTextEntryBase* w):pWindow(w){}

	virtual bool DoSetValue(int32_t v)
	{
		return DoSetValue(String::Format("%d",v));
	}

	virtual bool DoGetValue(int32_t& v)
	{
		String s;
		if(!DoGetValue(s))
		{
			return false;
		}
		if(!s.ToNumber(&v))
		{
			return false;
		}
		return true;
	}

	virtual bool DoSetValue(double v)
	{
		return DoSetValue(String::Format("%g",v));
	}

	virtual bool DoGetValue(double& v)
	{
		String s;
		if(!DoGetValue(s))
		{
			return false;
		}
		if(!s.ToNumber(&v))
		{
			return false;
		}
		return true;
	}

	virtual bool DoSetValue(const String& v)
	{
		pWindow->ChangeValue(str2wx(v));
		return true;
	}

	virtual bool DoGetValue(String& v)
	{
		v=wx2str(pWindow->GetValue());
		return true;
	}
};



class DLLIMPEXP_EWC_BASE ICmdProcTextEntry : public CmdProc
{
public:
	typedef CmdProc basetype;

	wxTextEntryBase& Target;
	ICmdProcTextEntry(wxTextEntryBase& t);

	bool DoExecId(ICmdParam& cmd);
	bool DoTestId(ICmdParam& cmd);
};

template<typename T>
class DLLIMPEXP_EWC_BASE IWnd_controlT : public T
{
public:
	using T::m_pVald;

	IWnd_controlT();

	void OnEnter(wxCommandEvent&)
	{
		if(!m_pVald) return;
		m_pVald->WndExecuteEx(IDefs::ACTION_VALUE_CHANGED);
	}

	void OnChar(wxKeyEvent& evt)
	{
		if(!m_pVald) return;
		m_pVald->WndExecuteEx(IDefs::ACTION_VALUE_CHANGING);
	}


	void OnCommandIntChanged(wxCommandEvent& evt)
	{
		if(!m_pVald) return;
		m_pVald->WndExecuteEx(IDefs::ACTION_VALUE_CHANGED,evt.GetInt());
	}

	void OnContextEditMenu(wxMouseEvent& evt)
	{
		EvtManager& ec(WndManager::current().evtmgr);
		wxMenu* mu=ec["Edit.Mini"].CreateMenu();
		if(mu)
		{
			mu->Enable(ec["Cut"].m_nId,this->CanCut());
			mu->Enable(ec["Copy"].m_nId,this->CanCopy());
			mu->Enable(ec["Paste"].m_nId,this->CanPaste());
			mu->Enable(ec["Undo"].m_nId,this->CanUndo());
			mu->Enable(ec["Redo"].m_nId,this->CanRedo());
			mu->Enable(ec["Delete"].m_nId,this->CanCut());
			mu->Enable(ec["SelectAll"].m_nId,true);
			this->PopupMenu(mu);
			delete mu;
		}
		else
		{
			evt.Skip();
		}
	}


	void OnCommandEditEvent(wxCommandEvent& evt)
	{
		EvtManager& ec(WndManager::current().evtmgr);
		int evtid=evt.GetId();

		if(evtid==ec["Cut"].m_nId)
		{
			this->Cut();
		}
		else if(evtid==ec["Copy"].m_nId)
		{
			this->Copy();
		}
		else if(evtid==ec["Paste"].m_nId)
		{
			this->Paste();
		}
		else if(evtid==ec["Undo"].m_nId)
		{
			this->Undo();
		}
		else if(evtid==ec["Redo"].m_nId)
		{
			this->Redo();
		}
		else if(evtid==ec["Delete"].m_nId)
		{
			this->RemoveSelection();
		}
		else if(evtid==ec["SelectAll"].m_nId)
		{
			this->SelectAll();
		}
		else
		{
			evt.Skip();
		}
	}
};


class IWnd_dialog;
class IWnd_frame;
class IWnd_container;
class IWnd_button;
class IWnd_label;
class IWnd_textctrl;
class IPassword;
class ISearchCtrl;
class IWnd_notebook;
class IWnd_checkbox;
class IWnd_hline;
class IWnd_vline;
class IWnd_cmdlink_button;
class IWnd_gauge;
class IWnd_slider;
class IWnd_spinctrl;
class IWnd_hyperlink;
class IWnd_datectrl;
class IWnd_timectrl;
class IWnd_bitmap;
class IWnd_combo;
class IWnd_dataview;
class IWnd_bookbase;

EW_LEAVE

#endif
