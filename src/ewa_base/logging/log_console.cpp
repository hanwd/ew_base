#include "ewa_base/threading/thread_mutex.h"
#include "ewa_base/basic/console.h"
#include "ewa_base/logging/logrecord.h"
#include "ewa_base/logging/logtarget.h"

#include <iostream>

#ifdef EW_WINDOWS
#include <windows.h>
#endif

EW_ENTER

//\033[22;30m - black
//\033[22;31m - red
//\033[22;32m - green
//\033[22;33m - brown
//\033[22;34m - blue
//\033[22;35m - magenta
//\033[22;36m - cyan
//\033[22;37m - gray
//\033[01;30m - dark gray
//\033[01;31m - light red
//\033[01;32m - light green
//\033[01;33m - yellow
//\033[01;34m - light blue
//\033[01;35m - light magenta
//\033[01;36m - light cyan
//\033[01;37m - white
//\033[0m

LogConsole::LogConsole()
{
	aColors.resize(LOGLEVEL_MAX+1);
	flags.add(FLAG_COLORED);
	for(size_t i=0; i<LOGLEVEL_MAX; i++)
	{
		aColors[i]=Console::COLOR_D;
	}
	aColors[LOGLEVEL_ERROR]=Console::COLOR_S|Console::COLOR_R;
	aColors[LOGLEVEL_LITE_ERROR]=Console::COLOR_S|Console::COLOR_R;
	aColors[LOGLEVEL_WARNING]=Console::COLOR_S|Console::COLOR_G;
}

void LogConsole::Handle(const LogRecord& o)
{
	if (!flags.get(FLAG_SHOWALL) && o.m_nId != 0)
	{
		return;
	}

	if(o.m_nLevel==LOGLEVEL_COMMAND)
	{
		if(o.m_sMessage=="clc")
		{
#ifdef EW_WINDOWS
			system("cls");
#endif
		}
		return;
	}

	String omsg=Format(o);

	typedef void (*fun)(const String&,int);
	
	fun write=o.m_nLevel == LOGLEVEL_PRINT?&Console::ColoredWrite:&Console::ColoredWriteLine;
	int color=flags.get(FLAG_COLORED)?aColors[o.m_nLevel]:-1;
	write(omsg,color);

}

EW_LEAVE
