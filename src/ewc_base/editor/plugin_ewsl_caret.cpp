#include "plugin_ewsl_caret.h"


EW_ENTER


BEGIN_EVENT_TABLE (EwslCaret, basetype)
    EVT_STC_START_DRAG(wxID_ANY,EwslCaret::OnDrag)
	EVT_STC_CHARADDED(wxID_ANY,EwslCaret::OnCharAdd)
	EVT_CHAR_HOOK(EwslCaret::OnCharHook)
	EVT_MOUSE_EVENTS(EwslCaret::OnMouseEvents)
	EVT_STC_HOTSPOT_CLICK(wxID_ANY,EwslCaret::OnCallTip)
END_EVENT_TABLE()

void EwslCaret::TestPos(int flag)
{
	long epos=GetInsertionPoint();
	if(epos>=pos1)
	{
		xpos=epos;
	}

	if((flag&CARET_MOVETO)!=0)
	{
		SetInsertionPoint(xpos);
		SetSelection(xpos,xpos);
	}
}

bool EwslCaret::CanCut() const
{
	if(Handler.busy) return false;
	long p1,p2;
	GetSelection(&p1,&p2);
	if(p1>=pos1&&p2>=pos1)
	{
		return basetype::CanCut();
	}
	return false;
}



void EwslCaret::Cut()
{
	if(Handler.busy) return;
	long p1,p2;
	GetSelection(&p1,&p2);
	if(p1>=pos1&&p2>=pos1)
	{
		basetype::Cut();
		TestPos(CARET_NONE);
	}
}

void EwslCaret::Clear()
{
	if(Handler.busy) return;
	long p1,p2;
	GetSelection(&p1,&p2);
	if(p1>=pos1&&p2>=pos1)
	{
		basetype::Clear();
	}
}

void EwslCaret::ClearAll()
{
	basetype::ClearAll();
	//CmdPrepare();
}

bool EwslCaret::CanPaste() const
{
	if(Handler.busy) return false;
	long p1,p2;
	GetSelection(&p1,&p2);
	if(p1>=pos1&&p2>=pos1)
	{
		return basetype::CanPaste();
	}
	return false;

}

void EwslCaret::Paste()
{
	if(!CanPaste()) return;
	basetype::Paste();
	TestPos(CARET_NONE);
}

void EwslCaret::Copy()
{
	if(HasSelection())
	{
		basetype::Copy();
	}
	else if(!Handler.busy)
	{
		CmdPrepare();
	}
}

EwslCaret::EwslCaret(wxWindow* p,const WndPropertyEx& x,EwslIface& h)
	:IWnd_stc(p,x)
	,Handler(h)
{

	tempp.flags.clr(FLAG_INDENT|FLAG_WRAPMODE);
	UpdateStyle();

	Handler.reset(this);
	pos0=pos1=xpos=0;
	CmdPrepare(CARET_PROMPT);
}

EwslCaret::~EwslCaret()
{

}

void EwslCaret::ChangeValue(const wxString& value)
{
	this->SetText(value);
}


void EwslCaret::PendingAppend(const wxString& txt)
{
	SetInsertionPointEnd();

	if(pos0>=0)
	{
		AppendText("\n");
		pos0=-1;
	}
		
	AppendText(txt+"\n");
	EmptyUndoBuffer();

	long xpos=this->GetLastPosition();
	SetSelection(xpos,xpos);
}

wxString EwslCaret::GetRange(long from, long to) const
{
	return const_cast<EwslCaret*>(this)->GetTextRange(from,to);
}

void EwslCaret::PendingExecute()
{

	long epos=GetLastPosition();
	SetInsertionPoint(epos);
	wxString cmd=GetRange(pos1,epos);
	if(cmd==wxEmptyString)
	{
		CmdPrepare();
	}
	else
	{
		Handler.PendingExecute(wx2str(cmd));
		m_vectCmdHistory.push_back(cmd);
		ncmd=m_vectCmdHistory.size();
	}
}

