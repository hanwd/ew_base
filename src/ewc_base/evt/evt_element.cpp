#include "ewc_base/evt/evt_element.h"
#include "ewc_base/evt/validator.h"
#include "ewc_base/wnd/wnd_info.h"

EW_ENTER


bool EvtProxyBase::DoWndExecute(IWndParam& cmd)
{
	if(cmd.action==IDefs::ACTION_TRANSFER2MODEL)
	{
		return InternalTransfer2Model();
	}
	else if(cmd.action==IDefs::ACTION_TRANSFER2WINDOW)
	{
		return InternalTransfer2Window();
	}
	else if(cmd.action==IDefs::ACTION_VALUE_CHANGED)
	{

		if(!flags.get(FLAG_VALUEBIND)) return true;
		if(!InternalTransfer2Model()) return false;
		WndExecuteEx(IDefs::ACTION_TRANSFER2WINDOW);
		return true;
	}
	return true;
}

EvtListener* FindNearestListener(EvtListenerGroup& alis);

bool EvtProxyBase::WndExecute(IWndParam& cmd)
{
	if (cmd.action != IDefs::ACTION_TRANSFER2MODEL)
	{
		return basetype::WndExecute(cmd);
	}
	

	EvtListener* plistener=FindNearestListener(this->m_aAttachedListeners);

	if (plistener) plistener->OnWndEvent(cmd, IDefs::PHASE_PRECALL);
	cmd.flags.set(IDefs::CMD_HANDLED_OK, DoWndExecute(cmd));
	if (plistener) plistener->OnWndEvent(cmd, IDefs::PHASE_POSTCALL);


	return cmd.flags.get(IDefs::CMD_HANDLED_OK);

}

template<typename T>
Validator* EvtProxyT<T>::CreateValidator(wxWindow* w)
{
	WndInfo* wi=WndInfoManger::current().GetWndInfo(w);
	if(!wi) return NULL;
	return wi->CreateValidator(w,this);
}

template class EvtProxyT<double>;
template class EvtProxyT<int32_t>;
template class EvtProxyT<Variant>;
template class EvtProxyT<String>;


EW_LEAVE
