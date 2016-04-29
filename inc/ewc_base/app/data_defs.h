#ifndef __H_EW_UI_APP_DATA_DEFS__
#define __H_EW_UI_APP_DATA_DEFS__

#include "ewc_base/config.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE IDat_wndconfig
{
public:

	int bmp_menu_size;
	int bmp_tool_size;

	IDat_wndconfig();

};


class IDat_search
{
public:

	IDat_search();

	String text_new;;
	String text_old;	
	BitFlags flags;

	static IDat_search& current();

};



class IWndParam
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

};

class IStdParam
{
public:

	IStdParam(int p1=-1,int p2=-1):param1(p1),param2(p2){}
	IStdParam(const String& s1,int p1=-1,int p2=-1):param1(p1),param2(p2),extra(s1){}
	IStdParam(EvtBase* evt,int p1=-1,int p2=-1,wxWindow* pw=NULL):param1(p1),param2(p2),evtptr(evt),wndptr(pw){}
	IStdParam(EvtBase* evt,const String& s1,int p1=-1,int p2=-1,wxWindow* pw=NULL):param1(p1),param2(p2),extra(s1),evtptr(evt),wndptr(pw){}

	BitFlags flags;

	intptr_t param1;
	intptr_t param2;

	String extra;

	LitePtrT<EvtBase> evtptr;
	LitePtrT<wxWindow> wndptr;

};

class ICmdParam
{
public:

	ICmdParam(int p1=-1,int p2=-1):param1(p1),param2(p2){}
	ICmdParam(EvtBase* evt,int p1=-1,int p2=-1,WndModel* pwm=NULL):param1(p1),param2(p2),evtptr(evt),iwmptr(pwm){}
	ICmdParam(EvtBase* evt,const String& s1,int p1=-1,int p2=-1,WndModel* pwm=NULL):param1(p1),param2(p2),extra(s1),evtptr(evt),iwmptr(pwm){}


	BitFlags flags;
	intptr_t param1;
	intptr_t param2;
	String extra;

	LitePtrT<EvtBase> evtptr;
	LitePtrT<WndModel> iwmptr;

};


class IUpdParam
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


class EvtItem
{
public:
	EvtItem(const String& s="",int f=0):sname(s),flags(f){}
	EvtItem(const String& s,const String& e,int f=0):sname(s),extra(e),flags(f){}
	String sname;
	String extra;
	BitFlags flags;

	operator const String&(){return sname;}
};


class IFileNameData : public ObjectData
{
public:

	virtual const String& GetFilename();
	virtual void SetFilename(const String& s);
	virtual const String& GetTempfile();

	IFileNameData* DoClone(ObjectCloneState&)
	{
		return new IFileNameData(*this);
	}

protected:
	String m_sFilename;
	String m_sTempfile;
};

class IFileNameHolder : public ObjectT<IFileNameData>
{
public:
	IFileNameHolder(){SetData(new IFileNameData);}

	const String& GetFilename(){return m_refData->GetFilename();}
	void SetFilename(const String& s){m_refData->SetFilename(s);}
	const String& GetTempfile(){return m_refData->GetTempfile();}
};

EW_LEAVE

#endif

