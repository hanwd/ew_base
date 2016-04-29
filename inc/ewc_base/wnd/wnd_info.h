#ifndef __H_EW_UI_WININFO__
#define __H_EW_UI_WININFO__

#include "ewc_base/config.h"
#include "ewc_base/wnd/wnd_property.h"
#include "ewc_base/evt/validator.h"
#include "ewc_base/evt/evt_element.h"
#include "ewc_base/evt/evt_array.h"
EW_ENTER

class WndInfoManger;
class WndPropertyEx;

template<typename T>
void WndInfoManger_Register(WndInfoManger& imgr,const String& name);

class DLLIMPEXP_EWC_BASE WndInfo : public Object
{
public:

	WndInfo(const String& s):m_sWindowType(s){}

	const String& GetType(){return m_sWindowType;}

	virtual wxWindow* Create(wxWindow* w,const WndPropertyEx& h){return NULL;}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<int32_t>*){return NULL;}
	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<Variant>*){return NULL;}
	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<String>*){return NULL;}
	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<double>*){return NULL;}
	virtual Validator* CreateValidator(wxWindow* w,EvtProxyArray*){return NULL;}

	virtual Validator* CreateValidator(wxWindow* w,EvtBase*){return NULL;}
	virtual Validator* CreateValidator(wxWindow* w){return NULL;}

	Validator* CreateValidatorEx(wxWindow* w,EvtBase* v)
	{
		return v?v->CreateValidator(w):CreateValidator(w);
	}

	virtual void Update(WndMaker& wk,wxWindow* pwin,EvtBase* v);

protected:
	String m_sWindowType;

};

class DLLIMPEXP_EWC_BASE WndInfoManger
{
public:

	static WndInfoManger& current();

	void Register(WndInfo* wi);

	wxWindow* Create(const String& s,wxWindow* w,const WndProperty& h);
	WndInfo* GetWndInfo(wxWindow* w);

private:

	WndInfoManger();
	bst_map<String,LitePtrT<WndInfo> > aValue;
	bst_map<void*,LitePtrT<WndInfo> > aVptrTable;
};


EW_LEAVE

#endif
