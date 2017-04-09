#ifndef __H_EW_WND_GLCONTEXT__
#define __H_EW_WND_GLCONTEXT__

#include "ewc_base/config.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE GLContext : public NonCopyable
{
public:

	GLContext();
	~GLContext();

	void SetCurrent(wxWindow* w);
	void SwapBuffers();

	static void CheckError();

protected:

	void* m_hWnd;
	void* m_hDC;
	void* m_hGL;

};

EW_LEAVE

#endif
