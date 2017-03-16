
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
	wm.RegisterT<PluginSearch>();
	wm.RegisterT<PluginWorkspace>();
	wm.RegisterT<PluginStatusBar>();
	wm.RegisterT<PluginHistoryFiles>();
	wm.RegisterT<PluginMessage>();
	wm.RegisterT<PluginOption>();

	wm.RegisterT<PluginTextEditor>();
	wm.RegisterT<PluginEwslEditor>();
	wm.RegisterT<PluginWebView>();

	wm.SetName(_kT("ew_ui_framework"));


	// ���Դ��ļ����������ļ�
	if (!wm.app.conf.Load("res:/config/default.conf"))
	{
		// �����ļ������ڣ���ʼ��������Ϣ
		wm.app.conf.SetValue<String>("/basic/language","Chinese");
	}

	String sLanguage;
	if (wm.app.conf.GetValue<String>("/basic/language", sLanguage))
	{
		wm.lang.SetLanguage(sLanguage);
	}


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
	
		// ����������Ϣ���ļ�
		wm.app.conf.Save("res:/config/default.conf");
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

