#include "ewa_base/scripting/helpdata.h"
#include "ewa_base/collection/bst_set.h"
#include "ewa_base/serialization/serializer.h"
#include "ewa_base/util/strlib.h"

EW_ENTER



void HelpData::_init()
{
	helpmap.clear();

	helpmap["name"];
	helpmap["desc"];
	helpmap["alias"];
	helpmap["usage"];
	helpmap["example"];
	helpmap["see also"];
	helpmap["source"];

}

HelpData::HelpData()
{
	_init();
}

HelpData::HelpData(const String& s, const String& v)
{
	parse(s, v);
}


static String string_remove_item(const String& src, bst_set<String>& alias)
{
	arr_1t<String> items = string_split(src, ",");
	arr_1t<String>::iterator it_end = std::remove_if(items.begin(), items.end(), [&alias](const String& s){return alias.find(s) != alias.end(); });
	alias.insert(items.begin(), it_end);

	return string_join(items.begin(), it_end, ",");
}

bool HelpData::ToValue(String& s, int n) const
{
	String desc=helpmap["desc"];
	if ( desc == "")
	{
		if(helpmap["source"]=="")
		{
			s = "no_help\n";
			return true;
		}
		desc="no description";
	}

	bst_set<String> alias;
	s = helpmap["name"];

	if (s == "")
	{
		s = "[unnamed]";
	}
	else
	{
		alias.insert(s);
	}

	s = s + ":\n";
	s = s + desc + "\n\n";

	if (helpmap["alias"] != "")
	{
		s = s + "alias:\n" + string_remove_item(helpmap["alias"], alias) + "\n\n";
	}

	if (helpmap["usage"] != "")
	{
		s = s + "usage:\n" + helpmap["usage"]+"\n\n";
	}

	if (helpmap["example"] != "")
	{
		s = s + "example:\n" + helpmap["example"]+"\n\n";
	}

	if (helpmap["see also"] != "")
	{
		s = s + "see also:\n" + string_remove_item(helpmap["see also"],alias) + "\n\n";
	}

	if (helpmap["source"] != "")
	{
		s = s + "source:\n" + helpmap["source"] + "\n\n";
	}

	return true;
}

void HelpData::parse(const String& name_, const String& desc)
{
	_init();

	String name("desc");
	helpmap["name"] = name_;

	arr_1t<String> lines = string_lines(desc);
	arr_1t<String>::const_iterator it1 = lines.cbegin();

	for (arr_1t<String>::const_iterator it2 = lines.cbegin();;it2++)
	{
		if (it2 == lines.cend())
		{
			helpmap[name] = string_trim(string_join(it1, it2, "\n"), TRIM_B, '\n');
			break;
		}

		const String& line(*it2);
		if (line.empty()) continue;

		const char* pstr = line.c_str();
		size_t nlen = line.size();

		if(pstr[0]!=' ' && pstr[0]!='\t' && pstr[nlen-1] == ':')
		{
			helpmap[name] = string_trim(string_join(it1, it2, "\n"), TRIM_B, '\n');
			name = (*it2).substr(0, nlen-1);
			it1 = it2+1;
		}		
	}

}

void HelpData::Serialize(SerializerHelper sh)
{
	Serializer& ar(sh.ref(0));
	ar & helpmap;
}

IMPLEMENT_OBJECT_INFO(HelpData, ObjectInfo);

EW_LEAVE

