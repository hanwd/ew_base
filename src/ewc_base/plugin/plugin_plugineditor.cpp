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
	String sExtension="."+string_to_lower(string_split(fn,".").back());
	if(m_aExtension.find(sExtension)==m_aExtension.end())
	{
		return 0;
	}
	return 10;
}

DataPtrT<MvcModel> PluginEditorEx::CreateModel(const String& fn)
{
	DataPtrT<MvcModel> p = CreateSampleModel();
	p->fn.SetFilename(fn);
	return p;
}

DataPtrT<MvcModel> PluginEditorEx::CreateSampleModel()
{
	return new MvcModel();;
}




EW_LEAVE
