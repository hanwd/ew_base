
#include "ewc_base/wnd/impl_wx/iwnd_stc.h"

EW_ENTER

StcStyleFlag::StcStyleFlag()
{
	flags.clr(IWnd_stc::FLAG_DEFAULT);
	nsize=14;
	wdtab=4;
	nlang=0;
}

StcStyleFlag IWnd_stc::ms_param;

BEGIN_EVENT_TABLE (IWnd_stc, wxStyledTextCtrl)
    EVT_STC_MARGINCLICK (wxID_ANY,IWnd_stc::OnMarginClick)
	EVT_STC_CHARADDED (wxID_ANY,IWnd_stc::OnCharAdded)
	EVT_CHAR_HOOK(IWnd_stc::OnKeyEvent)
	EVT_STC_UPDATEUI(wxID_ANY,IWnd_stc::OnStyledTextChanged)
	EVT_MOUSEWHEEL(IWnd_stc::OnMouseWheel)
	EVT_SET_FOCUS(IWnd_stc::OnFocus)
END_EVENT_TABLE()


IWnd_stc::~IWnd_stc()
{

}

bool IWnd_stc::SetFontSize(int fs)
{
	if(fs<6) fs=6;
	if(fs>24) fs=24;
	if(tempp.nsize==fs)
	{
		return false;
	}
	tempp.nsize=fs;
	UpdateStyle();
	return true;
}

void IWnd_stc::OnMouseWheel(wxMouseEvent& evt)
{
	int wr=evt.GetWheelRotation();
	if(wr==0||!evt.ControlDown())
	{
		evt.Skip();
		return;
	}
	SetFontSize(param.nsize+(wr>0?+2:-2));
}


void IWnd_stc::OnFocus(wxFocusEvent& evt)
{
	evt.Skip();
}



void IWnd_stc::OnKeyEvent(wxKeyEvent& evt)
{
	int kc = evt.GetRawKeyCode();

	if(kc==wxSTC_KEY_TAB)
	{
		if(evt.ShiftDown())
		{
			CmdKeyExecute (wxSTC_CMD_BACKTAB);
		}
		else
		{
			CmdKeyExecute (wxSTC_CMD_TAB);
		}
		return;
	}

	if(evt.ControlDown())
	{
		switch(kc)
		{
		case 'C':
			{
				Copy();
			}
			return;
		case 'X':
			{
				Cut();
			}
			return;
		case 'V':
			{
				Paste();
			}
			return;
		case 'A':
			{
				SelectAll();
			}
			return;
		case 'Z':
			{
				Undo();
			}
			return;
		case 'R':
			{
				Redo();
			}
			return;
		case 'D':
			{
				this->Clear();
			}
			return;
		//case 'F':
		//	if(style.get(STYLE_CAN_FIND))
		//	{
		//		WndManager::current().evtmgr["Find"].CmdExecuteEx(-1);
		//		evt.Skip();
		//		return;
		//	}
		//	break;
		//case 'H':
		//	if(style.get(STYLE_CAN_REPLACE))
		//	{
		//		WndManager::current().evtmgr["Replace"].CmdExecuteEx(-1);
		//		evt.Skip();
		//		return;
		//	}
		//	break;
		};
	}

	evt.Skip();
}

void IWnd_stc::UpdateMarginLineNumWidth()
{
	_DoUpdateMarginLineNumWidth(false);
}

void IWnd_stc::_DoUpdateMarginLineNumWidth(bool flag)
{
	BitFlags &flags(param.flags);

	if(!flags.get(FLAG_LINENUM))
	{
		if(m_nLineNumWidth==0)
		{
			return;
		}
		m_nLineNumWidth=0;
		SetMarginWidth(StcManager::LINE_NR_ID,0);
	}
	else
	{

		int _nWidth=3;
		for(int nLine = GetLineCount();nLine > 9;nLine=nLine/10)
		{
			_nWidth++;
		}

		if(_nWidth<5) _nWidth=5;
		if(_nWidth>8) _nWidth=8;


		if (!flag && _nWidth<=m_nLineNumWidth)
		{
			return;
		}

		m_nLineNumWidth=_nWidth;

		static const char* text="999999999999999";
		int wd=TextWidth (wxSTC_STYLE_LINENUMBER, wxString(text,m_nLineNumWidth));
		SetMarginWidth(StcManager::LINE_NR_ID,wd);

	}

}

void IWnd_stc::OnDocumentChanged()
{
	UpdateMarginLineNumWidth();

}
void IWnd_stc::OnStyledTextChanged(wxStyledTextEvent& evt)
{
	if(func) func();
	evt.Skip();
}

