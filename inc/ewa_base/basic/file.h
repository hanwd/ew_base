#ifndef __H_EW_BASIC_FILE__
#define __H_EW_BASIC_FILE__

#include "ewa_base/basic/string.h"
#include "ewa_base/basic/platform.h"
#include "ewa_base/basic/stream.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE File : public Stream
{
public:

	static bool Rename(const String& oldname_,const String& newname_);
	static bool Remove(const String& filename_);

	File();
	File(const String& filename_,int op=FileAccess::FLAG_RD);

	void Truncate(size_t size_);

	KO_Handle<KO_Policy_handle>::type native_handle();


};


EW_LEAVE
#endif
