#include "ewc_base/plugin/plugin_plugineditor.h"
#include "ewc_base/mvc/mvc_view.h"
#include "ewc_base/mvc/mvc_model.h"
#include "ewc_base/mvc/mvc_book.h"

#include "ewc_base/evt/evt_manager.h"



EW_ENTER


int PluginEditor::MatchIndex(const String&)
{
	return 0;
}


bool PluginEditor::Open(const String& fn) 
{
	DataPtrT<MvcModel> pModel(CreateModel(fn));
	if(!pModel) return false;
	return OpenModel(pModel.get());
}

bool PluginEditor::OpenModel(DataPtrT<MvcModel> pModel)
{
	return wm.book.OpenModel(pModel.get());
}

PluginEditor* PluginEditor::cast_editor()
{
	return this;
}

PluginEditor::PluginEditor(WndManager& w,const String& s):basetype(w,s)
{
	m_aDependency.push_back("Plugin.MainWindow");
}


int PluginEditorEx::MatchIndex(const String& fn)
{
	if(fn.length()<4) return 0;
	String sExtension=fn.substr(fn.length()-4);
	if(m_aExtension.find(sExtension)==m_aExtension.end())
	{
		return 0;
	}
	return 10;
}

DataPtrT<MvcModel> PluginEditorEx::CreateModel(const String& fn)
{
	if(!m_pSampleModel)	m_pSampleModel=CreateSampleModel();
	if(m_pSampleModel) m_pSampleModel->fn.SetFilename(fn);

	return m_pSampleModel;
}

DataPtrT<MvcModel> PluginEditorEx::CreateSampleModel()
{
	return NULL;
}




EW_LEAVE
