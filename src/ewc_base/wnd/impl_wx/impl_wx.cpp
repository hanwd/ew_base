
#include "ewc_base/wnd/impl_wx/window.h"
#include "ewc_base/evt/evt_command.h"
#include "ewc_base/app/app.h"
#include <wx/notebook.h>
#include <wx/artprov.h>

EW_ENTER

int wxEVT_USER_DROPDOWN_MENU = wxEVT_USER_FIRST;

int32_t WndPropertyEx::flag_nb_dir() const
{
	int d=nb_dir();
	if(d==IDefs::NB_BOTTOM) return wxNB_BOTTOM;
	if(d==IDefs::NB_LEFT) return wxNB_LEFT;
	if(d==IDefs::NB_RIGHT) return wxNB_RIGHT;
	return wxNB_TOP;
}

String wx2str(const wxString& v)
{
	const wchar_t* p=v.c_str();
	return p;
}

bool wx2buf(const wxString& v, StringBuffer<char>& sb)
{
	const wchar_t* p=v.c_str();
	return IConv::unicode_to_utf8(sb, p, std::char_traits<wchar_t>::length(p));
}

wxString str2wx(const String& v)
{
	return IConv::to_wide(v).c_str();
}

template<>
String WxImpl<String>::get(const wxVariant& v)
{
	return v.GetString().c_str().AsWChar();
}

template<>
void WxImpl<String>::set(wxVariant& v,const String& s)
{
	v=IConv::to_wide(s).c_str();
}

template<>
void WxImpl<long>::set(wxVariant& v,const long& s)
{
	v=s;
}


wxString WndPropertyEx::label1() const
{
	wxString v=str2wx(m_aSProperties["label"]);
	if(v=="")
	{
		v=str2wx(Language::current().Translate(m_aSProperties["name"]));
	}		
	return v;
}

wxString WndPropertyEx::label2() const
{
	if(flags().get(IDefs::IWND_WITHCOLON))
	{
		return str2wx(m_aSProperties["label"]+":");
	}
	else
	{
		return str2wx(m_aSProperties["label"]);
	}
}


ICmdProcTextEntry::ICmdProcTextEntry(wxTextEntryBase& t):Target(t){}

bool ICmdProcTextEntry::DoExecId(ICmdParam& cmd)
{
	switch(cmd.param1)
	{
	case CP_CUT:
		Target.Cut();
		break;
	case CP_COPY:
		Target.Copy();
		break;
	case CP_PASTE:
		Target.Paste();
		break;
	case CP_REDO:
		Target.Redo();
		break;
	case CP_UNDO:
		Target.Undo();
		break;
	case CP_DELETE:
		Target.RemoveSelection();
		break;
	case CP_SELECTALL:
		Target.SelectAll();
	default:
		return basetype::DoExecId(cmd);
	}
	return true;
}

bool ICmdProcTextEntry::DoTestId(ICmdParam& cmd)
{
	switch(cmd.param1)
	{
	case CP_DELETE:
	case CP_CUT:
		return Target.CanCut();
	case CP_COPY:
		return Target.CanCopy();
	case CP_PASTE:
		return Target.CanPaste();
	case CP_REDO:
		return Target.CanRedo();
	case CP_UNDO:
		return Target.CanUndo();
	case CP_SELECTALL:
		return true;
	case CP_FILEEXT:
		cmd.extra1=_hT("Text Files")+"(*.txt)|*.txt";
		return true;
	case CP_FIND:
		return true;
	case CP_REPLACE:
	case CP_REPLACEALL:
		return Target.IsEditable();
	default:
		return basetype::DoTestId(cmd);
	}
}



void BitmapBundle::set(const wxBitmap& bmp,int w)
{
	if(!bmp.IsOk()) return;
	if(w<0||bmp.GetHeight()==w)
	{
		bmp_normal=bmp;
		flags.del(FLAG_SCALED);
	}
	else
	{
		bmp_normal=wxBitmap(bmp.ConvertToImage().Scale(w,w));
		flags.add(FLAG_SCALED);
	}

	update_disabled();
}

void BitmapBundle::update_disabled()
{
	wxImage image=bmp_normal.ConvertToImage();
	bmp_disabled=wxBitmap(image.ConvertToGreyscale().ConvertToDisabled(255));
}

bool BitmapBundle::update(const wxBitmap& bmp,int w)
{
	if(bmp_normal.IsOk()) return true;
	if(!bmp.IsOk()) return false;
	set(bmp,w);
	return true;
}

bool BitmapBundle::IsOk() const
{
	return bmp_normal.IsOk();
}



void BitmapBundle::Serialize(Serializer& ar)
{
	ar & flags;

	if(ar.is_reader() && flags.get(FLAG_SCALED))
	{
		bmp_normal=wxNullBitmap;
		bmp_disabled=wxNullBitmap;
		return;
	}
		
	if(ar.is_writer() && flags.get(FLAG_SCALED))
	{
		return;
	}

	ar & bmp_normal;
	ar & bmp_disabled;

}

EW_LEAVE
