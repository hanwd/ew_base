#ifndef __H_EW_UI_WINMANAGER__
#define __H_EW_UI_WINMANAGER__

#include "ewc_base/config.h"
#include "ewc_base/app/data_defs.h"
#include "ewc_base/app/cmdproc.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE App;
class DLLIMPEXP_EWC_BASE EvtManagerTop;
class DLLIMPEXP_EWC_BASE WndModelTop;
class DLLIMPEXP_EWC_BASE WndManagerImpl;

class DLLIMPEXP_EWC_BASE WndModelHolder
{
public:

	friend class WndManager;

	void SetData(WndModelTop* p){m_refData.reset(p);}
	WndModelTop* GetData(){return m_refData.get();}

	wxWindow* GetWindow();
	void Show(bool flag);
	bool Create();

	void OnChildWindow(IWindowPtr w,int a);
	bool mu_set(const String& a);

private:
	LitePtrT<WndModelTop> m_refData;
	String m_sName;
	String m_sLabel;
};

class DLLIMPEXP_EWC_BASE WndManager : public NonCopyable
{
protected:
	WndManagerImpl* m_pImpl;

public:

	friend class WndModel;
	
	App& app;

	EvtManagerTop& evtmgr;

	WndUpdator& wup;

	PluginManager& plugin;

	MvcBook& book;

	WndModelHolder model;

	LogPtr& logptr;

	CmdProcHolderT<Object> cmdptr;

	Language& lang;

	IDat_wndconfig conf;


	WndManager();
	~WndManager();

	static WndManager& current();

	template<typename T>
	void RegisterT(){RegisterPluginT<T>(*this);}

	void StartFrame();

	bool LoadPlugins();
	bool LoadScript(const String& s);

	bool InitConfig(const String& s);

	bool LoadConfig();
	bool SaveConfig(bool save_file=true);

	void SetName(const String& s);
	String GetTitle();

	void UpdateTitle();

	void Activate();

};

EW_LEAVE
#endif
