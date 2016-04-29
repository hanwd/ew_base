#include "ewc_base/evt/evt_listener.h"
#include "ewc_base/evt/evt_base.h"
#include "ewc_base/mvc/mvc_view.h"
EW_ENTER


bool EvtListener::CmdExecute(ICmdParam& cmd)
{
	return DoCmdExecute(cmd);
}

bool EvtListener::StdExecute(IStdParam& cmd)
{
	return DoStdExecute(cmd);
}

bool EvtListener::WndExecute(IWndParam& cmd)
{
	return DoWndExecute(cmd);
}

void EvtListener::AttachEvent(EvtBase* evt)
{
	if(!evt) return;
	evt->AttachListener(this);
}

void EvtListener::DetachEvent(EvtBase* evt)
{
	if(!evt) return;
	evt->DetachListener(this);
}

void EvtListener::AttachEvent(const String& s)
{
	AttachEvent(EvtManager::current().get(s));
}

void EvtListener::DetachEvent(const String& s)
{
	DetachEvent(EvtManager::current().get(s));
}


void EvtListener::UnLink()
{
	DetachFromAllEvents();
}

void EvtListener::DetachFromAllEvents()
{
	for(size_t i=0;i<m_aAttachedEvents.size();i++)
	{
		m_aAttachedEvents[i]->m_aAttachedListeners.remove(this);
	}
	m_aAttachedEvents.clear();
}

EvtListener::~EvtListener()
{
	DetachFromAllEvents();
}

EW_LEAVE
