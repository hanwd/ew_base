#ifndef __H_EW_BASIC_CONSOLE__
#define __H_EW_BASIC_CONSOLE__

#include "ewa_base/config.h"
#include "ewa_base/basic/formater.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE Console
{
public:
	enum
	{
		COLOR_R=1<<2,
		COLOR_G=1<<1,
		COLOR_B=1<<0,
		COLOR_S=1<<3,
		COLOR_D=COLOR_R|COLOR_G|COLOR_B,
	};

	static void SetColor(int color);


	STRING_FORMATER_FORMAT_FUNCS_SB(static void Write,ColoredWrite(fb);)
	STRING_FORMATER_FORMAT_FUNCS_SB(static void WriteLine,ColoredWriteLine(fb);)

	static void ColoredWrite(const String& s,int color=-1);
	static void ColoredWriteLine(const String& s,int color=-1);

	static String ReadLine();

	static void Pause();

};

EW_LEAVE
#endif
