#ifndef __UI_IWND_FLASH__
#define __UI_IWND_FLASH__

#include "ewc_base/wnd/impl_wx/iwnd_bookbase.h"
class wxActiveXEvent;

EW_ENTER

class FlashCanvasData;

class DLLIMPEXP_EWC_BASE IWnd_flash : public IWnd_bookbase
{
public:
	typedef IWnd_bookbase basetype;

	IWnd_flash(wxWindow* w);
	~IWnd_flash();

	void LoadFile(const String& movie);
	void LoadURL(const String& url,int layer=0);

	void Play();
	void Stop();
	void Forward();
	void Backward();
	void Rewind();
	void GotoFrame(long f);
	bool IsPlaying();
	long CurrentFrame();

	String GetVariable(const String& var);
	void SetVariable(const String& var,const String& val);


	void OnActiveXEvent(wxActiveXEvent& event);

	ValidatorHolderT<Validator> m_pVald;
protected:

	IWnd_flash(const IWnd_flash&);
	IWnd_flash& operator=(const IWnd_flash&);

	FlashCanvasData* m_pData;

	DECLARE_EVENT_TABLE();
};

EW_LEAVE

#endif