int IWnd_stc::GetIndentWidth()
{
	return GetUseTabs() ? GetTabWidth() : 1;
}

void IWnd_stc::OnCharAdded (wxStyledTextEvent &evt)
{
	char chr = (char)evt.GetKey();
	if (chr == '\n')
	{
		int currentLine = GetCurrentLine();
		if (currentLine < 1) return;

		int lineInd = GetLineIndentation(currentLine - 1);
		if (lineInd == 0) return;

		SetLineIndentation (currentLine, lineInd);
		GotoPos(GetLineIndentPosition(currentLine));
	}

}

void IWnd_stc::OnMarginClick (wxStyledTextEvent &evt)
{
	if (evt.GetMargin() == StcManager::FOLDING_ID) 
	{
        int lineClick = LineFromPosition (evt.GetPosition());
        int levelClick = GetFoldLevel (lineClick);
        if ((levelClick & wxSTC_FOLDLEVELHEADERFLAG) > 0) 
		{
            ToggleFold (lineClick);
        }
    }
}

void IWnd_stc::_DoUpdateStyle()
{
	BitFlags& flags(param.flags);

	SetTabWidth (param.wdtab);

	SetUseTabs (true);
	SetTabIndents (true);
	SetBackSpaceUnIndents (true);
	SetIndent (flags.get(FLAG_INDENT)? param.wdtab: 0);

	SetViewEOL (flags.get(FLAG_SHOWEOL));
	SetIndentationGuides (flags.get(FLAG_INDENTGUIDE));
	SetEdgeColumn (80);
	SetEdgeMode (flags.get(FLAG_LONGLINE)?wxSTC_EDGE_LINE: wxSTC_EDGE_NONE);
	SetViewWhiteSpace (flags.get(FLAG_WHITESPACE)?wxSTC_WS_VISIBLEALWAYS: wxSTC_WS_INVISIBLE);
	SetOvertype (flags.get(FLAG_OVERTYPE));
	SetReadOnly (flags.get(FLAG_READONLY));
	SetWrapMode (flags.get(FLAG_WRAPMODE)?wxSTC_WRAP_WORD: wxSTC_WRAP_NONE);

	_DoUpdateMarginLineNumWidth(true);
}




void IWnd_stc::_Init()
{
	param.nlang = -1;

    // Use all the bits in the style byte as styles, not indicators.
    SetStyleBits(8);
    
	wxFont font (ms_param.nsize, wxMODERN, wxNORMAL, wxNORMAL);
    StyleSetFont (wxSTC_STYLE_DEFAULT, font);
    StyleSetForeground(wxSTC_STYLE_DEFAULT, *wxBLACK);
    StyleSetBackground(wxSTC_STYLE_DEFAULT, *wxWHITE);

    StyleSetForeground(wxSTC_STYLE_LINENUMBER,wxColour (wxT("DARK GREY")));
	StyleSetBackground(wxSTC_STYLE_LINENUMBER,*wxWHITE);

    StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour (wxT("DARK GREY")));

    MarkerDefine(wxSTC_MARKNUM_FOLDER,        wxSTC_MARK_BOXPLUS, wxT("WHITE"), wxT("BLACK"));
    MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN,    wxSTC_MARK_BOXMINUS,  wxT("WHITE"), wxT("BLACK"));
    MarkerDefine(wxSTC_MARKNUM_FOLDERSUB,     wxSTC_MARK_VLINE,     wxT("WHITE"), wxT("BLACK"));
    MarkerDefine(wxSTC_MARKNUM_FOLDEREND,     wxSTC_MARK_BOXPLUSCONNECTED, wxT("WHITE"), wxT("BLACK"));
    MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUSCONNECTED, wxT("WHITE"), wxT("BLACK"));
    MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_TCORNER,     wxT("WHITE"), wxT("BLACK"));
    MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL,    wxSTC_MARK_LCORNER,     wxT("WHITE"), wxT("BLACK"));

	UpdateStyle();

    // set visibility
    SetVisiblePolicy (wxSTC_VISIBLE_STRICT|wxSTC_VISIBLE_SLOP, 1);
    SetXCaretPolicy (wxSTC_CARET_EVEN|wxSTC_VISIBLE_STRICT|wxSTC_CARET_SLOP, 1);
    SetYCaretPolicy (wxSTC_CARET_EVEN|wxSTC_VISIBLE_STRICT|wxSTC_CARET_SLOP, 1);

    // annotations
    AnnotationSetVisible(wxSTC_ANNOTATION_BOXED);

    CmdKeyClear (wxSTC_KEY_TAB, 0); // this is done by the menu accelerator key
    SetLayoutCache (wxSTC_CACHE_PAGE);
}


