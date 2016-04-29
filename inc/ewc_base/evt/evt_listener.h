#ifndef __H_EW_UI_EVTLISTENER__
#define __H_EW_UI_EVTLISTENER__

#include "ewc_base/config.h"


EW_ENTER


class DLLIMPEXP_EWC_BASE EvtListener : public ObjectData
{
public:
	friend class EvtBase;

	~EvtListener();

	virtual bool OnCmdEvent(ICmdParam&,int){return true;}
	virtual bool OnWndEvent(IWndParam&,int){return true;}
	virtual bool OnStdEvent(IStdParam&,int){return true;}
	virtual bool OnCfgEvent(int){return true;}

	virtual bool CmdExecute(ICmdParam& cmd);
	virtual bool WndExecute(IWndParam& cmd);
	virtual bool StdExecute(IStdParam& cmd);

	void DetachFromAllEvents();

	void AttachEvent(EvtBase* evt);
	void DetachEvent(EvtBase* evt);

	void AttachEvent(const String& s);
	void DetachEvent(const String& s);

	virtual wxWindow* GetWindow(){return NULL;}

	// unlink all connected objects
	virtual void UnLink();

	virtual bool DoCmdExecute(ICmdParam&){return true;}
	virtual bool DoStdExecute(IStdParam&){return true;}
	virtual bool DoWndExecute(IWndParam&){return true;}

protected:
	ObjectGroupT<EvtBase> m_aAttachedEvents;
};

template<typename B>
class EvtListenerT : public ObjectT<EvtListener,B>
{
public:
	virtual bool OnCmdEvent(ICmdParam& cmd,int phase)
	{
		return m_refData?m_refData->OnCmdEvent(cmd,phase):true;
	}
};

EW_LEAVE


#endif
