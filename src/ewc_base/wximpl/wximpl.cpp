#include "../wximpl.h"
#include "ewc_base/wnd/impl_wx/window.h"

EW_ENTER

bool GuiImplWx::Init(int argc, char** argv)
{
	wxApp::SetInstance(new wxApp);

	if (!wxEntryStart(argc, argv))
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
			lv = LogSource::current().get("Wxmsgs");
		}

		virtual void DoLogRecord(wxLogLevel level,
			const wxString& msg,
			const wxLogRecordInfo& info)
		{
			if (level == LOGLEVEL_COMMAND) return;
			ew::LogRecord rcd(wx2str(msg), lv, 0, LOGLEVEL_MESSAGE);

			if (level == wxLOG_Warning) rcd.m_nLevel = LOGLEVEL_WARNING;
			if (level == wxLOG_Error) rcd.m_nLevel = LOGLEVEL_ERROR;
			if (level == wxLOG_FatalError) rcd.m_nLevel = LOGLEVEL_FATAL;

			Logger::def()->Handle(rcd);
		}
	};

	delete ::wxLog::SetActiveTarget(new WxLogRedirector);

	return true;
}

int GuiImplWx::MainLoop()
{
	return wxApp::GetInstance()->OnRun();
}

void GuiImplWx::Fini()
{
	::wxEntryCleanup();
}


void GuiImplWx::ExitLoop()
{
	wxApp::GetInstance()->ExitMainLoop();
}



EW_LEAVE
