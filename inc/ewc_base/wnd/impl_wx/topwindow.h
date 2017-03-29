#include "ewc_base/wnd/impl_wx/window.h"
#include "ewc_base/evt/validator_top.h"

#include <wx/aui/aui.h>

EW_ENTER

class EvtCtrlGroup : public arr_1t<EvtGroup*>
{
public:

	EvtCtrlGroup(){}
	template<typename X> EvtCtrlGroup(X p1,X p2){gp_add(p1,p2);}

	template<typename X> void gp_add(X p1,X p2){for(X p=p1;p!=p2;++p) gp_add(*p);}

	void gp_add(const EvtItem& item)
	{
		if(item.flags.val())
		{
			EvtGroup* pgrp=EvtManager::current().get_group(item.sname);
			if(!pgrp) return;
			if(pgrp->flags.get(EvtGroup::FLAG_HIDE_UI)) return;
			gp_add(pgrp->items().begin(),pgrp->items().end());
		}
		else
		{
			EvtGroup* pgrp=EvtManager::current().get_group(item.sname);
			if(!pgrp) return;
			if(pgrp->flags.get(EvtGroup::FLAG_HIDE_UI)) return;
			push_back(pgrp);
		}
	}

};

inline bool operator==(const EvtCtrlGroup& lhs,const EvtCtrlGroup& rhs)
{
	if(lhs.size()!=rhs.size()) return false;
	return memcmp(lhs.data(),rhs.data(),lhs.size()*sizeof(void*))==0;
}

template<typename T>
class DLLIMPEXP_EWC_BASE IWnd_topwindow : public T
{
public:

	typedef T basetype;

	IWnd_topwindow(wxWindow* w,const WndPropertyEx& h,int f);
	~IWnd_topwindow();


	void OnCloseFrame(wxCloseEvent& evt);

	void OnActivate(wxActivateEvent &evt);

	void OnDropDown(wxCommandEvent& evt);

	virtual bool UpdateModel();

	bool Show(bool f=true);

	void OnCommandEvent(wxCommandEvent& evt);

	bool TransferDataFromWindow();

	bool TransferDataToWindow();

	bool Validate();

	bool IsModified();

	wxAuiManager amgr;

	EvtCtrlGroup aMenus;
	EvtCtrlGroup aTools;

	void SetValidatorTop(ValidatorTop* p);


protected:

	DataPtrT<ValidatorTop> m_pVald;
	DataPtrT<WndModel> m_pModel;
	LitePtrT<WndModelTop> m_pModelTop;
	BitFlags flags;

};


EW_LEAVE

