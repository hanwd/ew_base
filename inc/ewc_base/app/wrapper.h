#ifndef __H_EW_UI_APP_WRAPPER__
#define __H_EW_UI_APP_WRAPPER__

#include "ewc_base/config.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE Wrapper
{
public:

	static int FileDialog(arr_1t<String>& files,int type=IDefs::FD_OPEN,const String& title="",const String& exts="");	
	static int FileDialog(String& file,int type=IDefs::FD_OPEN,const String& title="",const String& exts="");	

	static int MsgsDialog(const String& cont,int type=IDefs::BTN_OK,const String& title="");
	static int LogsDialog(arr_1t<LogRecord>& records,int level,const String& title="");

	static bool SetClipboardText(const String& s);
	static bool GetClipboardText(String& s);

};

EW_LEAVE

#endif
