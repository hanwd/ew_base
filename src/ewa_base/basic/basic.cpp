

#include "ewa_base/basic/system.h"
#include "ewa_base/basic/platform.h"
#include "ewa_base/basic/atomic.h"
#include "ewa_base/basic/functor.h"

#ifdef EW_WINDOWS
#include <windows.h>
#endif

EW_ENTER

void OnAssertFailure(const char* msg,const char* file,long line)
{
	System::LogTrace("assert_failure: %s at %s:%d\n",msg,file,line);
	System::DebugBreak();
}

DLLIMPEXP_EWA_BASE tl::int2type<1> _1;
DLLIMPEXP_EWA_BASE tl::int2type<2> _2;
DLLIMPEXP_EWA_BASE tl::int2type<3> _3;
DLLIMPEXP_EWA_BASE tl::int2type<4> _4;
DLLIMPEXP_EWA_BASE tl::int2type<5> _5;
DLLIMPEXP_EWA_BASE tl::int2type<6> _6;

#ifdef EW_WINDOWS
void KO_Policy_handle::destroy(type& o)
{
	::CloseHandle(o);
}
#else
void KO_Policy_handle::destroy(type& o)
{
	::close(o);
}
#endif

EW_LEAVE
