#ifndef __H_EW_UI_APP__
#define __H_EW_UI_APP__


#include "ewc_base/config.h"
#include "ewc_base/app/app_config.h"
#include "ewc_base/wnd/wnd_manager.h"
#include "ewc_base/app/wrapper.h"

#pragma push_macro("new")
#undef new
#include "wx/wx.h"
#pragma pop_macro("new")

#include <msvc/wx/setup.h>


EW_ENTER

class DLLIMPEXP_EWC_BASE App : public NonCopyable
{
public:

	App(int argc=0,char** argv=NULL);
	~App();

	AppConfig conf;

	static App& current();

	static bool Init(int argc=0,char** argv=NULL);
	static void Fini();

	static int MainLoop();

	static void ReqExit();
	static void ExitLoop();

	static bool TestDestroy();

	static bool IsOk();

	static wxWindow* GetDummyParent();

	operator bool();

};

EW_LEAVE
#endif
