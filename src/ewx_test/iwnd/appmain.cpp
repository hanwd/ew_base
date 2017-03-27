
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
	

// ע����
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

	// ���ò����������ļ�
	wm.InitConfig("res:/config/default.conf");

	// ִ����Դ���ýű�����Ҫ��ͼ�ꡣ
	wm.LoadScript("res:/scripting/ui/res_manager.ewsl");	

	// �������еĲ��
	if(wm.LoadPlugins())
	{
	
		// ִ�нű������Ʋ˵��������������ÿ�ݼ��ȡ�
		wm.LoadScript("res:/scripting/ui/evt_manager.ewsl");
		
		// ����������Ϣ
		wm.LoadConfig();


		// ��ʾ�����ڽ���
		wm.StartFrame();

		// ������Ϣѭ��
		wm.app.MainLoop();

		// ����������Ϣ
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
