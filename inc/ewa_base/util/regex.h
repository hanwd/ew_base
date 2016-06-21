#ifndef __H_EW_UTIL_REGEX__
#define __H_EW_UTIL_REGEX__

#include "ewa_base/basic.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE Match;

class DLLIMPEXP_EWA_BASE Regex
{
public:

	enum
	{
		FLAG_CASE_INSENSITIVE =1<<0,
	};


	Regex();
	Regex(const Regex& r);
	Regex(const String& s,int f=0);

	Regex& operator=(const Regex& o);

	bool assign(const String& s,int f=0);

	bool match(const String& s);
	bool match(const String& s,Match& m);

	bool search(const String& s,Match& m);

private:
	DataPtrT<ObjectData> pimpl;
};

class DLLIMPEXP_EWA_BASE Match
{
public:

	class item
	{
	public:
		item(const char* p1=NULL,const char* p2=NULL):it_beg(p1),it_end(p2){}
		const char* it_beg;
		const char* it_end;
		operator String() const {return String(it_beg,it_end);}
	};

	class item_array : public arr_1t<item>
	{
	public:
		operator String() const;
	};

	const item_array& operator[](size_t i) const
	{
		return matchs[i];
	}

	size_t size() const {return matchs.size();}

	String replace(const String& dst) const;

	bool search_next();

	typedef arr_1t<item_array>::const_iterator iterator;
	iterator begin(){return matchs.cbegin();}
	iterator end(){return matchs.cend();}

	arr_1t<item_array> matchs;
	String orig_str;
	Regex orig_reg;
};

DLLIMPEXP_EWA_BASE bool regex_match(const String& s,Regex& re);
DLLIMPEXP_EWA_BASE bool regex_match(const String& s,Match& res,Regex& re);
DLLIMPEXP_EWA_BASE bool regex_search(const String& s,Match& res,Regex& re);

DLLIMPEXP_EWA_BASE String regex_replace(const String& s,Regex& re,const String& p);

EW_LEAVE

#endif
