#include "ewc_base/wnd/impl_wx/iwnd_stc.h"
#include "plugin_ewsl_iface.h"

EW_ENTER

class EwslIface;

class EwslCaret : public IWnd_stc
{
public:

	typedef IWnd_stc basetype;

	EwslIface& Handler;

	long pos0;	// cmd raw begin
	long pos1;	// cmd begin
	long xpos;	// cmd current pos
	long line;
	long ncmd;

	enum
	{
		CARET_NONE		=0,
		CARET_MOVETO	=1<<0,
		CARET_NEWLINE	=1<<1,
		CARET_PROMPT	=1<<2,
		CARET_BUSY		=1<<3,
	};


	void TestPos(int flag=CARET_MOVETO);

	void Cut();
	void Copy();
	void Paste();
	void Clear();
	void ClearAll();

	bool CanCut() const;
	bool CanPaste() const;

	EwslCaret(wxWindow* p,const WndPropertyEx& x,EwslIface& h);
	~EwslCaret();

	virtual void ChangeValue(const wxString& value);
	void PendingAppend(const wxString& txt);

	virtual wxString GetRange(long from, long to) const;

	void PendingExecute();
	void OnCharHook(wxKeyEvent& evt);
	void OnCharAdd(wxStyledTextEvent &evt);
	void OnDrag(wxStyledTextEvent &evt);
	void OnCallTip(wxStyledTextEvent &evt);
	void CmdPrepare(int flag=CARET_NEWLINE);
	void OnMouseEvents(wxMouseEvent& evt);

	arr_1t<wxString> m_vectCmdHistory;

protected:	

	DECLARE_EVENT_TABLE();
};



EW_LEAVE
