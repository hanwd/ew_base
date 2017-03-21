
#include "ewc_base/app/app.h"
#include "ewc_base/app/res_manager.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/wnd/impl_wx/window.h"
#include "ewc_base/app/wrapper.h"

#include "wx/clipbrd.h"
#include "wx/filedlg.h"


EW_ENTER

int Wrapper::wxid_to_btn(int id)
{
	switch (id)
	{
	case wxID_OK:
		return IDefs::BTN_OK;
	case wxID_YES:
		return IDefs::BTN_YES;
	case wxID_NO:
		return IDefs::BTN_NO;
	case wxID_CANCEL:
		return IDefs::BTN_CANCEL;
	case wxID_APPLY:
		return IDefs::BTN_APPLY;
	case wxID_CLOSE:
		return IDefs::BTN_CLOSE;
	case wxID_RESET:
		return IDefs::BTN_RESET;
	default:
		return IDefs::BTN_CANCEL;
	}
}

int Wrapper::btn_to_wxid(int id)
{
	switch (id)
	{
	case IDefs::BTN_OK:
		return wxID_OK;
	case IDefs::BTN_YES:
		return wxID_YES;
	case IDefs::BTN_NO:
		return wxID_NO;
	case IDefs::BTN_CANCEL:
		return wxID_CANCEL;
	case IDefs::BTN_APPLY:
		return wxID_APPLY;
	case IDefs::BTN_CLOSE:
		return wxID_CLOSE;
	case IDefs::BTN_RESET:
		return wxID_RESET;
	default:
		return wxID_CANCEL;
	}
}

 int Wrapper::FileDialog(String& file,int type,const String& title,const String& exts)
{
	arr_1t<String> files;
	int ret=FileDialog(files,type,title,exts);
	if(!files.empty())
	{
		file=files[0];
	}
	return ret;
}

int Wrapper::FileDialog(arr_1t<String>& files,int type,const String& title,const String& exts)
{

	wxString wxtitle(str2wx(title));
	if(wxtitle=="")
	{
		wxtitle=str2wx(_hT("Please select a file"));
	}

	int flag=(type&IDefs::FD_SAVE)==0?wxFD_OPEN:wxFD_SAVE;
	if(type&IDefs::FD_MULTI) flag|=wxFD_MULTIPLE;
	if(type&IDefs::FD_MUST_EXIST) flag|=wxFD_FILE_MUST_EXIST;

	wxString wxexts;
	if(!exts.empty())
	{
		if(exts.find('|')<0)
		{
			wxexts+=str2wx(exts+"|"+exts+"|"+_hT("AllFiles")+"(*.*)|*.*");
		}
		else
		{
			wxexts+=str2wx(exts+"|"+_hT("AllFiles")+"(*.*)|*.*");
		}
	}


	wxWindow* ptopwindow=WndModel::current().GetWindow();

	::wxFileDialog dlg(ptopwindow,wxtitle,"","",wxexts,flag);
	if(files.size()==1)
	{
		dlg.SetPath(str2wx(files[0]));
	}

	if(dlg.ShowModal()==wxID_OK)
	{
		files.clear();
		wxArrayString wxfiles;
		dlg.GetPaths(wxfiles);
		for(size_t i=0;i<wxfiles.size();i++)
		{
			files.append(wx2str(wxfiles[i]));
		}
		return IDefs::BTN_OK;
	}
	else
	{
		return IDefs::BTN_CANCEL;
	}
}

int Wrapper::LogsDialog(arr_1t<LogRecord>& records,int level,const String& title)
{
	StringBuilder cont;

	for(size_t i=0;i<records.size();i++)
	{
		cont<<records[i].m_sMessage<<"\r\n";
	}

	int type=IDefs::BTN_OK;
	if(level>=LOGLEVEL_ERROR) type|=IDefs::ICON_ERROR;
	else if(level==LOGLEVEL_WARNING) type|=IDefs::ICON_WARNING;
	else type|=IDefs::ICON_INFO;

	return MsgsDialog(cont,type,title);
}

int Wrapper::MsgsDialog(const String& cont,int type,const String& title)
{
	wxWindow* ptopwindow=WndModel::current().GetWindow();
	int fg=wxCENTER;
	if(type&IDefs::BTN_OK) fg|=wxOK;
	if(type&IDefs::BTN_YES) fg|=wxYES;
	if(type&IDefs::BTN_NO) fg|=wxNO;
	if(type&IDefs::BTN_CANCEL) fg|=wxCANCEL;
	if(type&IDefs::BTN_CLOSE) fg|=wxCLOSE;
	if(type&IDefs::BTN_APPLY) fg|=wxAPPLY;
	if(type&IDefs::BTN_RESET) fg|=wxRESET;

	if(type&IDefs::ICON_ERROR) fg|=wxICON_ERROR;
	else if(type&IDefs::ICON_WARNING) fg|=wxICON_WARNING;
	else if(type&IDefs::ICON_QUESTION) fg|=wxICON_QUESTION;
	else fg|=wxICON_INFORMATION;

	String dlg_title(title);
	if(dlg_title.empty())
	{
		dlg_title=WndManager::current().GetTitle();
	}

	::wxMessageDialog dlg(NULL,str2wx(cont),str2wx(dlg_title),fg);
	int ret=dlg.ShowModal();

	return wxid_to_btn(ret);
}

bool Wrapper::SetClipboardText(const String& s)
{
	wxClipboard* pcb=wxClipboard::Get();
	if(!pcb||!pcb->Open()) return false;
	pcb->SetData(new wxTextDataObject(str2wx(s)));
	pcb->Close();
	return true;
}

bool Wrapper::GetClipboardText(String& s)
{
	wxClipboard* pcb=wxClipboard::Get();
	if(!pcb||!pcb->Open()) return false;
	wxTextDataObject data;
	pcb->GetData(data);
	s=wx2str(data.GetText());
	pcb->Close();
	return true;
}

EW_LEAVE
