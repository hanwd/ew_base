#include "ewc_base/wnd/wnd_glcontext.h"
#include "gl/gl.h"

EW_ENTER

void GLContext::CheckError()
{
	GLenum errLast = GL_NO_ERROR;

	Logger& logger(this_logger());

	for (;;)
	{
		GLenum err = glGetError();
		if (err == GL_NO_ERROR)
			return;

		if (err == errLast)
		{
			logger.LogError("OpenGL error state couldn't be reset.");
			return;
		}

		errLast = err;
		logger.LogError("OpenGL error %d", err);
	}
}

GLContext::GLContext()
{
	m_hWnd = NULL;
	m_hDC = NULL;
	m_hGL = NULL;
}

GLContext::~GLContext()
{
	if (!m_hDC) return;
	wglMakeCurrent(NULL, NULL);
	::ReleaseDC((HWND)m_hWnd, (HDC)m_hDC);
	wglDeleteContext((HGLRC)m_hGL);
}


void GLContext::SetCurrent(wxWindow* w)
{
	if (!m_hDC)
	{
		// default neutral pixel format
		PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),  // size
			1,                              // version
			PFD_SUPPORT_OPENGL |
			PFD_DRAW_TO_WINDOW |
			PFD_DOUBLEBUFFER,               // use double-buffering by default
			PFD_TYPE_RGBA,                  // default pixel type
			0,                              // preferred color depth (don't care)
			0, 0, 0, 0, 0, 0,               // color bits and shift bits (ignored)
			0, 0,                           // alpha bits and shift (ignored)
			0,                              // accumulation total bits
			0, 0, 0, 0,                     // accumulator RGBA bits (not used)
			16,                             // depth buffer
			0,                              // no stencil buffer
			0,                              // no auxiliary buffers
			PFD_MAIN_PLANE,                 // main layer
			0,                              // reserved
			0, 0, 0,                        // no layer, visible, damage masks
		};

		m_hWnd = w->GetHWND();
		m_hDC = ::GetDC((HWND)m_hWnd);

		int pixelFormat = ChoosePixelFormat((HDC)m_hDC, &pfd);
		if (!pixelFormat)
		{
			return;
		}

		if (!::SetPixelFormat((HDC)m_hDC, pixelFormat, &pfd))
		{
			return;
		}

		m_hGL = ::wglCreateContext((HDC)m_hDC);
		if (!m_hGL) CheckError();
	}

	::wglMakeCurrent((HDC)m_hDC, (HGLRC)m_hGL);
}

void GLContext::SwapBuffers()
{
	::SwapBuffers((HDC)m_hDC);
}

EW_LEAVE

