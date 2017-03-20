#ifndef __H_EW_UI_WINMAKER__
#define __H_EW_UI_WINMAKER__

#include "ewc_base/config.h"
#include "ewc_base/wnd/wnd_property.h"
#include "ewc_base/evt/validator_group.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE WndPropertyEx;

class DLLIMPEXP_EWC_BASE WndProxyState
{
public:
	WndProxyState(IWindowPtr w=NULL):hwnd(w),p_sz(NULL){}
	WndProxyState(const WndProperty& p,IWindowPtr w=NULL):prop(p),hwnd(w),p_sz(NULL){}
	WndProxyState(const WndProperty& p,ISizerPtr w):prop(p),hwnd(NULL),p_sz(w){}

	WndProperty prop;
	IWindowPtr hwnd;
	ISizerPtr p_sz;
	DataPtrT<ValidatorGroup> p_vg;

};


class DLLIMPEXP_EWC_BASE WndMaker : public IDefs
{
public:

	friend class WndModel;

	WndMaker(IWindowPtr t=NULL);
	WndMaker(WndModel* t);

	WndMaker& ld(int n){_tempp=_aprops[n];return *this;}
	WndMaker& sv(int n){_aprops[n]=_tempp;_tempp=_aprops[0];return *this;}

	WndMaker& operator()(int n){return ld(n);}

#define IWINPROPERTY_PROPERTY_I1(X) WndMaker& X(int32_t v){_tempp.X(v);return *this;}
#define IWINPROPERTY_PROPERTY_I2(X) WndMaker& X(int32_t x,int32_t y){_tempp.X(x,y);return *this;}
#define IWINPROPERTY_PROPERTY_S1(X) WndMaker& X(const String& v){_tempp.X(v);return *this;}
#define IWINPROPERTY_PROPERTY_S2(X) WndMaker& X(const String& x,const String& y){_tempp.X(x,y);return *this;}

	IWINPROPERTY_PROPERTY_I1(propotion);
	IWINPROPERTY_PROPERTY_I1(border);
	IWINPROPERTY_PROPERTY_I1(align);
	IWINPROPERTY_PROPERTY_I1(valign);
	IWINPROPERTY_PROPERTY_I1(width);
	IWINPROPERTY_PROPERTY_I1(height);
	IWINPROPERTY_PROPERTY_I1(flags);
	IWINPROPERTY_PROPERTY_I1(nvalue);
	IWINPROPERTY_PROPERTY_S1(name);
	IWINPROPERTY_PROPERTY_S1(icon);
	IWINPROPERTY_PROPERTY_S1(label);
	IWINPROPERTY_PROPERTY_S1(value);
	IWINPROPERTY_PROPERTY_S1(page);
	IWINPROPERTY_PROPERTY_S1(pane);
	IWINPROPERTY_PROPERTY_S1(desc);
	IWINPROPERTY_PROPERTY_S1(hint);
	IWINPROPERTY_PROPERTY_S1(help);
	IWINPROPERTY_PROPERTY_S1(tooltip);
	IWINPROPERTY_PROPERTY_S1(add_extra);

	IWINPROPERTY_PROPERTY_I2(position);
	IWINPROPERTY_PROPERTY_I2(size);
	IWINPROPERTY_PROPERTY_S2(sprops);

	operator WndProperty&(){return _tempp;}
	operator WndPropertyEx&(){return *(WndPropertyEx*)&_tempp;}


	WndMaker& win(IWindowPtr t,const WndProperty& p);
	WndMaker& add(IWindowPtr t,const WndProperty& p);

	WndMaker& win(const String& t,const WndProperty& p);
	WndMaker& add(const String& t,const WndProperty& p);

	WndMaker& row(const WndProperty& p);
	WndMaker& col(const WndProperty& p);

	WndMaker& win(IWindowPtr t){return win(t,_tempp);}
	WndMaker& add(IWindowPtr t){return add(t,_tempp);}

	WndMaker& win(const String& t){return win(t,_tempp);}
	WndMaker& add(const String& t){return add(t,_tempp);}

	WndMaker& row(){return row(_tempp);}
	WndMaker& col(){return col(_tempp);}

	WndMaker& end();

	IWindowPtr get(){return pwin;}

	EvtBase* find(const String& s);

	void vd_set(Validator* vd);

	WndProxyState icur;
	DataPtrT<ValidatorGroup> vald;
	DataPtrT<WndModel> pmodel;

	void set_model(WndModel* p);
	
protected:

	arr_1t<EvtBase*> aEvtArray;
	arr_1t<DataPtrT<ValidatorGroup> > aValdStack;

	WndMaker& szr(ISizerPtr psz,const WndProperty& p);

	void _check_hwnd();
	WndProxyState itmp;

	arr_1t<WndProxyState> awin;
	IWindowPtr pwin;

	WndProperty _tempp;
	bst_map<int,WndProperty> _aprops;

	LitePtrT<EvtManager> pevtmgr;


};


class DLLIMPEXP_EWC_BASE CallableWndModelPtr : public CallableData
{
public:
	CallableWndModelPtr(WndModel* p):pmodel(p){}
	DataPtrT<WndModel> pmodel;
};


class DLLIMPEXP_EWC_BASE CallableMaker : public CallableTableProxy
{
public:

	WndMaker& km;

	CallableMaker(WndModel* p=NULL);
	CallableMaker(WndMaker& km_);

	int __fun_call(Executor& ewsl,int);

	CallableData* DoClone(ObjectCloneState& cs) 
	{
		if(cs.type==0) return this;
		return new CallableMaker();
	}


protected:
	WndMaker _km;
};



EW_LEAVE
#endif

