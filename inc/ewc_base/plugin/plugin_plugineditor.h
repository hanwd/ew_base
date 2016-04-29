#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN_PLUGINEDITOR__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN_PLUGINEDITOR__


#include "ewc_base/plugin/plugin.h"
#include "ewc_base/mvc/mvc_model.h"

class wxAuiNotebook;

EW_ENTER

class MvcModel;

class DLLIMPEXP_EWC_BASE PluginEditor : public PluginEx
{
public:
	typedef PluginEx basetype;

	PluginEditor(WndManager& w,const String&);

	virtual int MatchIndex(const String&);
	virtual bool Open(const String&);


	virtual DataPtrT<MvcModel> CreateModel(const String& fn)=0;


	bool OpenModel(DataPtrT<MvcModel> pModel);

	virtual PluginEditor* cast_editor();

};


class DLLIMPEXP_EWC_BASE EvtCommandNewEditorFile : public EvtCommand
{
public:
	PluginEditor& Target;

	EvtCommandNewEditorFile(PluginEditor& t,const String& s)
		:EvtCommand(s)
		,Target(t)
	{

	}

	bool DoCmdExecute(ICmdParam&)
	{
		Target.OpenModel(Target.CreateModel(""));
		return true;
	}
};

class DLLIMPEXP_EWC_BASE PluginEditorEx : public PluginEditor
{
public:
	PluginEditorEx(WndManager& w,const String& s):PluginEditor(w,s){}

	virtual int MatchIndex(const String& fn);

	virtual DataPtrT<MvcModel> CreateModel(const String& fn);
	virtual DataPtrT<MvcModel> CreateSampleModel();

protected:
	DataPtrT<MvcModel> m_pSampleModel;
	bst_set<String> m_aExtension;
};


EW_LEAVE

#endif