IWnd_stc::IWnd_stc(wxWindow* p,const WndPropertyEx& h,int w)
:basetype(p,h.id(),h,h,w,h.name())
{

	_Init();
	func.bind(&IWnd_stc::OnDocumentChanged,this);

	this->Connect(wxEVT_RIGHT_UP,wxMouseEventHandler(IWnd_controlT<IWnd_stc>::OnContextEditMenu));
	this->Connect(wxEVT_MENU,wxCommandEventHandler(IWnd_controlT<IWnd_stc>::OnCommandEditEvent));
}


void IWnd_stc::SetPropertyEx(const String& id,bool f)
{
	SetProperty(str2wx(id),f?"1":"0");
}

void IWnd_stc::UpdateStyle(const StcStyleFlag& flag)
{
	tempp=flag;
	tempp.nlang=param.nlang;
	UpdateStyle();
}

void IWnd_stc::UpdateStyle(const String& lang)
{
	wxArrayString s=wxSplit(str2wx(lang),'.');
	if(s.size()==0) return;

	wxString ext=wxT("*.")+s.Last().Lower()+wxT(";");
	for(size_t i=0;i<StcManager::current().langs.size();i++)
	{
		StcLangInfo& lf(StcManager::current().langs[i]);
		if(lf.name==lang||str2wx(lf.filepattern).Find(ext)>=0)
		{
			tempp.nlang=i;
			break;
		}
	}

	if(tempp.nlang!=param.nlang)
	{
		UpdateStyle();
	}
}

void IWnd_stc::UpdateStyle()
{

	bool flag =
		param.nlang == tempp.nlang &&
		param.nsize == tempp.nsize &&
		(param.flags.get(FLAG_FOLD) == tempp.flags.get(FLAG_FOLD)) &&
		(param.flags.get(FLAG_SYNTAX) == tempp.flags.get(FLAG_SYNTAX))
		;

	param=tempp;	

	if(flag)
	{
		_DoUpdateStyle();
		return;
	}


	BitFlags& flags(param.flags);

	StcLangInfo& lf(StcManager::current().langs[param.nlang]);

	wxFont fontNr (param.nsize, wxMODERN, wxNORMAL, wxNORMAL,false,str2wx(param.sface));
    for (int Nr = 0; Nr < wxSTC_STYLE_LASTPREDEFINED; Nr++)
	{
        StyleSetFont (Nr, fontNr);
    }

	if(!flags.get(FLAG_FOLD))
	{
		ClearDocumentStyle();
	}

    StyleClearAll();
    SetLexer (lf.lexer);

    SetMarginType (StcManager::LINE_NR_ID, wxSTC_MARGIN_NUMBER);
    StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour (wxT("DARK GREY")));
    StyleSetBackground (wxSTC_STYLE_LINENUMBER, *wxWHITE);
    SetMarginWidth (StcManager::LINE_NR_ID, 0);

    // set common styles
    StyleSetForeground (wxSTC_STYLE_DEFAULT, wxColour (wxT("DARK GREY")));
    StyleSetForeground (wxSTC_STYLE_INDENTGUIDE, wxColour (wxT("DARK GREY")));

	SetMarginType(StcManager::DIVIDER_ID, wxSTC_MARGIN_SYMBOL);
	SetMarginMask(StcManager::DIVIDER_ID, wxSTC_MASK_FOLDERS);
    SetMarginWidth (StcManager::DIVIDER_ID, 0);
    SetMarginSensitive (StcManager::DIVIDER_ID, false);

    // folding
    SetMarginType (StcManager::FOLDING_ID, wxSTC_MARGIN_SYMBOL);
    SetMarginMask (StcManager::FOLDING_ID, wxSTC_MASK_FOLDERS);
    StyleSetBackground (StcManager::FOLDING_ID, *wxWHITE);
    SetMarginWidth (StcManager::FOLDING_ID, 0);
    SetMarginSensitive (StcManager::FOLDING_ID, false);

	if (flags.get(FLAG_FOLD)) 
	{

		SetMarginWidth (StcManager::FOLDING_ID,StcManager::FOLDING_MARGIN);
		SetMarginSensitive (StcManager::FOLDING_ID, true);

		SetPropertyEx ("fold", true);
		SetPropertyEx ("fold.comment",true);
		SetPropertyEx ("fold.compact",true);
		SetPropertyEx ("fold.preprocessor",true);
		SetPropertyEx ("fold.html", true);
		SetPropertyEx ("fold.html.preprocessor",true);
		SetPropertyEx ("fold.comment.python", true);
		SetPropertyEx ("fold.quotes.python", true);
	}
	else
	{
		SetMarginWidth (StcManager::DIVIDER_ID, StcManager::FOLDING_MARGIN);
	}

	SetFoldFlags (wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED| wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);

	if (flags.get(FLAG_SYNTAX))
	{
		int keywordnr = 0;

		for(StcLangInfo::map_type::iterator it=lf.mapStyles.begin();it!=lf.mapStyles.end();++it)
		{
			int Nr=(*it).first;
			StcLangInfo::StyleWords& s((*it).second);
			StcStyleInfo& curType(StcManager::current().style[s.id]);

			wxFont font (param.nsize, wxMODERN, wxNORMAL, wxNORMAL, false,str2wx(curType.fontname));

			StyleSetFont (Nr, font);
			if (curType.foreground!=wxEmptyString) 
			{
				StyleSetForeground (Nr, wxColour (curType.foreground.c_str()));
			}
			if (curType.background!=wxEmptyString) 
			{
				StyleSetBackground (Nr, wxColour (curType.background.c_str()));
			}

			StyleSetBold (Nr, curType.fontstyle.get(StcManager::STYLE_BOLD));
			StyleSetItalic (Nr, curType.fontstyle.get(StcManager::STYLE_ITALIC));
			StyleSetUnderline (Nr, curType.fontstyle.get(StcManager::STYLE_UNDERL));
			StyleSetVisible (Nr, !curType.fontstyle.get(StcManager::STYLE_HIDDEN));

			//StyleSetCase (Nr, curType.lettercase);
			if (s.words!="") 
			{
				SetKeyWords (keywordnr, str2wx(s.words));
				keywordnr += 1;
			}
		}
	}

	_DoUpdateStyle();

	Colourise(0,this->GetLastPosition());

}




