#include "ewc_base/app/res_manager.h"
#include "ewc_base/wnd/wnd_maker.h"
#include "ewc_base/app/app.h"

#include "wx/app.h"
#include "ewc_base/wnd/impl_wx/window.h"

#ifdef EWC_BASE_DLL
#include <msvc/wx/setup.h>


BOOL APIENTRY DllMain(HANDLE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
	EW_UNUSED(hModule);
	EW_UNUSED(ul_reason_for_call);
	EW_UNUSED(lpReserved);

	return TRUE;
}

#endif

EW_ENTER


class AppData
{
public:

	int nInitCount;
	bool bInitOk;
	bool bDestroying;
	wxWindow* pTopWindow;

	AtomicSpin spin;
};

static AppData app_data;

App& App::current()
{
	static App gInstance;
	return gInstance;
}

bool App::Init(int argc,char** argv)
{
	if(app_data.nInitCount++==0)
	{
		wxApp::SetInstance(new wxApp);

		app_data.bInitOk=wxEntryStart(argc,argv);

		if(!app_data.bInitOk)
		{
			return false;
		}

		wxInitAllImageHandlers();

		class WxLogRedirector : public wxLog
		{
		public:

			int lv;

			WxLogRedirector()
			{
				lv=LogSource::current().get("Wxmsgs");
			}

			virtual void DoLogRecord(wxLogLevel level,
								const wxString& msg,
								const wxLogRecordInfo& info)
			{
	
				ew::LogRecord rcd(msg.c_str().AsChar(),lv,0,LOGLEVEL_MESSAGE);

				if(level==wxLOG_Warning) rcd.m_nLevel=LOGLEVEL_WARNING;
				if(level==wxLOG_Error) rcd.m_nLevel=LOGLEVEL_ERROR;
				if(level==wxLOG_FatalError) rcd.m_nLevel=LOGLEVEL_FATAL;

				Logger::def()->Handle(rcd);
			}
		};

		delete ::wxLog::SetActiveTarget(new WxLogRedirector);

		ResManager::current();

		CG_GGVar &gi(CG_GGVar::current());
		gi.add(new CallableMaker,"ui.maker");
	}

	return app_data.bInitOk;	
}



int App::MainLoop()
{
	if(!app_data.bInitOk) return -1;
	return wxApp::GetInstance()->OnRun();
}



void App::Fini()
{
	if(--app_data.nInitCount==0)
	{
		app_data.bInitOk=false;
		::wxEntryCleanup();
	}
}


void App::ExitLoop()
{
	wxApp::GetInstance()->ExitMainLoop();
}

void App::ReqExit()
{
	app_data.bDestroying=true;
}

bool App::TestDestroy()
{
	return app_data.bDestroying;
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
	return app_data.bInitOk;
}

wxWindow* App::GetDummyParent()
{
	return NULL;
}

App::operator bool()
{
	return app_data.bInitOk;
}

EW_LEAVE
