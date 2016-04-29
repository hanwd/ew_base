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

// 注册插件
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

	// 尝试从文件载入配置文件
	if(!wm.app.conf.Load("config/default.conf"))
	{
		// 配置文件不存在，初始化配置信息
		wm.app.conf.SetValue<String>("/basic/language","Chinese");
	}

	// 执行资源配置脚本，主要是图标。
	wm.LoadScript("scripting/ui/res_manager.script");

	// 载入所有的插件
	if(wm.LoadPlugins())
	{
		// 执行脚本，定制菜单，工具条，设置快捷键等。
		wm.LoadScript("scripting/ui/evt_manager.script");
		
		// 载入配置信息
		wm.LoadConfig();

		// 显示主窗口界面
		wm.StartFrame();

		// 进入消息循环
		wm.app.MainLoop();

		// 保存配置信息
		wm.SaveConfig();
	
		// 保存配置信息到文件
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

