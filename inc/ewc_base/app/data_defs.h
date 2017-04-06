#ifndef __H_EW_UI_APP_DATA_DEFS__
#define __H_EW_UI_APP_DATA_DEFS__

#include "ewc_base/config.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE IDat_search
{
public:

	IDat_search();

	String text_new;;
	String text_old;	
	BitFlags flags;

	static IDat_search& current();

};

class DLLIMPEXP_EWC_BASE AppData
{
public:
	AppData();

	int toolbitmap_size;
	int menubitmap_size;
	String toolbar_type;

	const int evt_user_dropdown_menu;
	Functor<void(wxWindow*,wxMenu*)> popup_dropdown_menu;

	static AppData& current();
	
};


class DLLIMPEXP_EWC_BASE IWndParam
{
public:

	IWndParam(int act=-1,int p1=-1,int p2=-1):action(act),param1(p1),param2(p2){}
	IWndParam(Validator* pwv,int act,int p1=-1,int p2=-1):action(act),param1(p1),param2(p2),iwvptr(pwv){}
	IWndParam(EvtBase* evt,int act,int p1=-1,int p2=-1):action(act),param1(p1),param2(p2),evtptr(evt){}

	int action;
	BitFlags flags;

	intptr_t param1;
	intptr_t param2;

	LitePtrT<EvtBase> evtptr;
	LitePtrT<Validator> iwvptr;
	LitePtrT<wxWindow> wndptr;

};

class DLLIMPEXP_EWC_BASE IStdParam
{
public:

	IStdParam(int p1=-1,int p2=-1):param1(p1),param2(p2){}
	IStdParam(const String& s1,int p1=-1,int p2=-1):param1(p1),param2(p2),extra1(s1){}
	IStdParam(EvtBase* evt,int p1=-1,int p2=-1,wxWindow* pw=NULL):param1(p1),param2(p2),evtptr(evt),wndptr(pw){}
	IStdParam(EvtBase* evt,const String& s1,int p1=-1,int p2=-1,wxWindow* pw=NULL):param1(p1),param2(p2),extra1(s1),evtptr(evt),wndptr(pw){}

	BitFlags flags;

	intptr_t param1;
	intptr_t param2;

	String extra1;
	String extra2;

	LitePtrT<EvtBase> evtptr;
	LitePtrT<wxWindow> wndptr;

};

class DLLIMPEXP_EWC_BASE ICmdParam
{
public:

	ICmdParam(int p1=-1,int p2=-1):param1(p1),param2(p2){}
	ICmdParam(EvtBase* evt,int p1=-1,int p2=-1,WndModel* pwm=NULL):param1(p1),param2(p2),evtptr(evt),iwmptr(pwm){}
	ICmdParam(EvtBase* evt,const String& s1,int p1=-1,int p2=-1,WndModel* pwm=NULL):param1(p1),param2(p2),extra1(s1),evtptr(evt),iwmptr(pwm){}


	BitFlags flags;
	intptr_t param1;
	intptr_t param2;
	String extra1;
	String extra2;

	LitePtrT<EvtBase> evtptr;
	LitePtrT<WndModel> iwmptr;

};


class DLLIMPEXP_EWC_BASE IUpdParam
{
public:

	IUpdParam()
	{

	}

	enum
	{
		FLAG_NO_RECURSIVE=1<<0,
	};

	void clear()
	{
		m_aUpdated.clear();
	}

	inline bool test(void* p)
	{
		size_t id=m_aUpdated.size();		
		return m_aUpdated.insert(p)==id;
	}

protected:
	indexer_set<void*> m_aUpdated;
};


class DLLIMPEXP_EWC_BASE EvtItem
{
public:
	EvtItem(const String& s="",int f=0):sname(s),flags(f){}
	EvtItem(const String& s,const String& e,int f=0):sname(s),extra(e),flags(f){}
	String sname;
	String extra;
	BitFlags flags;

	operator const String&(){return sname;}
};


class DLLIMPEXP_EWC_BASE IFileNameData : public ObjectData
{
public:

	virtual void SetFilename(const String& s);
	virtual String GetFilename();

	virtual void SetExts(const String& exts);
	virtual String GetExts();

	virtual String GetTempfile();

	virtual bool IsSavable();	

	IFileNameData* DoClone(ObjectCloneState&);

protected:
	String m_sFilename;
	String m_sTempfile;
	String m_sExts;
	BitFlags flags;
};

class DLLIMPEXP_EWC_BASE IFileNameHolder : public ObjectT<IFileNameData>
{
public:
	IFileNameHolder(){SetData(new IFileNameData);}

	String GetFilename(){return m_refData->GetFilename();}
	void SetFilename(const String& s){m_refData->SetFilename(s);}
	String GetTempfile(){return m_refData->GetTempfile();}
	void SetExts(const String& exts){ m_refData->SetExts(exts); }
	String GetExts(){ return m_refData->GetExts(); }

	bool IsSavable(){ return m_refData->IsSavable(); }

};

EW_LEAVE

#endif

