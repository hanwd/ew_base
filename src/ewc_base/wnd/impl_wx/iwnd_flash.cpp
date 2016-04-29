
#include "ewc_base/wnd/impl_wx/iwnd_flash.h"
#include "wx/filename.h"

#pragma warning(disable:4005)
#include "wx/msw/ole/activex.h"

#import "libid:D27CDB6B-AE6D-11CF-96B8-444553540000" no_auto_exclude

using namespace ShockwaveFlashObjects;

const CLSID CLSID_ShockwaveFlash = __uuidof(ShockwaveFlash);
const IID IID_IShockwaveFlash = __uuidof(IShockwaveFlash);

// taken from type library
namespace
{

const int FLASH_DISPID_ONREADYSTATECHANGE = -609; // DISPID_ONREADYSTATECHANGE
const int FLASH_DISPID_ONPROGRESS = 0x7a6;
const int FLASH_DISPID_FSCOMMAND = 0x96;
const int FLASH_DISPID_FLASHCALL = 0xc5;

enum FlashState
{
    FlashState_Unknown = -1,
    FlashState_Loading,
    FlashState_Uninitialized,
    FlashState_Loaded,
    FlashState_Interactive,
    FlashState_Complete,
    FlashState_Max
};

} // anonymous namespace


EW6_ENTER

class FlashCanvasData
{
public:

	FlashCanvasData()
	{
		m_nState=FlashState_Unknown;
	

		IShockwaveFlash* flash=NULL;

		HRESULT hr = ::CoCreateInstance
					(
					CLSID_ShockwaveFlash,
					NULL,
					CLSCTX_INPROC_SERVER,
					IID_IShockwaveFlash,
					(void **)&flash
					);

		new char[1024];

		if ( FAILED(hr) )
		{
			System::LogError("CoCreateInstance flash failed!");
			return ;
		}

		m_pFlash=flash;
		m_pFlash->PutAllowScriptAccess(L"always");
	}


	void LoadFile(const String& movie)
	{
		if(!m_pFlash) return;

		wxFileName fn(movie.c_str());
		fn.MakeAbsolute();
		const wxString swf = fn.GetFullPath();

		if(swf==m_sUri.c_str())
		{
			m_pFlash->PutMovie(L"");
		}
		else
		{
			m_sUri=swf.c_str().AsChar();
			m_pFlash->PutMovie(swf.wc_str());
		}
	}

	void LoadURL(const String& url,int layer)
	{
		if(!m_pFlash) return;

		m_sUri=url;
		StringBuffer<wchar_t> wurl(url);
		m_pFlash->LoadMovie(layer,wurl.c_str());
	}

