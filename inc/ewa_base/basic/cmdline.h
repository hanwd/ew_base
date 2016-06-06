#ifndef __H_EW_BASIC_CMDLINE__
#define __H_EW_BASIC_CMDLINE__

#include "ewa_base/config.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/collection/arr_1t.h"
#include "ewa_base/collection/bst_set.h"
#include "ewa_base/collection/bst_map.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE CmdLineParser
{
public:

	enum
	{
		CMDLINE_SWITCH,
		CMDLINE_OPTION,
		CMDLINE_PARAM,
	};

	CmdLineParser();

	void AddSwitch(const String& s);
	void AddOption(const String& s);
	void AddOption(const String& s,const String& d);

	void AddParam(int n = 1);

	bool parse(int argc, char** argv);
	bool parse(const arr_1t<String>& argv);

	bool Found(const String& s);
	bool Found(const String& s, String& v);
	bool Found(const String& s, int32_t& v);
	bool Found(const String& s, int64_t& v);

	size_t GetParamCount();
	String& GetParam(size_t i = 0);


	typedef bst_map<String, String>::iterator option_iterator;
	option_iterator options_begin()
	{
		return m_aOptions.begin();
	}

	option_iterator options_end()
	{
		return m_aOptions.end();
	}


private:

	class OptionType
	{
	public:
		OptionType():type(0),tags(0){}
		void set(int t,int n=0){type=t;tags=n;}
		int type;
		int tags;
	};

	bst_map<String, OptionType> m_aCmdLineOptions;
	bst_map<String, String> m_aOptionsDefault;

	arr_1t<String> m_aParams;
	bst_set<String> m_aSwitchs;
	bst_map<String, String> m_aOptions;

};

EW_LEAVE
#endif
