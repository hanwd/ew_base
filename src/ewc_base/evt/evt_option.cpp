#include "ewc_base/evt/evt_option.h"
#include "ewc_base/wnd/wnd_model.h"
#include "ewc_base/wnd/wnd_maker.h"

EW_ENTER

EvtOptionPage::EvtOptionPage(const String& s):basetype(s),wm(WndManager::current())
{
	
}


wxWindow* EvtOptionPage::CreatePage(wxWindow* w)
{
	WndMaker km(w);

	DoCreatePage(km);

	wxWindow* pw=km.get();		
	m_pVald=km.vald;

	if(m_pValdGroup)
	{
		m_pValdGroup->append(m_pVald.get());
	}
	return pw;
}

void  EvtOptionPage::DoCreatePage(WndMaker&)
{

}

EvtOptionPageScript::EvtOptionPageScript(const String& s, const String& f) :basetype(s), m_sScriptFile(f)
{

}

void EvtOptionPageScript::DoCreatePage(WndMaker& km)
{
	Executor ewsl;
	ewsl.push(new CallableMaker(km));
	if (!ewsl.execute_file(m_sScriptFile,1))
	{
		return;
	}
}


EW_LEAVE
