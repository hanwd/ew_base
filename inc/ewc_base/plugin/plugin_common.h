#ifndef __H_EW_EWC_BASE_PLUGIN_PLUGIN_COMMON__
#define __H_EW_EWC_BASE_PLUGIN_PLUGIN_COMMON__


#include "ewc_base/plugin/plugin.h"


EW_ENTER


class DLLIMPEXP_EWC_BASE PluginCommon : public PluginEx
{
public:
	typedef PluginEx basetype;
	PluginCommon(WndManager& w,const String& s):basetype(w,s){}

	virtual PluginCommon* cast_common();

};

class PluginCommon2 : public PluginCommon
{
public:

	typedef PluginCommon basetype;
	PluginCommon2(WndManager& w,const String& s):basetype(w,s){}

	void GetDependency(arr_1t<String>& arr);

};


EW_LEAVE
#endif