bool IWnd_stc::DoLoadFile(const wxString& file,int)
{
	StringBuffer<char> buff;
	buff.load(wx2str(file));

	SetText(str2wx(buff));
	EmptyUndoBuffer();
	SetSavePoint();

	return true;
}

bool IWnd_stc::DoSaveFile(const wxString& file,int)
{

	wxString text=GetValue();

	StringBuffer<char> buff;
	buff=wx2str(text);

	buff.save(wx2str(file));
	SetSavePoint();

	if(func) func();
	return true;

}

/*
const char* CppWordlist1 =
    "asm auto bool break case catch char class const const_cast "
    "continue default delete do double dynamic_cast else enum explicit "
    "export extern false float for friend goto if inline int long "
    "mutable namespace new operator private protected public register "
    "reinterpret_cast return short signed sizeof static static_cast "
    "struct switch template this throw true try typedef typeid "
    "typename union unsigned using virtual void volatile wchar_t "
    "while";

const char* CppWordlist2 =
    "";
*/

// C++
const char* CppWordlist1 =
    "asm auto bool break case catch char class const const_cast "
    "continue default delete do double dynamic_cast else enum explicit "
    "export extern false float for friend goto if inline int long "
    "mutable namespace new operator private protected public register "
    "reinterpret_cast return short signed sizeof static static_cast "
    "struct switch template this throw true try typedef typeid "
    "typename union unsigned using virtual void volatile wchar_t "
    "while";
const char* CppWordlist2 =
    "file";
const char* CppWordlist3 =
    "a addindex addtogroup anchor arg attention author b brief bug c "
    "class code date def defgroup deprecated dontinclude e em endcode "
    "endhtmlonly endif endlatexonly endlink endverbatim enum example "
    "exception f$ f[ f] file fn hideinitializer htmlinclude "
    "htmlonly if image include ingroup internal invariant interface "
    "latexonly li line link mainpage name namespace nosubgrouping note "
    "overload p page par param post pre ref relates remarks return "
    "retval sa section see showinitializer since skip skipline struct "
    "subsection test throw todo typedef union until var verbatim "
    "verbinclude version warning weakgroup $ @ \"\" & < > # { }";


class LangInfoCpp : public StcLangInfo
{
public:
	typedef StcLangInfo basetype;

