
#include "ewc_base/app/app.h"
#include "ewc_base/wnd/wnd_manager.h"
#include "ewc_base/plugin/plugin.h"
#include "ewc_base/plugin/plugin_manager.h"

#pragma comment(lib,"ewa_base.lib")
#pragma comment(lib,"ewc_base.lib")


int ew_main()
{
	using namespace ew;

	mp_check_leak(1);

	System::SetLogFile("ew.log",true);
	System::LogTrace("----  process enter   -------");

	ObjectInfo::Invoke(InvokeParam::TYPE_INIT);	


	WndManager& wm(WndManager::current());
	

// 注册插件
	wm.RegisterT<PluginMainWindow>();
	wm.RegisterT<PluginBasic>();
	wm.RegisterT<PluginMenu>();
	wm.RegisterT<PluginToolBar>();
	wm.RegisterT<PluginRibbon>();
	wm.RegisterT<PluginSearch>();
	wm.RegisterT<PluginWorkspace>();
	wm.RegisterT<PluginStatusBar>();
	wm.RegisterT<PluginHistoryFiles>();
	wm.RegisterT<PluginMessage>();
	wm.RegisterT<PluginOption>();
	wm.RegisterT<PluginExpression>();

	wm.RegisterT<PluginTextEditor>();
	wm.RegisterT<PluginEwslEditor>();
	wm.RegisterT<PluginWebView>();

	wm.SetName(_kT("ew_ui_framework"));

	// 设置并读入配置文件
	wm.InitConfig("res:/config/default.conf");

	// 执行资源配置脚本，主要是图标。
	wm.LoadScript("res:/scripting/ui/res_manager.ewsl");	

	// 载入所有的插件
	if(wm.LoadPlugins())
	{
	
		// 执行脚本，定制菜单，工具条，设置快捷键等。
		wm.LoadScript("res:/scripting/ui/evt_manager.ewsl");
		
		// 载入配置信息
		wm.LoadConfig();


		// 显示主窗口界面
		wm.StartFrame();

		// 进入消息循环
		wm.app.MainLoop();

		// 保存配置信息
		wm.SaveConfig();
	
	}

	ObjectInfo::Invoke(InvokeParam::TYPE_FINI);

	System::LogTrace("----  process leave   -------");
	return 0;

}

int main(int,char**)
{
	return ew_main();
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return ew_main();
}