void EwslCaret::OnCharHook(wxKeyEvent& evt)
{
	if(Handler.busy) return;

	if(evt.ControlDown())
	{
		evt.Skip();			
		return;
	}

	int ec=evt.GetKeyCode();
	switch(ec)
	{
		case WXK_UP:
		case WXK_DOWN:
		{

			int ln=this->GetCurrentLine();
			int lnmax=this->GetLineCount()-1;

			if(lnmax!=line) break;

			if(ec==WXK_UP)
			{
				if(ln>line)
				{
					break;
				}
			}
			if(ec==WXK_DOWN)
			{
				if(ln!=line)
				{
					break;
				}
			}
				
			//this->GetLineEndPosition(ln);

			int k=ncmd+(ec==WXK_UP?-1:+1);
			if(k<0||k>=(int)m_vectCmdHistory.size())
			{
				return;
			}
			ncmd=k;
			long pos2=GetLastPosition();
			this->Replace(pos1,pos2,m_vectCmdHistory[k]);
			pos2=GetLastPosition();
			this->SetSelection(pos2,pos2);

			return;
		}
		case WXK_TAB:
		{
			return;
		}
		break;
		case WXK_LEFT:
		case WXK_RIGHT:
		{
			TestPos(CARET_NONE);
			int tmpp=xpos+(ec==WXK_RIGHT?+1:-1);
			if(tmpp<pos1)
			{
				return;
			}
			xpos=tmpp;

		}
		break;
		case WXK_DELETE:
		{
			if(xpos<pos1)
			{
				return;
			}
		}
		break;
		case WXK_BACK:
		{
			if(this->HasSelection())
			{
				if(!this->CanCut())
				{
					return;
				}
				xpos=this->GetSelectionStart();
			}
			else
			{
				TestPos(CARET_NONE);
				int tmpp=xpos-1;
				if(tmpp<pos1)
				{
					return;
				}
				xpos=tmpp;
			}
		}
		break;
		case WXK_RETURN:
		{
			if(!evt.ShiftDown())
			{
				PendingExecute();
				return;
			}

		}
		break;
		case WXK_CONTROL:
		case WXK_SHIFT:
		case WXK_ALT:
			break;
		default:
			TestPos();
			break;
	};

	evt.Skip();
}
	

void EwslCaret::OnCharAdd(wxStyledTextEvent &evt)
{
	(void)&evt;
	TestPos();
}

void EwslCaret::OnDrag(wxStyledTextEvent &evt)
{
	(void)&evt;
	evt.SetDragText(wxEmptyString);
}

void EwslCaret::OnCallTip(wxStyledTextEvent &evt)
{
	(void)&evt;
	wxString ss=evt.GetText();
	wxString s2=evt.GetString();
	ss=s2;
		
}

void EwslCaret::CmdPrepare(int flag)
{
	if((flag&CARET_BUSY)==0)
	{
		Handler.busy=false;
	}

	SetInsertionPointEnd();
	if((flag&CARET_NEWLINE)!=0)
	{
		AppendText("\n");
	}

	if((flag&CARET_PROMPT)!=0)
	{
		AppendText("EastWave Scripting Language\n\n");
	}

	pos0=GetLastPosition();
	AppendText(">> ");
	SetInsertionPointEnd();
	pos1=xpos=GetLastPosition();
	line=this->GetLineCount()-1;
	SetSelection(pos1,pos1);
	this->GotoPos(pos1);
	EmptyUndoBuffer();

}

void EwslCaret::OnMouseEvents(wxMouseEvent& evt)
{
	if(evt.RightUp())
	{
		//wxMenu* mu=CmdProc().CreateEditMenu();
		//this->PopupMenu(mu);
		evt.Skip();
	}
	else
	{
		evt.Skip();
	}
}

EW_LEAVE
