#ifndef __H_EW_UTIL_REGEX__
#define __H_EW_UTIL_REGEX__

#include "ewa_base/basic.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE regex
{
public:


	regex();
	regex(const regex& r);
	regex(const String& s);

	regex& operator=(const regex& o);

	bool assign(const String& s);

private:
	DataPtrT<ObjectData> pimpl;
};

class DLLIMPEXP_EWA_BASE cmatch
{
public:
	String operator[](size_t i) const;
	arr_1t<arr_1t<std::pair<const char*,const char*> > > matchs;
	String orig_str;
};

DLLIMPEXP_EWA_BASE bool regex_match(const String& s,regex& re);
DLLIMPEXP_EWA_BASE bool regex_match(const String& s,cmatch& res,regex& re);
DLLIMPEXP_EWA_BASE bool regex_search(const String& s,cmatch& res,regex& re);

EW_LEAVE

#endif