	LangInfoCpp():basetype(wxT("C++"),wxT("*.c;*.cc;*.cpp;*.cxx;*.cs;*.h;*.hh;*.hpp;*.hxx;*.sma;"),wxSTC_LEX_CPP)
	{
		mapStyles[wxSTC_C_DEFAULT].set(StcManager::STYLE_DEFAULT);
		mapStyles[wxSTC_C_COMMENT].set(StcManager::STYLE_COMMENT);
		mapStyles[wxSTC_C_COMMENTLINE].set(StcManager::STYLE_COMMENT_LINE);
		mapStyles[wxSTC_C_COMMENTDOC].set(StcManager::STYLE_COMMENT_DOC);
		mapStyles[wxSTC_C_COMMENTLINEDOC].set(StcManager::STYLE_COMMENT_DOC);

		mapStyles[wxSTC_C_NUMBER].set(StcManager::STYLE_NUMBER);
		mapStyles[wxSTC_C_WORD].set(StcManager::STYLE_KEYWORD1,CppWordlist1);
		mapStyles[wxSTC_C_STRING].set(StcManager::STYLE_STRING);
		mapStyles[wxSTC_C_CHARACTER].set(StcManager::STYLE_CHARACTER);
		mapStyles[wxSTC_C_UUID].set(StcManager::STYLE_UUID);
		mapStyles[wxSTC_C_PREPROCESSOR].set(StcManager::STYLE_PREPROCESSOR);
		mapStyles[wxSTC_C_OPERATOR].set(StcManager::STYLE_OPERATOR);
		mapStyles[wxSTC_C_IDENTIFIER].set(StcManager::STYLE_IDENTIFIER);
		mapStyles[wxSTC_C_WORD2].set(StcManager::STYLE_KEYWORD2,CppWordlist2);
		mapStyles[wxSTC_C_COMMENTDOCKEYWORD].set(StcManager::STYLE_KEYWORD3,CppWordlist3);

	}

};


// EWSL
const char* EwslWordlist1 =
    "break break2 case try catch throw class "
    "continue continue2 default do def else explicit "
    "false for for_each function in if implicit "
    "global local judge "
    "return "
    "self switch this meta throw true try "
	"while "
	"@ "
	;

const char* EwslWordlist2 =
    "math io os coroutine logger "
    "integer double string complex boolean table nil "
	"array array_integer array_double array_complex array_variant "
	"pack unpack pcall map reduce  "
	"eval exec load_var save_var load_txt save_txt "
	;

const char* EwslWordlist3 =
    "a addindex addtogroup anchor arg attention author b brief bug c "
    "class code date def defgroup deprecated dontinclude e em endcode "
    "endhtmlonly endif endlatexonly endlink endverbatim enum example "
    "exception f$ f[ f] file fn hideinitializer htmlinclude "
    "htmlonly if image include ingroup internal invariant interface "
    "latexonly li line link mainpage name namespace nosubgrouping note "
    "overload p page par param post pre ref relates remarks return "
    "retval sa section see showinitializer since skip skipline struct "
    "subsection test throw todo typedef union until var verbatim "
    "verbinclude version warning weakgroup $ @ \"\" & < > # { }"
	;

class LangInfoEwsl : public StcLangInfo
{
public:
	typedef StcLangInfo basetype;

	LangInfoEwsl():basetype(wxT("EWSL"),wxT("*.ewsl;"),wxSTC_LEX_CPP)
	{
		mapStyles[wxSTC_C_DEFAULT].set(StcManager::STYLE_DEFAULT);
		mapStyles[wxSTC_C_COMMENT].set(StcManager::STYLE_COMMENT);
		mapStyles[wxSTC_C_COMMENTLINE].set(StcManager::STYLE_COMMENT_LINE);
		mapStyles[wxSTC_C_COMMENTDOC].set(StcManager::STYLE_COMMENT_DOC);
		mapStyles[wxSTC_C_COMMENTLINEDOC].set(StcManager::STYLE_COMMENT_DOC);

		mapStyles[wxSTC_C_NUMBER].set(StcManager::STYLE_NUMBER);
		mapStyles[wxSTC_C_WORD].set(StcManager::STYLE_KEYWORD1,EwslWordlist1);
		mapStyles[wxSTC_C_STRING].set(StcManager::STYLE_STRING);
		mapStyles[wxSTC_C_CHARACTER].set(StcManager::STYLE_CHARACTER);
		mapStyles[wxSTC_C_UUID].set(StcManager::STYLE_UUID);
		mapStyles[wxSTC_C_PREPROCESSOR].set(StcManager::STYLE_PREPROCESSOR);
		mapStyles[wxSTC_C_OPERATOR].set(StcManager::STYLE_OPERATOR);
		mapStyles[wxSTC_C_IDENTIFIER].set(StcManager::STYLE_IDENTIFIER);
		mapStyles[wxSTC_C_WORD2].set(StcManager::STYLE_KEYWORD2,EwslWordlist2);
		mapStyles[wxSTC_C_COMMENTDOCKEYWORD].set(StcManager::STYLE_KEYWORD3,EwslWordlist3);

	}

};

