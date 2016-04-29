#ifdef EW_WINDOWS

#include "socket_impl_windows.h"
typedef ew::SocketImpl_windows socket_detail;

#else

#include "socket_impl_linux.h"
typedef ew::SocketImpl_linux socket_detail;

#endif

