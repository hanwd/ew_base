#ifndef __H_EW_UI_EVTCENTER__
#define __H_EW_UI_EVTCENTER__

#include "ewc_base/config.h"
#include "ewc_base/evt/evt_base.h"
#include "ewc_base/evt/evt_command.h"
#include "ewc_base/evt/evt_group.h"
#include "ewc_base/app/cmdproc.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE EvtManager : public Object
{
public:

	friend class WndModel;
	friend class WndBuilder;

	EvtBase* get(int evtid);
	EvtBase* get(const String& s);
	EvtGroup* get_group(const String& s);
	EvtCommand* get_command(const String& s);

	bool OnCmdExecute(int evtid,int p1,WndModel* pwm);

	static EvtManager& current();

	int id_new();

	void gp_beg(const String& s,const String& d);
	void gp_beg(const String& s);
	void gp_new(const String& s);
	void gp_beg(EvtGroup* g);

	int append(EvtBase* v);
	int gp_add(EvtBase* v);

	void gp_add(const String& s,const String& e,int f=0);
	void gp_add(const String& s,int f=0);
	void gp_end();

	void get_item(arr_1t<EvtItem>& v);
	void gp_set(const arr_1t<EvtItem>& v);

	void item_get(const String& s,arr_1t<EvtItem>& v);
	void item_set(const String& s,const arr_1t<EvtItem>& v);

	void link(const String& s,EvtBase* p);
	void link(const String& s,String& v,IValueOptionData* d);

	void link(const String& s,double& v);
	void link(const String& s,float& v);
	void link(const String& s,String& v);
	void link(const String& s,int32_t& v);
	void link(const String& s,int64_t& v);
	void link(const String& s,bool& v);
	void link(const String& s,BitFlags& v,int n);


	template<typename T>
	EvtProxyVariantT<T>& link_v(const String& s,VariantTable& m,const String& k="")
	{
		DataPtrT<EvtProxyVariantT<T> > pevt=new EvtProxyVariantT<T>(m,k.empty()?s:k);
		link(s,pevt.get());
		return *pevt;
	}

	template<typename T>
	EvtProxyVariantT<T>& link_v(const String& s,VariantTable& m,int64_t p,const String& k="")
	{
		DataPtrT<EvtProxyVariantT<T> > pevt=new EvtProxyVariantT<T>(m,k.empty()?s:k,p);
		link(s,pevt.get());
		return *pevt;
	}

	template<typename T>
	EvtProxyVariantT<T>& link_c(const String& s,const String& k="")
	{
		return link_v<T>(s,App::current().conf.values,k);
	}

	template<typename T>
	EvtProxyVariantT<T>& link_c(const String& s,int64_t p,const String& k="")
	{
		return link_v<T>(s,App::current().conf.values,p,k);
	}

	EvtBase& operator[](const String& s);


	void update(const String& id);
	virtual void pending(const String& id,int v=1);

	EvtManager(int id=-1);
	~EvtManager();

	virtual wxWindow* GetWindow();

	bool LoadConfig();
	bool SaveConfig();

	
protected:

	typedef arr_1t<DataPtrT<EvtGroup> > arr_events;
	typedef indexer_map<String,DataPtrT<EvtBase> > map_events;

	arr_events m_aGroups;
	map_events m_aEvents;
	arr_1t<String> m_aPendingUpdate;
	AtomicMutex m_tMutex;

	int m_nIdMax;
	int m_nIdMin;
};

class DLLIMPEXP_EWC_BASE EvtManagerTop : public EvtManager
{
public:
	EvtManagerTop(WndManager& wm_);

	void langup();
	wxWindow* GetWindow();

	VariantTable table;

	template<typename T>
	EvtProxyVariantT<T>& link_a(const String& s,const String& k="")
	{
		return link_v<T>(s,table,k);
	}

	template<typename T>
	EvtProxyVariantT<T>& link_a(const String& s,int64_t p,const String& k="")
	{
		return link_v<T>(s,table,p,k);
	}

protected:
	WndManager& wm;
};



EW_LEAVE
#endif
