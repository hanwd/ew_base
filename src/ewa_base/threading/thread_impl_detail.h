#ifdef EW_MSVC
#include "thread_impl_windows.h"
typedef ew::ThreadImpl_windows ThreadImpl_detail;
#else
#include "thread_impl_pthread.h"
typedef ew::ThreadImpl_pthread ThreadImpl_detail;
#endif

