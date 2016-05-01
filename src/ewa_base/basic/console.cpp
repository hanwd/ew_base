#include "ewa_base/basic/console.h"
#include "ewa_base/basic/string.h"
#include "ewa_base/basic/lockguard.h"
#include "ewa_base/basic/atomic.h"

#include <string>

#ifdef EW_WINDOWS
#include <windows.h>
#endif

#include <cstdio>
#include <iostream>

EW_ENTER

int g_cConsoleColor=Console::COLOR_D;
AtomicSpin g_tSpinConsole;

void ConsoleDoSetColor(int color)
{
	g_cConsoleColor=color;

#ifdef EW_WINDOWS
	HANDLE hCon=GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon,color);
#else
	switch(color)
	{
	case 0:
		::printf("\033[22;30m");
		break;
	case Console::COLOR_R:
		::printf("\033[01;31m");
		break;
	case Console::COLOR_R|Console::COLOR_S:
		::printf("\033[22;31m");
		break;
	case Console::COLOR_G:
		::printf("\033[01;32m");
		break;
	case Console::COLOR_G|Console::COLOR_S:
		::printf("\033[22;32m");
		break;
	case Console::COLOR_B:
		::printf("\033[01;34m");
		break;
	case Console::COLOR_B|Console::COLOR_S:
		::printf("\033[22;34m");
		break;
	case Console::COLOR_D:
		::printf("\033[0m");
		break;
	default:
		::printf("\033[0m");
		break;
	};

#endif
}


void Console::SetColor(int color)
{
	g_tSpinConsole.lock();
	ConsoleDoSetColor(color);
	g_tSpinConsole.unlock();
}


//void Console::Write(const String& s)
//{
//	g_tSpinConsole.lock();
//	std::cout<<s.c_str();
//	g_tSpinConsole.unlock();
//}
//
//void Console::WriteLine(const String& s)
//{
//	g_tSpinConsole.lock();
//	std::cout<<s.c_str()<<std::endl;
//	g_tSpinConsole.unlock();
//}


void Console::Write(const String& s,int color)
{
	g_tSpinConsole.lock();

	if(color<0)
	{
		std::cout<<s.c_str();
	}
	else
	{
		int oldcr=g_cConsoleColor;
		ConsoleDoSetColor(color);
		std::cout<<s.c_str();
		ConsoleDoSetColor(oldcr);
	}

	g_tSpinConsole.unlock();
}

void Console::WriteLine(const String& s,int color)
{
	g_tSpinConsole.lock();
	if(color<0)
	{
		std::cout<<s.c_str()<<std::endl;
	}
	else
	{
		int oldcr=g_cConsoleColor;
		ConsoleDoSetColor(color);
		std::cout<<s.c_str()<<std::endl;
		ConsoleDoSetColor(oldcr);
	}
	g_tSpinConsole.unlock();
}

String Console::ReadLine()
{
	static const int LINEBUF_SIZE=1024*8;
	char buf[LINEBUF_SIZE];
	buf[0]=0;

	Console::Write(">> ");
	std::cin.getline(buf,LINEBUF_SIZE,'\n');

	return buf;
}


void Console::Pause()
{
#ifdef EW_WINDOWS
	system("pause");
#else
	Write("Press ENTER to continue...");
	std::cin.get();
#endif

}


EW_LEAVE