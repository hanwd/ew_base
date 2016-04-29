#include "ewc_base/wnd/impl_wx/window.h"
#include <wx/process.h>
#include <wx/utils.h>

#include "ewc_base/app/app.h"
#include "ewc_base/wnd/wnd_model.h"
#include "ewc_base/wnd/wnd_manager.h"

#include "ewc_base/plugin/plugin.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewa_base/scripting/executor_proxy.h"

#include <msvc/wx/setup.h>

#pragma comment(lib,"ewa_base.lib")
#pragma comment(lib,"ewc_base.lib")

int ew_main()
{
	using namespace ew;

	mp_check_leak(1);

	System::SetLogFile("ew.log");
	System::LogTrace("----  process enter   -------");

	WndManager& wm(WndManager::current());

// ע����
	wm.RegisterT<PluginMainWindow>();
	wm.RegisterT<PluginBasic>();
	wm.RegisterT<PluginSearch>();
	wm.RegisterT<PluginWorkspace>();
	wm.RegisterT<PluginStatusBar>();
	wm.RegisterT<PluginHistoryFiles>();
	wm.RegisterT<PluginMessage>();
	wm.RegisterT<PluginOption>();

	wm.RegisterT<PluginTextEditor>();
	wm.RegisterT<PluginEwslEditor>();
	wm.RegisterT<PluginWebView>();

	wm.SetName(_kT("ew6_ui_framework"));

	// ���Դ��ļ����������ļ�
	if(!wm.app.conf.Load("config/default.conf"))
	{
		// �����ļ������ڣ���ʼ��������Ϣ
		wm.app.conf.SetValue<String>("/basic/language","Chinese");
	}

	// ִ����Դ���ýű�����Ҫ��ͼ�ꡣ
	wm.LoadScript("scripting/ui/res_manager.script");

	// �������еĲ��
	if(wm.LoadPlugins())
	{
		// ִ�нű������Ʋ˵��������������ÿ�ݼ��ȡ�
		wm.LoadScript("scripting/ui/evt_manager.script");
		
		// ����������Ϣ
		wm.LoadConfig();

		// ��ʾ�����ڽ���
		wm.StartFrame();

		// ������Ϣѭ��
		wm.app.MainLoop();

		// ����������Ϣ
		wm.SaveConfig();
	
		// ����������Ϣ���ļ�
		wm.app.conf.Save("config/default.conf");
	}

	ThreadManager::current().close();
	ThreadManager::current().wait();

	System::LogTrace("----  process leave   -------");
	return 0;
}

using namespace ew;



int main(int,char**)
{
	return ew_main();
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return ew_main();
}

