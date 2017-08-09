#ifndef __H_EW_UI_EVT_EVT_OPTION__
#define __H_EW_UI_EVT_EVT_OPTION__

#include "ewc_base/evt/evt_command.h"


EW_ENTER

class IWnd_bookbase;

class DLLIMPEXP_EWC_BASE EvtDynamicPage : public EvtCommand
{
public:
	typedef EvtCommand basetype;

	WndManager& wm;

	EvtDynamicPage(const String& s,const String& n="");

	String m_sName;

	DataPtrT<ValidatorGroup> m_pValdGroup;
	DataPtrT<Validator> m_pVald;

	virtual wxWindow* CreatePage(wxWindow* w,EvtManager* p);

protected:
	virtual void DoCreatePage(WndMaker&);

};

class EvtDynamicPageScript : public EvtDynamicPage
{
public:
	typedef EvtDynamicPage basetype;
	EvtDynamicPageScript(const String& s,const String& f,const String& n="");

protected:
	virtual void DoCreatePage(WndMaker&);
	String m_sScriptFile;
};


class IPageItem
{
public:
	IPageItem(EvtDynamicPage* p = NULL);

	DataPtrT<EvtDynamicPage> dpage;
	LitePtrT<wxWindow> ppage;
	DataPtrT<Validator> pvald;
	String sname;
	bool binit;
};


class IPageSelector : public NonCopyable
{
public:

	void init_combo(wxWindow* p, const String& s);
	void init_tree(wxWindow* p, const String& s);

	arr_1t<IPageItem> items;
	LitePtrT<wxWindow> ptype;
	LitePtrT<IWnd_bookbase> pbook;
	DataPtrT<Validator> pvald;
	DataPtrT<Validator> p_sel;
	LitePtrT<EvtManager> pevtmgr;


};



EW_LEAVE
#endif
