

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
	System::LogTrace("ASSERT_FAILURE: %s at %s:%d\n",msg,file,line);
	System::DebugBreak();
}

void OnNoop()
{

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

KO_Policy_handle::type KO_Policy_handle::duplicate(type o,int h)
{
	type handle=invalid_value();
	if(o!=invalid_value())
	{
		void* hProcess=::GetCurrentProcess();	
		DuplicateHandle(hProcess,o,hProcess,&handle,0,h?TRUE:FALSE,DUPLICATE_SAME_ACCESS);
	}

	return handle;
}

#else
void KO_Policy_handle::destroy(type& o)
{
	::close(o);
}
KO_Policy_handle::type KO_Policy_handle::duplicate(type o)
{
	type h=dup(o);
	return h;
}
#endif

EW_LEAVE
