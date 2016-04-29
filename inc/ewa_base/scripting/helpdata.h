#ifndef __H_EW_SCRIPTING_HELPDATA__
#define __H_EW_SCRIPTING_HELPDATA__

#include "ewa_base/scripting/callable_data.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE HelpData : public CallableData
{
public:

	HelpData();
	HelpData(const String& s, const String& v);
	
	indexer_map<String, String> helpmap;

	void parse(const String& name,const String& desc);

	virtual bool ToValue(String& s, int n = 0) const;

	virtual void Serialize(Serializer& ar);

private:
	void _init();


	DECLARE_OBJECT_INFO(HelpData, ObjectInfo);
};

EW_LEAVE
#endif
