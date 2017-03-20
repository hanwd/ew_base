#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN__

#include "ewc_base/app/cmdproc.h"
#include "ewc_base/wnd/wnd_model.h"

#define IMPLEMENT_IPLUGIN(T) \
template<>\
void DLLIMPEXP_EWC_BASE RegisterPluginT<T>(WndManager& wm)\
{\
	Plugin::DoRegister(wm,new T(wm));\
}
	

EW_ENTER
	
class DLLIMPEXP_EWC_BASE PluginCommon;
class DLLIMPEXP_EWC_BASE PluginEditor;
class DLLIMPEXP_EWC_BASE IEditorPlugin;

class DLLIMPEXP_EWC_BASE PluginMainWindow;
class DLLIMPEXP_EWC_BASE PluginBasic;
class DLLIMPEXP_EWC_BASE PluginSearch;
class DLLIMPEXP_EWC_BASE PluginWorkspace;
class DLLIMPEXP_EWC_BASE PluginHistoryFiles;
class DLLIMPEXP_EWC_BASE PluginMessage;
class DLLIMPEXP_EWC_BASE PluginOption;
class DLLIMPEXP_EWC_BASE PluginStatusBar;
class DLLIMPEXP_EWC_BASE PluginExpression;
class DLLIMPEXP_EWC_BASE PluginTextEditor;
class DLLIMPEXP_EWC_BASE PluginFlashViewer;
class DLLIMPEXP_EWC_BASE PluginWebView;
class DLLIMPEXP_EWC_BASE PluginEwslEditor;

template<typename T>
void DLLIMPEXP_EWC_BASE RegisterPluginT(WndManager& wm);

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

class DLLIMPEXP_EWC_BASE PluginEx : public Plugin
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

