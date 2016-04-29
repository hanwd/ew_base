#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN_MESSAGE__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN_MESSAGE__

#include "ewc_base/plugin/plugin_common.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE PluginMessage : public PluginCommon
{
public:
	typedef PluginCommon basetype;

	PluginMessage(WndManager& w);

	virtual bool OnAttach();

protected:
	LitePtrT<wxWindow> m_pWindow;
};

EW_LEAVE

#endif