// Python
const char* PythonWordlist1 =
    "and assert break class continue def del elif else except exec "
    "finally for from global if import in is lambda None not or pass "
    "print raise return try while yield";
const char* PythonWordlist2 =
    "ACCELERATORS ALT AUTO3STATE AUTOCHECKBOX AUTORADIOBUTTON BEGIN "
    "BITMAP BLOCK BUTTON CAPTION CHARACTERISTICS CHECKBOX CLASS "
    "COMBOBOX CONTROL CTEXT CURSOR DEFPUSHBUTTON DIALOG DIALOGEX "
    "DISCARDABLE EDITTEXT END EXSTYLE FONT GROUPBOX ICON LANGUAGE "
    "LISTBOX LTEXT MENU MENUEX MENUITEM MESSAGETABLE POPUP PUSHBUTTON "
    "RADIOBUTTON RCDATA RTEXT SCROLLBAR SEPARATOR SHIFT STATE3 "
    "STRINGTABLE STYLE TEXTINCLUDE VALUE VERSION VERSIONINFO VIRTKEY";

class LangInfoPython : public StcLangInfo
{
public:
	typedef StcLangInfo basetype;

	LangInfoPython():basetype("Python","*.py;*.pyw;",wxSTC_LEX_PYTHON)
	{
		mapStyles[wxSTC_P_DEFAULT].set(StcManager::STYLE_DEFAULT);
		mapStyles[wxSTC_P_COMMENTLINE].set(StcManager::STYLE_COMMENT_LINE);
		mapStyles[wxSTC_P_NUMBER].set(StcManager::STYLE_NUMBER);
		mapStyles[wxSTC_P_STRING].set(StcManager::STYLE_STRING);
		mapStyles[wxSTC_P_WORD].set(StcManager::STYLE_KEYWORD1,PythonWordlist1);
		mapStyles[wxSTC_P_WORD2].set(StcManager::STYLE_KEYWORD2,PythonWordlist2);
	}

};


const char* LuaWordlist1 =
    ""
    ;
const char* LuaWordlist2 =
    "";

const char* LuaWordlist3 =
    "";

class LangInfoLua : public StcLangInfo
{
public:
	typedef StcLangInfo basetype;

	LangInfoLua():basetype("LUA","*.lua;",wxSTC_LEX_LUA)
	{
		mapStyles[wxSTC_LUA_DEFAULT].set(StcManager::STYLE_DEFAULT);
		mapStyles[wxSTC_LUA_COMMENT].set(StcManager::STYLE_COMMENT_LINE);
		mapStyles[wxSTC_LUA_COMMENTLINE].set(StcManager::STYLE_COMMENT_LINE);
		mapStyles[wxSTC_LUA_COMMENTDOC].set(StcManager::STYLE_COMMENT_MULTI);
		mapStyles[wxSTC_LUA_NUMBER].set(StcManager::STYLE_NUMBER);
		mapStyles[wxSTC_LUA_STRING].set(StcManager::STYLE_STRING);
		mapStyles[wxSTC_LUA_WORD].set(StcManager::STYLE_KEYWORD1,LuaWordlist1);
		mapStyles[wxSTC_LUA_WORD2].set(StcManager::STYLE_KEYWORD2,LuaWordlist2);
	}
};

const char* SqlWordlist1 =
    "select from where insert into update create truncate alter inner full outter join on "
	"delete table key index column primary forign loaddata database add drop duplicate"
	"column group by order desc asc limit case when then union else between and or not null is like default "
	"charset comment duplicate set LOAD DATA INFILE FIELDS TERMINATED enclosed lines"

    ;
const char* SqlWordlist2 =
	"";

const char* SqlWordlist3 =
    "";

class LangInfoSql : public StcLangInfo
{
public:
	typedef StcLangInfo basetype;

