#ifndef __H_EW_UI_VALUEPROXY__
#define __H_EW_UI_VALUEPROXY__

#include "ewc_base/config.h"
#include "ewc_base/app/app.h"
#include "ewc_base/evt/evt_listener.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE IUpdParam;

class DLLIMPEXP_EWC_BASE IValueOptionData : public ObjectData
{
public:

	virtual size_t size() const {return 0;}
	virtual const String item(size_t) const {return "";}

};

class DLLIMPEXP_EWC_BASE IValueOptionDataEx : public IValueOptionData
{
public:

	class OptionItem
	{
	public:
		OptionItem(const String& v=""):value(v){}
		String value;
	};

	void add(const String& v)
	{
		aOption.push_back(v);
	}

	virtual size_t size() const {return aOption.size();}
	virtual const String item(size_t i) const {return aOption[i].value;}

	arr_1t<OptionItem> aOption;
};

class DLLIMPEXP_EWC_BASE EvtListenerGroup : public arr_1t<EvtListener*>
{
public:

	bool append(EvtListener* p)
	{
		if(!p) return false;
		if(std::find(begin(),end(),p)!=end())
		{
			return false;
		}
		arr_1t<EvtListener*>::append(p);
		return true;
	}

	bool remove(EvtListener* p)
	{
		iterator it=std::find(begin(),end(),p);
		if(it==end()) return false;
		erase(it);
		return true;
	}
};



class DLLIMPEXP_EWC_BASE EvtBase : public EvtListener
{
public:

	typedef EvtListener basetype;
	friend class EvtListener;

	enum
	{
		FLAG_MIN		=1,
		FLAG_DISABLE	=FLAG_MIN<<0,	// �ù��ܽ���
		FLAG_HIDE_UI	=FLAG_MIN<<1,	// �ӽ�������
		FLAG_SEPARATOR	=FLAG_MIN<<2,	// �ָ���
		FLAG_G_DYNAMIC	=FLAG_MIN<<3,	// ������ݶ�̬����
		FLAG_G_LOADED	=FLAG_MIN<<4,	// ��������Ѿ�����
		FLAG_CHECK		=FLAG_MIN<<5,
		FLAG_RADIO		=FLAG_MIN<<6,
		FLAG_CHECKED	=FLAG_MIN<<7,
		FLAG_DOTDOT		=FLAG_MIN<<8,
		FLAG_GROUP		=FLAG_MIN<<9,
		FLAG_VALUEBIND	=FLAG_MIN<<10,
		FLAG_RECURSIVE	=FLAG_MIN<<11,
		FLAG_READONLY	=FLAG_MIN<<12,
		FLAG_MAX		=FLAG_MIN<<13,
	};

	enum
	{
		LABEL_MENU,
		LABEL_TOOL,
		LABEL_MENUBAR,
	};
	
	EvtBase();
	EvtBase(const String& s);
	~EvtBase();

	virtual Validator* CreateValidator(wxWindow*);

	virtual bool CmdExecute(ICmdParam& cmd);
	virtual bool StdExecute(IStdParam& cmd);
	virtual bool WndExecute(IWndParam& cmd);


	bool WndExecuteEx(int action,int p1=-1,int p2=-1);

	bool StdExecuteEx(int p1=-1,int p2=-1,wxWindow* pw=NULL);
	bool StdExecuteEx(const String& s1,int p1=-1,int p2=-1,wxWindow* pw=NULL);

	bool CmdExecuteEx(int p1,int p2=-1,WndModel* pwm=NULL);
	bool CmdExecuteEx(const String& s1,int p1=-1,int p2=-1);

	// ���������Combo��Ŀؼ����������������б������
	virtual IValueOptionData* GetComboArray(){return NULL;}

	void UpdateCtrl(IUpdParam& upd);
	void UpdateCtrl();

	// ���´��¼���صĹ��������˵��ȡ�
	virtual void DoUpdateCtrl(IUpdParam& upd);

	// ���ɽ�����ص�Label����˵��͹�������
	virtual String MakeLabel(int hint=LABEL_TOOL) const{return m_sId;}

	// �¼������������ڼ��CmdExecute/StdExecute/WndExecute�ȵĵ��á�
	void AttachListener(EvtListener* pListener);
	void DetachListener(EvtListener* pListener);

	String m_sId;	// ��ʶ
	int m_nId;		// wxWindowId,������wxWindow��ص��¼�ӳ��
	BitFlags flags;

	virtual Plugin* cast_plugin(){return NULL;}
	virtual EvtCommand* cast_command(){return NULL;}
	virtual EvtGroup* cast_group(){return NULL;}

	// ���������Ĺ��ߣ��˵����򴰿ڡ�
	virtual IToolItemPtr CreateToolItem(ITbarPtr){return NULL;}
	virtual IMenuItemPtr CreateMenuItem(IMenuPtr){return NULL;}
	virtual IWindowPtr CreateWndsItem(IWindowPtr){return NULL;}

	virtual void UpdateMenuItem(IMenuItemPtr){}
	virtual void UpdateToolItem(IToolItemPtr){}

	virtual EvtGroup* CreateGroup(const String&){return NULL;}

	virtual wxWindow* GetWindow(){return NULL;}


	virtual IMenuPtr CreateMenu(HeMenuImpl* mu=NULL,bool prepare=true){return NULL;}
	virtual ITbarPtr CreateTbar(IWindowPtr pw,int wd=-1){return NULL;}

	virtual bool PopupMenu(IWindowPtr pw=NULL);

	void DetachAllListeners();

	void UnLink();

protected:

	EvtListenerGroup m_aAttachedListeners;
};


template<typename T,typename B=EvtBase>
class DLLIMPEXP_EWC_BASE EvtBaseT : public B
{
public:
	typedef B basetype;
	T& Target;

	EvtBaseT(const String& n,T& t):basetype(n),Target(t){}
};

EW_LEAVE
#endif
