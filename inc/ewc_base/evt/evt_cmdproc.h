#ifndef __H_EW_UI_EVT_EVT_CMDPROC__
#define __H_EW_UI_EVT_EVT_CMDPROC__

#include "ewc_base/config.h"
#include "ewc_base/app/app.h"
#include "ewc_base/evt/evt_command.h"

EW_ENTER



class DLLIMPEXP_EWC_BASE EvtCommandCmdProc : public EvtCommand
{
public:

	WndManager& wm;
	const int id;

	EvtCommandCmdProc(WndManager& w,const String& s,int d);

	void DoUpdateCtrl(IUpdParam& upd);

	bool DoCmdExecute(ICmdParam&);

	String sExtra;
};


EW_LEAVE
#endif