	LangInfoSql():basetype("SQL","*.sql;",wxSTC_LEX_SQL)
	{
		mapStyles[wxSTC_SQL_DEFAULT].set(StcManager::STYLE_DEFAULT);
		mapStyles[wxSTC_SQL_COMMENT].set(StcManager::STYLE_COMMENT_LINE);
		mapStyles[wxSTC_SQL_COMMENTLINE].set(StcManager::STYLE_COMMENT_LINE);
		mapStyles[wxSTC_SQL_COMMENTDOC].set(StcManager::STYLE_COMMENT_MULTI);
		mapStyles[wxSTC_SQL_NUMBER].set(StcManager::STYLE_NUMBER);
		mapStyles[wxSTC_SQL_STRING].set(StcManager::STYLE_STRING);
		mapStyles[wxSTC_SQL_WORD].set(StcManager::STYLE_KEYWORD1,SqlWordlist1);
		mapStyles[wxSTC_SQL_WORD2].set(StcManager::STYLE_KEYWORD2,SqlWordlist2);
	}
};

StcManager::StcManager()
{
	langs.push_back(StcLangInfo());
	langs.push_back(LangInfoCpp());
	langs.push_back(LangInfoEwsl());
	langs.push_back(LangInfoLua());
	langs.push_back(LangInfoPython());
	langs.push_back(LangInfoSql());


	style.resize(STYLE_MAX);
	style[STYLE_DEFAULT]=StcStyleInfo("Default","BLACK", "WHITE", "",0,0);
	style[STYLE_COMMENT]=StcStyleInfo("Comment","FOREST GREEN", "WHITE","",0,0);
	style[STYLE_COMMENT_LINE]=StcStyleInfo("Comment","FOREST GREEN", "WHITE","",0,0);
	style[STYLE_COMMENT_MULTI]=StcStyleInfo("Comment","FOREST GREEN", "WHITE","",0,0);
	style[STYLE_COMMENT_DOC]=StcStyleInfo("Comment","SIENNA", "WHITE","",0,0);

	style[STYLE_KEYWORD1]=StcStyleInfo("Keyword1","BLUE", "WHITE","",0,0);
	style[STYLE_KEYWORD2]=StcStyleInfo("Keyword2","BLUE","WHITE","",0,0);
	style[STYLE_KEYWORD3]=StcStyleInfo("Keyword3","BLUE", "WHITE","",0,0);
	style[STYLE_STRING]=StcStyleInfo("String","BROWN", "WHITE", "",0,0);
	style[STYLE_NUMBER]=StcStyleInfo("Number","SIENNA", "WHITE","",0,0);
	style[STYLE_PREPROCESSOR]=StcStyleInfo("Preprocessor","GREY", "WHITE","",0,0);
	style[STYLE_UUID]=StcStyleInfo("UUID","ORCHID", "WHITE", "",0,0);
	style[STYLE_ERROR]=StcStyleInfo("Error","RED", "WHITE", "",0,0);
	style[STYLE_UNDEFINED]=StcStyleInfo("Undefined","ORANGE", "WHITE", "",0,0);
	style[STYLE_SCRIPT]=StcStyleInfo("Script","DARK GREY", "WHITE", "",0,0);
	style[STYLE_VALUE]=StcStyleInfo("Value","ORCHID", "WHITE", "",0,0);
	style[STYLE_IDENTIFIER]=StcStyleInfo("Identifier","BLACK", "WHITE", "",0,0);
	style[STYLE_OPERATOR]=StcStyleInfo("Operator","BLACK", "WHITE", "",0,0);
	style[STYLE_PARAMETER]=StcStyleInfo("Parameter","VIOLET", "WHITE", "",0,0);
	style[STYLE_LABEL]=StcStyleInfo("Label","VIOLET", "WHITE", "",0,0);
	style[STYLE_COMMAND]=StcStyleInfo("Label","BLUE", "WHITE", "",0,0);
	style[STYLE_DELIMITER]=StcStyleInfo("Delimiter","ORANGE", "WHITE", "",0,0);


}


StcStyleInfo::StcStyleInfo():name(""),foreground("RED"){}

StcStyleInfo::StcStyleInfo(const String& n,const String& f,const String& b,const String& k,int ft,int le)
	:name(n),foreground(f),background(b),fontname(k)
{
	fontstyle=ft;
	lettercase=le;
}



template<>
class ValidatorW<IWnd_stc> : public ValidatorW<wxTextEntryBase>
{
public:
	LitePtrT<IWnd_stc> pWindow;
	ValidatorW(IWnd_stc* w):ValidatorW<wxTextEntryBase>(w),pWindow(w)
	{
		pWindow->m_pVald.reset(this);
	}
};


template<>
class WndInfoT<IWnd_stc> : public WndInfoBaseT<IWnd_stc> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_stc>(s)
	{

	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<int32_t>* p)
	{
		return CreateValidatorBaseT(w,p);
	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<String>* p)
	{
		return CreateValidatorBaseT(w,p);
	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyT<double>* p)
	{
		return CreateValidatorBaseT(w,p);
	}

	Validator* CreateValidator(wxWindow* w,EvtBase* p)
	{
		return CreateValidatorVoidT(w,p);
	}

	Validator* CreateValidator(wxWindow* w)
	{
		return new ValidatorW<IWnd_stc>((IWnd_stc*)w);
	}
};

