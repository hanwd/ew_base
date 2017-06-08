#include "ewc_base/app/app.h"
#include "ewc_base/app/res_manager.h"
#include "ewc_base/wnd/wnd_maker.h"
#include "ewc_base/wnd/wnd_updator.h"
#include "ewc_base/wnd/wnd_manager.h"

#include "../wximpl.h"


#ifdef EWC_BASE_DLL

BOOL APIENTRY DllMain(HANDLE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
	EW_UNUSED(hModule);
	EW_UNUSED(ul_reason_for_call);
	EW_UNUSED(lpReserved);

	return TRUE;
}

#endif

EW_ENTER


class IDat_internal
{
public:

	IDat_internal()
	{
		nInitCount = 0;
		bInitOk = false;
		bDestroying = false;
		pTopWindow = NULL;
	}

	int nInitCount;
	bool bInitOk;
	bool bDestroying;

	wxWindow* pTopWindow;

	AtomicSpin spin;

	static IDat_internal& current()
	{
		static IDat_internal gInstance;
		return gInstance;
	}
};



App& App::current()
{
	return WndManager::current().app;
}

bool App::Init(int argc,char** argv)
{
	if(IDat_internal::current().nInitCount++==0 && GuiImplWx::Init(argc, argv))
	{
		IDat_internal::current().bInitOk = true;
		ResManager::current();
		set_logs_dialog_function(&Wrapper::LogsDialog);
	}

	return IDat_internal::current().bInitOk;	
}



int App::MainLoop()
{
	if(!IDat_internal::current().bInitOk) return -1;

	return GuiImplWx::MainLoop();
}



void App::Fini()
{
	if(--IDat_internal::current().nInitCount==0)
	{
		IDat_internal::current().bInitOk=false;
		set_logs_dialog_function(NULL);
		GuiImplWx::Fini();
	}
}


void App::ExitLoop()
{
	GuiImplWx::ExitLoop();
}

void App::ReqExit()
{
	IDat_internal::current().bDestroying=true;
}

bool App::TestDestroy()
{
	return IDat_internal::current().bDestroying;
}

App::App(int argc,char** argv)
{
	Init(argc,argv);
}

App::~App()
{
	Fini();
}


bool App::IsOk()
{
	return IDat_internal::current().bInitOk;
}

App::operator bool()
{
	return IDat_internal::current().bInitOk;
}

EW_LEAVE
