#include "ewc_base/evt/evt_option.h"

EW_ENTER

EvtOptionPage::EvtOptionPage(const String& s):basetype(s),wm(WndManager::current())
{
	
}


wxWindow* EvtOptionPage::CreatePage(wxWindow* w)
{
	WndMaker km(w);

	km.win("container");
		DoCreatePage(km);
	km.end();

	wxWindow* pw=km.get();		
	m_pVald=km.vald;

	if(m_pValdGroup)
	{
		m_pValdGroup->append(m_pVald.get());
	}
	return pw;
}

void EvtOptionPage::DoCreatePage(WndMaker&)
{

}


EW_LEAVE
