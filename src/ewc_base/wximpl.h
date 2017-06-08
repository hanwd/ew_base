#ifndef __H_EW_WXIMPL__
#define __H_EW_WXIMPL__

#include "ewc_base/config.h"

EW_ENTER

class GuiImplWx
{
public:

	static bool Init(int argc, char** argv);
	static void Fini();

	static int MainLoop();
	static void ExitLoop();

};


EW_LEAVE

#endif
