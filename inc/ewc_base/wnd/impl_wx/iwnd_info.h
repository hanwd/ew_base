#ifndef __H_UI_IWND_BUILDER__
#define __H_UI_IWND_BUILDER__

#include "ewc_base/wnd/impl_wx/impl_wx.h"
#include "ewc_base/wnd/wnd_model.h"
#include "ewc_base/wnd/wnd_maker.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/evt/evt_array.h"
#include "ewc_base/evt/evt_command.h"
#include "ewc_base/evt/evt_element.h"
#include "ewc_base/evt/validator_top.h"

#define IMPLEMENT_WX_CONTROL(X) \
template<>\
class DLLIMPEXP_EWC_BASE WndInfoT<X> : public WndInfoBaseT<X> \
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

inline String wx2str(const wxString& v)
{
	return v.c_str().AsChar();
}

inline wxString str2wx(const String& v)
{
	return v.c_str();
}


class WndInfo;
class DLLIMPEXP_EWC_BASE WndInfoManger
{
public:

	static WndInfoManger& current()
	{
		static WndInfoManger gInstance;
		return gInstance;
	}

	void Register(WndInfo* wi);

	wxWindow* Create(const String& s,wxWindow* w,const WndProperty& h);
	void Update(WndBuilder& builder,wxWindow* pwin);

	WndInfo* GetWndInfo(wxWindow* w);

private:

	WndInfoManger();
	bst_map<String,LitePtrT<WndInfo> > aValue;
	bst_map<void*,LitePtrT<WndInfo> > aVptrTable;
};

template<typename T>
void WndInfoManger_Register(WndInfoManger& imgr,const String& name);

class DLLIMPEXP_EWC_BASE WndInfo : public Object
{
public:

	WndInfo(const String& s):m_sWindowType(s)
	{

	}

	const String& GetType()
	{
		return m_sWindowType;
	}


	virtual wxWindow* Create(wxWindow* w,const WndPropertyEx& h){return NULL;}

	virtual Validator* CreateValidator(wxWindow* w,IValueProxyT<int32_t>* p){return NULL;}
	virtual Validator* CreateValidator(wxWindow* w,IValueProxyT<String>* p){return NULL;}
	virtual Validator* CreateValidator(wxWindow* w,IValueProxyT<double>* p){return NULL;}
	virtual Validator* CreateValidator(wxWindow* w,IValueProxyArray* p){return NULL;}

	virtual Validator* CreateValidator(wxWindow* w){return NULL;}

	Validator* CreateValidatorEx(wxWindow* w,IValueProxy* v)
	{
		if(!v) return CreateValidator(w);
		return v->CreateValidator(w);
	}

	virtual void Update(WndBuilder& builder,wxWindow* pwin);

protected:
	void UpdateGroup(WndBuilder& builder,wxWindow* pwin);
	String m_sWindowType;

};



template<typename T>
class DLLIMPEXP_EWC_BASE WndInfoBaseT : public WndInfo
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
	Validator* CreateValidatorT(wxWindow* w,IValueProxyT<X>* p)
	{
		return new ValidatorWT<T,X>((T*)w,p);
	}

	template<typename X>
	Validator* CreateValidatorBaseT(wxWindow* w,IValueProxyT<X>* p)
	{
		return new ValidatorBaseWT<T,X>((T*)w,p);
	}
};

template<typename T>
class WndInfoT;



template<typename W> 
class DLLIMPEXP_EWC_BASE ValidatorW : public Validator
{
public:
	LitePtrT<W> pWindow;
	ValidatorW(W* w):pWindow(w){}
};

template<typename W,typename T> 
class DLLIMPEXP_EWC_BASE ValidatorBaseWT : public ValidatorW<W>
{
public:

	DataPtrT<IValueProxyT<T> > proxy;

	ValidatorBaseWT(W* w,IValueProxyT<T>* t)
		:ValidatorW<W>(w)
		,proxy(t)
	{
		proxy->attach(this);
	}

	~ValidatorBaseWT()
	{
		proxy->detach(this);
	}



	virtual void OnValueChanged()
	{
		if(!DoGetValue(proxy->tmp_value))
		{
			return;
		}
		proxy->ValueChanged();
	}


	bool Transfer2Window()
	{
		if(!proxy->GetValue())
		{
			return false;
		}		
		return DoSetValue(proxy->tmp_value);
	}

	bool Transfer2Model()
	{
		if(!DoGetValue(proxy->tmp_value))
		{
			return false;
		}
		if(!proxy->SetValue())
		{
			return false;
		}
		return true;
	}

};

template<typename W,typename T>
class ValidatorWT;

EW_LEAVE
#endif
