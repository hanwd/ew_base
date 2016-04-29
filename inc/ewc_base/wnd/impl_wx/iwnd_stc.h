#ifndef __UI_IWND_STC__
#define __UI_IWND_STC__

#include "ewc_base/wnd/impl_wx/window.h"
#include "ewc_base/app/data_defs.h"
#include "wx/stc/stc.h"


EW_ENTER


class DLLIMPEXP_EWC_BASE StcLangInfo
{
public:

    String name;
    String filepattern;
    int lexer;
    BitFlags folds;

	class StyleWords
	{
	public:
		void set(int i,const String& w=""){id=i;words=w;}

		int id;
		String words;
	};

	typedef bst_map<int,StyleWords> map_type;

	map_type mapStyles;

	StcLangInfo(const String& n,const String& p,int lex):name(n),filepattern(p),lexer(lex){}
	StcLangInfo():name("default"){lexer=wxSTC_LEX_NULL;}

};

class DLLIMPEXP_EWC_BASE StcStyleInfo 
{
public:

	StcStyleInfo();
	StcStyleInfo(const String& n,const String& f,const String& b,const String& k="",int ft=0,int le=0);

	String name;
	String foreground;
	String background;
	String fontname;
	BitFlags fontstyle;
	int lettercase;
};


class DLLIMPEXP_EWC_BASE StcManager
{
public:

	arr_1t<StcLangInfo> langs;
	arr_1t<StcStyleInfo> style;

	static StcManager& current();

	enum
	{
		STYLE_BOLD		=1<<0,
		STYLE_ITALIC	=1<<1,
		STYLE_UNDERL	=1<<2,
		STYLE_HIDDEN	=1<<3,
		FOLD_COMMENT	=1<<4,
		FOLD_COMPACT	=1<<5,
		FOLD_PREPROC	=1<<6,
		FOLD_HTML		=1<<7,
		FOLD_HTMLPREP	=1<<8,
		FOLD_COMMENTPY	=1<<9,
		FOLD_QUOTESPY	=1<<10,
	};

	enum
	{
		STYLE_DEFAULT,
		STYLE_KEYWORD1,
		STYLE_KEYWORD2,
		STYLE_KEYWORD3,
		STYLE_KEYWORD4,
		STYLE_STRING,
		STYLE_NUMBER,
		STYLE_COMMENT,
		STYLE_COMMENT_LINE,
		STYLE_COMMENT_MULTI,
		STYLE_COMMENT_DOC,
		STYLE_CHARACTER,
		STYLE_CHARACTER_EOL,
		STYLE_DELIMITER,
		STYLE_OPERATOR,
		STYLE_IDENTIFIER,
		STYLE_PARAMETER,
		STYLE_UUID,
		STYLE_PREPROCESSOR,
		STYLE_ERROR,
		STYLE_UNDEFINED,
		STYLE_SCRIPT,
		STYLE_VALUE,
		STYLE_LABEL,
		STYLE_COMMAND,
		STYLE_MAX,
	};

	enum
	{
		LINE_NR_ID = 0,
		DIVIDER_ID = 1,
		FOLDING_ID = 2,
		FOLDING_MARGIN = 16
	};

protected:
	StcManager();
};


class LanguageInfo;

class DLLIMPEXP_EWC_BASE StcStyleFlag
{
public:

	int nsize;
	int wdtab;
	int nlang;
	String sface;
	BitFlags flags;

	StcStyleFlag();
};

class DLLIMPEXP_EWC_BASE IWnd_stc : public wxStyledTextCtrl
{
public:
	typedef wxStyledTextCtrl basetype;
	ValidatorHolderT<Validator> m_pVald;

	enum
	{
		FLAG_SYNTAX		=1<<0,
		FLAG_FOLD		=1<<1,
		FLAG_INDENT		=1<<2,
		FLAG_READONLY	=1<<3,
		FLAG_OVERTYPE	=1<<4,
		FLAG_WRAPMODE	=1<<5,
		FLAG_SHOWEOL	=1<<6,
		FLAG_INDENTGUIDE=1<<7,
		FLAG_LINENUM	=1<<8,
		FLAG_LONGLINE	=1<<9,
		FLAG_WHITESPACE	=1<<10,
		FLAG_DEFAULT=FLAG_SYNTAX|FLAG_INDENTGUIDE|FLAG_LINENUM|FLAG_LONGLINE|FLAG_WRAPMODE,

	};



	IWnd_stc(wxWindow* p,const WndPropertyEx& h,int w=0);
	~IWnd_stc();

	bool DoLoadFile(const wxString& file,int);
	bool DoSaveFile(const wxString& file,int);
	
	static StcStyleFlag ms_param;
	StcStyleFlag tempp;

	BitFlags style;

	void UpdateStyle(const StcStyleFlag& flag);
	void UpdateStyle(const String& lang);
	void UpdateStyle();
	
	Functor<void()> func;

	bool SetFontSize(int fs);

	int GetIndentWidth();

protected:

	StcStyleFlag param;

	void _DoUpdateStyle();
	void _Init();

	void SetPropertyEx(const String& id,bool f);

	void OnMarginClick (wxStyledTextEvent &evt);
	void OnCharAdded (wxStyledTextEvent &evt);
	void OnKeyEvent(wxKeyEvent& evt);
	void OnStyledTextChanged(wxStyledTextEvent& evt);

	void OnMouseWheel(wxMouseEvent& evt);
	void OnFocus(wxFocusEvent& evt);

	void UpdateMarginLineNumWidth(bool flag=false);

	int m_nLineNumWidth;

	virtual void OnDocumentChanged();

	DECLARE_EVENT_TABLE();
};


class DLLIMPEXP_EWC_BASE ICmdProcTextEntryStc : public ICmdProcTextEntry
{
public:
	typedef ICmdProcTextEntry basetype;

	IWnd_stc& Target;
	IDat_search& data;

	ICmdProcTextEntryStc(IWnd_stc& t);

	bool TestSelection();
	bool DoFind(ICmdParam& cmd);
	bool DoReplace(ICmdParam& cmd);
	bool DoReplaceAll(ICmdParam& cmd);
	bool DoExecId(ICmdParam& cmd);
	bool DoTestId(ICmdParam& cmd);

};



EW_LEAVE
#endif
