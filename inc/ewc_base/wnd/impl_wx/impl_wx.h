#ifndef __H_UI_IWND_IMPL_WX__
#define __H_UI_IWND_IMPL_WX__

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#pragma push_macro("new")
#undef new
#include "wx/wx.h"
#pragma pop_macro("new")

#include "ewc_base/config.h"
#include "ewc_base/wnd/wnd_info.h"
#include "ewc_base/wnd/wnd_model.h"
#include "ewc_base/wnd/wnd_maker.h"

#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/evt/evt_array.h"
#include "ewc_base/evt/evt_command.h"
#include "ewc_base/evt/evt_element.h"
#include "ewc_base/evt/validator_top.h"

#define IMPLEMENT_WX_CONTROL(X) \
template<>\
class WndInfoT<X> : public WndInfoBaseT<X> \
{\
public:\
	WndInfoT(const String& s):WndInfoBaseT<X>(s){}\
};\
template<>\
void WndInfoManger_Register<X>(WndInfoManger& imgr,const String& name)\
{\
	static WndInfoT<X> info(name);\
	imgr.Register(&info);\
}\


EW_ENTER

class WndPropertyEx;

String wx2str(const wxString& v);
wxString str2wx(const String& v);
bool wx2buf(const wxString& v,StringBuffer<char>& sb);

template<typename T>
class WndInfoBaseT : public WndInfo
{
public:
	WndInfoBaseT(const String& s):WndInfo(s)
	{

	}

	wxWindow* Create(wxWindow* w,const WndPropertyEx& h)
	{
		T* p=new T(w,h);
		return p;
	}

	template<typename X>
	Validator* CreateValidatorBaseT(wxWindow* w,EvtProxyT<X>* p)
	{
		return new ValidatorBaseWT<T,X>((T*)w,p);
	}

	Validator* CreateValidatorVoidT(wxWindow* w,EvtBase* p)
	{
		return new ValidatorBaseWT<T,void>((T*)w,p);
	}
};

template<typename T>
class WndInfoT;

template<typename W> 
class ValidatorW;

template<typename W,typename T> 
class ValidatorBaseWT : public ValidatorW<W>
{
public:
	typedef ValidatorW<W> basetype;

	DataPtrT<EvtProxyT<T> > proxy;

	ValidatorBaseWT(W* w,EvtProxyT<T>* t)
		:basetype(w)
		,proxy(t)
	{
		proxy->AttachListener(this);
	}

	~ValidatorBaseWT()
	{
		
	}

	virtual wxWindow* GetWindow()
	{
		return this->pWindow;
	}

	virtual bool OnWndEvent(IWndParam& cmd,int phase)
	{

		if(cmd.action==IDefs::ACTION_UPDATECTRL)
		{
			this->OnUpdateWindow(this->pWindow,proxy.get());
			return true;
		}

		if(cmd.iwvptr && cmd.iwvptr!=this)
		{
			return true;
		}

		if(cmd.action==IDefs::ACTION_TRANSFER2WINDOW && phase==IDefs::PHASE_POSTCALL)
		{
			return DoSetValue(proxy->tmp_value);
		}
		else if(cmd.action==IDefs::ACTION_TRANSFER2MODEL && phase==IDefs::PHASE_PRECALL)
		{
			return DoGetValue(proxy->tmp_value);
		}
		else if(phase==IDefs::PHASE_POSTCALL)
		{
			return OnWndUpdate(cmd,proxy.get());
		}
		else
		{
			return basetype::OnWndEvent(cmd,phase);
		}
	}

	virtual bool WndExecute(IWndParam& cmd)
	{
		cmd.iwvptr=this;
		cmd.evtptr=proxy.get();

		if(cmd.action==IDefs::ACTION_VALUE_CHANGING)
		{
			return proxy->WndExecute(cmd);			
		}
		else if(cmd.action==IDefs::ACTION_VALUE_CHANGED)
		{
			if(!DoGetValue(proxy->tmp_value)) return false;
			return proxy->WndExecute(cmd);				
		}
		else if(cmd.action==IDefs::ACTION_TRANSFER2WINDOW)
		{
			return proxy->WndExecute(cmd);
		}
		else if(cmd.action==IDefs::ACTION_TRANSFER2MODEL)
		{
			return proxy->WndExecute(cmd);
		}
		else
		{
			return basetype::WndExecute(cmd);
		}
	}	
};

template<typename W> 
class ValidatorBaseWT<W,void> : public ValidatorW<W>
{
public:
	typedef ValidatorW<W> basetype;

	DataPtrT<EvtBase> proxy;

	ValidatorBaseWT(W* w,EvtBase* t)
		:ValidatorW<W>(w)
		,proxy(t)
	{
		proxy->AttachListener(this);
	}

	~ValidatorBaseWT()
	{
		
	}

	virtual wxWindow* GetWindow()
	{
		return this->pWindow;
	}

	virtual bool DoWndExecute(IWndParam& cmd)
	{
		return basetype::DoWndExecute(cmd);
	}

	virtual bool OnWndEvent(IWndParam& cmd,int phase)
	{
		if(cmd.action==IDefs::ACTION_UPDATECTRL)
		{
			this->OnUpdateWindow(this->pWindow,proxy.get());
			return true;
		}
		if(phase==IDefs::PHASE_POSTCALL) return true;
		return OnWndUpdate(cmd,proxy.get());
	}

	virtual bool WndExecute(IWndParam& cmd)
	{
		cmd.evtptr=proxy.get();
		return proxy->WndExecute(cmd);
	}	
};

EW_LEAVE
#endif


