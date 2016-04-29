#include "ewc_base/plugin/plugin_mainwindow.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/mvc/mvc_book.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/mvc/mvc_book.h"

#include "ewc_base/wnd/impl_wx/window.h"
#include "wx/aui/auibook.h"

EW_ENTER

PluginMainWindow::PluginMainWindow(WndManager& w):basetype(w,"Plugin.MainWindow")
{
	
}

bool PluginMainWindow::OnAttach()
{

	if(!wm.model.Create())
	{
		return false;
	}

	wxWindow* pwin=wm.model.GetWindow();
	if(!pwin)
	{
		System::LogMessage("PluginManWindow::OnAttach: WndModel::GetWindow() return NULL");
		return false;
	}

	wxWindow* book=wm.book.CreateBook(pwin);
	if(!book)
	{
		System::LogMessage("PluginManWindow::OnAttach: MvcBook::CreateBook() return NULL");
		return false;
	}

	book->SetName("Centerpane");

	wm.model.OnChildWindow(book,IDefs::WND_ATTACH);
	
	return true;
}

IMPLEMENT_IPLUGIN(PluginMainWindow)

EW_LEAVE
