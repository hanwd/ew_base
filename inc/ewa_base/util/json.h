#ifndef __H_EW_JSON__
#define __H_EW_JSON__

#include "ewa_base/scripting/variant.h"

EW_ENTER

DLLIMPEXP_EWA_BASE Variant parse_json(const String& json,bool unescape_value=false);
DLLIMPEXP_EWA_BASE void to_json(const Variant& json,StringBuffer<char>& sb);

EW_LEAVE
#endif
