#ifndef __H_EW_UI_VALIDATOR__
#define __H_EW_UI_VALIDATOR__

#include "ewc_base/config.h"
#include "ewc_base/evt/evt_listener.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE Validator : public EvtListener
{
public:

	Validator();
	~Validator();	

	bool WndExecuteEx(int action,int p1=-1,int p2=-1);

	virtual bool DoSetLabel(const String&);
	virtual bool DoGetLabel(String&);

	virtual bool DoSetValue(const String&);
	virtual bool DoGetValue(String&);
	virtual bool DoSetValue(int32_t);
	virtual bool DoGetValue(int32_t&);
	virtual bool DoSetValue(double);
	virtual bool DoGetValue(double&);

	virtual bool DoSetValue(const Variant& v);
	virtual bool DoGetValue(Variant&);

	bool OnWndEvent(IWndParam&,int);

	bool WndExecute(IWndParam&);

	virtual bool OnUpdateWindow(wxWindow* p,EvtBase* x);

	bool OnWndUpdate(IWndParam&,EvtBase*);
};


EW_LEAVE
#endif