template<>
void WndInfoManger_Register<IWnd_stc>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_stc> info(name);
	imgr.Register(&info);
}

StcManager& StcManager::current()
{
	static StcManager gInstance;
	return gInstance;
}




ICmdProcTextEntryStc::ICmdProcTextEntryStc(IWnd_stc& t):basetype(t),Target(t),data(IDat_search::current()){}

bool ICmdProcTextEntryStc::TestSelection()
{
	long p1,p2(0);
	Target.GetSelection(&p1,&p2);
	if(p1==p2)
	{
		return false;
	}

	Target.SetTargetStart(p1);
	Target.SetTargetEnd(Target.GetLastPosition());

	if(Target.SearchInTarget(str2wx(data.text_old))==-1)
	{
		return false;
	}

	if(Target.GetTargetStart()==p1&&Target.GetTargetEnd()==p2)
	{
		Target.GotoPos(p1);
		return true;	
	}

	return false;
}

bool ICmdProcTextEntryStc::DoFind(ICmdParam& cmd)
{		
	long p1=Target.GetInsertionPoint();

	Target.SetTargetStart(p1);
	Target.SetTargetEnd(Target.GetLastPosition());

	if(Target.SearchInTarget(str2wx(data.text_old))==-1)
	{

		if(cmd.param2==1) return false;
		
		Target.SetTargetStart(0);
		Target.SetTargetEnd(Target.GetLastPosition());

		if(Target.SearchInTarget(str2wx(data.text_old))!=-1)
		{
			int rt=Wrapper::MsgsDialog(_hT("no more results, restart?"),IDefs::BTN_YES|IDefs::BTN_NO|IDefs::ICON_QUESTION);
			if(rt!=IDefs::BTN_YES)
			{
				return false;
			}
		}
		else
		{
			Wrapper::MsgsDialog(_hT("no more results!"),IDefs::ICON_MESSAGE|IDefs::BTN_OK);
			return false;
		}
	}

	Target.GotoPos(Target.GetTargetStart());
	Target.SetSelection(Target.GetTargetStart(),Target.GetTargetEnd());
	Target.SetInsertionPoint(Target.GetTargetEnd());

	return true;
}


bool ICmdProcTextEntryStc::DoReplace(ICmdParam& cmd)
{	
	if(!TestSelection())
	{
		return DoFind(cmd);
	}

	int pos=Target.GetTargetStart();	
	int len=Target.ReplaceTarget(str2wx(data.text_new));

	Target.SetInsertionPoint(pos+len);
	Target.SetSelection(pos+len,pos+len);

	DoFind(cmd);
	return true;
}

bool ICmdProcTextEntryStc::DoReplaceAll(ICmdParam& cmd)
{
	cmd.param2=1;

	Target.SetInsertionPoint(0);
	if(!TestSelection() && !DoFind(cmd))
	{
		Wrapper::MsgsDialog(_hT("no results"),0);
		return false;
	}

	int n=0;
	Target.BeginUndoAction();
	try
	{
		while(DoReplace(cmd)) n++;
	}
	catch(std::exception&)
	{
			
	}
	Target.EndUndoAction();
	Wrapper::MsgsDialog(String::Format(_hT("%d results replaced"),n),0);
	return true;
}


bool ICmdProcTextEntryStc::DoExecId(ICmdParam& cmd)
{
	switch(cmd.param1)
	{
	case CP_DIRTY:
		Target.MarkDirty();
		break;
	case CP_FIND:
		if(data.text_old=="") return false;
		Target.SetSearchFlags(data.flags.val());
		return DoFind(cmd);
	case CP_REPLACE:
		if(data.text_old=="") return false;
		Target.SetSearchFlags(data.flags.val());
		return DoReplace(cmd);
	case CP_REPLACEALL:
		if(data.text_old=="") return false;
		Target.SetSearchFlags(data.flags.val());
		return DoReplaceAll(cmd);
	default:
		return basetype::DoExecId(cmd);
	}
	return true;
}

bool ICmdProcTextEntryStc::DoTestId(ICmdParam& cmd)
{
	switch(cmd.param1)
	{
	case CP_DIRTY:
		return Target.IsModified();
	default:
		return basetype::DoTestId(cmd);
	}
}


EW_LEAVE
