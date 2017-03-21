#ifndef __H_EW_UI_APP_APP_CONFIG__
#define __H_EW_UI_APP_APP_CONFIG__

#include "ewc_base/config.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE AppConfig
{
public:

	template<typename T>
	void SetValue(const String& key,const T& value)
	{
		values[key].reset(value);
	}

	template<typename T>
	bool GetValue(const String& key,T& value)
	{
		return values[key].get<T>(value);
	}

	template<typename T>
	bool GetValue(const String& key,T& value,const T& def)
	{
		if(!values[key].get<T>(value))
		{
			value=def;
			return false;
		}
		return true;
	}

	bool Load(const String& s="");
	bool Save(const String& s="");

	void CfgUpdate(int lv,const String& s,int32_t& v,int v1,int v2);
	void CfgUpdate(int lv,const String& s,int32_t& v);
	void CfgUpdate(int lv,const String& s,String& v);
	void CfgUpdate(int lv,const String& s,arr_1t<String>& v);

	void CfgUpdate(int lv,const String& s,BitFlags& v,int m);


	typedef VariantTable map_type;
	map_type values;
	String s_file;
};

EW_LEAVE
#endif