	bool CheckFlashCall(HRESULT hr, const char *func)
	{
		if ( FAILED(hr) )
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	bool Play()
	{
		if(!m_pFlash) return false;
		return CheckFlashCall(m_pFlash->Play(), "Play");
	}

	bool Stop()
	{
		if(!m_pFlash) return false;
		return CheckFlashCall(m_pFlash->Stop(), "Stop");
	}

	bool Forward()
	{
		if(!m_pFlash) return false;
		return CheckFlashCall(m_pFlash->Back(), "Forward");
	}

	bool Backward()
	{
		if(!m_pFlash) return false;
		return CheckFlashCall(m_pFlash->Back(), "Back");
	}

	bool Rewind()
	{
		if(!m_pFlash) return false;
		return CheckFlashCall(m_pFlash->Rewind(), "Rewind");
	}

	bool GotoFrame(long f)
	{
		if(!m_pFlash) return false;
		return CheckFlashCall(m_pFlash->GotoFrame(f), "GotoFrame");
	}

	bool IsPlaying()
	{
		if(!m_pFlash) return false;
		return m_pFlash->IsPlaying()!=0;
	}

	long CurrentFrame()
	{
		if(!m_pFlash) return -1;
		return m_pFlash->CurrentFrame();
	}

	long GetFrameNum()
	{
		if(!m_pFlash) return 0;
		return m_pFlash->GetFrameNum();
	}

	long GetTotalFrames()
	{
		if(!m_pFlash) return 0;
		return m_pFlash->GetTotalFrames();
	}

	String GetFlashStateString(int state)
	{
		static const char *knownStates[] =
		{
			"Loading", "Uninitialized", "Loaded", "Interactive", "Complete",
		};

		if ( state >= 0 && state < sizeof(knownStates)/sizeof(char*) )
		{
			return knownStates[state];
		}

		return String::Format("unknown state (%d)", state);
	}


	String GetVariable(const String& var)
	{
		if(!m_pFlash) return "";
		StringBuffer<wchar_t> wvar(var);	
		const wchar_t *v=m_pFlash->GetVariable(wvar.c_str());
		return v;
	}

	void SetVariable(const String& var,const String& val)
	{
		if(!m_pFlash) return;
		StringBuffer<wchar_t> wvar(var);
		StringBuffer<wchar_t> wval(val);
		m_pFlash->SetVariable(wvar.c_str(),wval.c_str());
	}


	String CallFlashFunc(const String& argtype,
									const String& func,
									const String& arg)
	{
		if(!m_pFlash) return "";

		String args;
		StringBuffer<wchar_t> request;

		if ( !argtype.empty() )
		{
			args = String::Format("<%s>%s</%s>", argtype, arg, argtype);
		}

		// take care with XML formatting: there should be no spaces in it or the
		// call would fail!
		request = String::Format
							(
								"<invoke name=\"%s\" returntype=\"xml\">"
									"<arguments>"
									"%s"
									"</arguments>"
								"</invoke>",
								func,
								args
							);

		const wchar_t* v=m_pFlash->CallFunction(request.c_str());
		return v;
	}

	wxActiveXContainer* CreateCanvas(wxWindow* parent)
	{
		return m_pFlash?new wxActiveXContainer(parent, IID_IShockwaveFlash, m_pFlash):NULL;
	}

	void SetState(int state)
	{
		if ( state == m_nState ) return;

		if ( state >= 0 && state < FlashState_Max )
		{
			m_nState = static_cast<FlashState>(state);
		}
		else
		{
			m_nState = FlashState_Unknown;
		}
	}

	int GetState()
	{
		if(!m_pFlash) return -1;	
		const int state = m_pFlash->GetReadyState();
		return state;
	}

	~FlashCanvasData()
	{
		
	}

protected:
	IShockwaveFlashPtr m_pFlash;
	String m_sUri;
	FlashState m_nState;
};


IWnd_flash::~IWnd_flash()
{
	delete m_pData;
}

IWnd_flash::IWnd_flash(wxWindow* w)
	:basetype(w)
{
	this->SetWindowStyle(wxBORDER_NONE);
	m_pData=new FlashCanvasData;
	SelPage(m_pData->CreateCanvas(this));
}

void IWnd_flash::LoadFile(const String& movie)
{
	m_pData->LoadFile(movie);
}

void IWnd_flash::LoadURL(const String& url,int layer)
{
	m_pData->LoadURL(url,layer);
}

void IWnd_flash::Play()
{
	m_pData->Play();
}

void IWnd_flash::Stop()
{
	m_pData->Stop();
}

void IWnd_flash::Forward()
{
	m_pData->Forward();
}

void IWnd_flash::Backward()
{
	m_pData->Backward();
}

void IWnd_flash::Rewind()
{
	m_pData->Rewind();
}

void IWnd_flash::GotoFrame(long f)
{
	m_pData->GotoFrame(f);
}

bool IWnd_flash::IsPlaying()
{
	return m_pData->IsPlaying();
}

long IWnd_flash::CurrentFrame()
{
	return m_pData->CurrentFrame();
}

String IWnd_flash::GetVariable(const String& var)
{
	return m_pData->GetVariable(var);
}

void IWnd_flash::SetVariable(const String& var,const String& val)
{
	m_pData->SetVariable(var,val);
}

void IWnd_flash::OnActiveXEvent(wxActiveXEvent& event)
{
	switch ( event.GetDispatchId() )
	{
		case FLASH_DISPID_ONREADYSTATECHANGE:
			m_pData->SetState(event[0].GetInteger());

			break;

		case FLASH_DISPID_ONPROGRESS:
			//("Progress: %d%%", event[0].GetInteger());
			break;

		case FLASH_DISPID_FSCOMMAND:
			//("Flash command %s(%s)", event[0].GetString(), event[1].GetString());
			break;

		case FLASH_DISPID_FLASHCALL:
			//("Flash request \"%s\"", event[0].GetString());
			break;

		default:
			//("Unknown event %ld", event.GetDispatchId());rea
			break;
	}

	event.Skip();
}

BEGIN_EVENT_TABLE(IWnd_flash,basetype)
    EVT_ACTIVEX(wxID_ANY, IWnd_flash::OnActiveXEvent)
END_EVENT_TABLE()

EW6_LEAVE
