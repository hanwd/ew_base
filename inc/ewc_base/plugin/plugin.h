#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN__

#include "ewc_base/app/cmdproc.h"
#include "ewc_base/wnd/wnd_model.h"

#define IMPLEMENT_IPLUGIN(T) \
template<>\
void RegisterPluginT<T>(WndManager& wm)\
{\
	Plugin::DoRegister(wm,new T(wm));\
}
	

EW_ENTER
	
class PluginCommon;
class PluginEditor;
class IEditorPlugin;

class PluginMainWindow;
class PluginBasic;
class PluginSearch;
class PluginWorkspace;
class PluginHistoryFiles;
class PluginMessage;
class PluginOption;
class PluginStatusBar;

class PluginTextEditor;
class PluginFlashViewer;
class PluginWebView;
class PluginEwslEditor;

template<typename T>
void RegisterPluginT(WndManager& wm);

class DLLIMPEXP_EWC_BASE Plugin : public EvtBase
{
public:

	typedef EvtBase basetype;
	enum
	{
		FLAG_MIN		=basetype::FLAG_MAX,
		FLAG_ATTACHED	=FLAG_MIN<<0,
		FLAG_PENDING	=FLAG_MIN<<1, // attaching
	};

	WndManager& wm;

	Plugin(WndManager& w,const String& s):EvtBase(s),wm(w){}

	virtual bool OnAttach()=0;
	virtual void OnDetach()=0;


	virtual void OnEditorActivate(PluginEditor*,PluginEditor*)=0;

	virtual Plugin* cast_plugin(){return this;}
	virtual PluginCommon* cast_common(){return NULL;}
	virtual PluginEditor* cast_editor(){return NULL;}
	virtual IEditorPlugin* cast_editor_plugin(){return NULL;}

	virtual void GetDependency(arr_1t<String>&)=0;

	virtual bool GetState(int) const =0;
	virtual void SetState(int,bool)=0;

	static EvtListener* GetViewListener();

	static void DoRegister(WndManager& wm,Plugin* p);

};

class PluginEx : public Plugin
{
public:

	PluginEx(WndManager& w,const String& s):Plugin(w,s){}

	virtual bool OnAttach();
	virtual void OnDetach();

	virtual void OnEditorActivate(PluginEditor*,PluginEditor*);


	virtual bool GetState(int f) const;
	virtual void SetState(int f,bool b);

	virtual void GetDependency(arr_1t<String>& arr);


protected:
	arr_1t<String> m_aDependency;

};


EW_LEAVE

#endif

