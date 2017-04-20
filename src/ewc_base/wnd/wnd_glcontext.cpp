#include "ewc_base/wnd/wnd_glcontext.h"
#include "ewc_base/data/data_node.h"
#include "ewc_base/data/data_model.h"
#include "gl/gl.h"

EW_ENTER

DVec2i::DVec2i(const wxSize& sz) :vec2i(sz.x, sz.y){}
DVec2i::DVec2i(const wxPoint& pt) :vec2i(pt.x, pt.y){}
DVec2i::DVec2i(const vec2i& o) :vec2i(o){}

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

class GLBufferExtraData : public Object
{
public:
	wxImage image;
};

GLBuffer::GLBuffer()
{
	m_pExtraData.reset(new GLBufferExtraData);
	flags.clr(-1);
}


void GLBuffer::ReadPixels()
{
	wxImage& image(static_cast<GLBufferExtraData&>(*m_pExtraData).image);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	vpos[0] = viewport[0];
	vpos[1] = viewport[1];
	
	vec2i sz(std::max(GLint(16),viewport[2]), std::max(GLint(16),viewport[3]));

	if (size != sz)
	{
		try
		{
			depth.resize(sz[0], sz[1]);
			alpha.resize(sz[0], sz[1]);
			color.resize(sz[0], sz[1]);
			image.Create(sz[0], sz[1], (unsigned char*)color.data(),true);
		}
		catch (...)
		{
			size[0] = size[1] = 0;
			image = wxImage();
			throw;
		}

		size = sz;
	}

	glGetDoublev(GL_PROJECTION_MATRIX, projmatrix.data());
	glGetDoublev(GL_MODELVIEW_MATRIX, viewmatrix.data());

	glReadBuffer(GL_BACK);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	if (flags.get(FLAG_NEED_COLOR))
	{
		glReadPixels(viewport[0], viewport[1], viewport[2], viewport[3], GL_RGB, GL_UNSIGNED_BYTE, color.data());
	}

	if (flags.get(FLAG_NEED_ALPHA))
	{
		glReadPixels(viewport[0], viewport[1], viewport[2], viewport[3], GL_ALPHA, GL_UNSIGNED_BYTE, alpha.data());
	}

	if (flags.get(FLAG_NEED_DEPTH))
	{
		glReadPixels(viewport[0], viewport[1], viewport[2], viewport[3], GL_DEPTH_COMPONENT, GL_FLOAT, depth.data());
	}

}

void GLBuffer::Blit(wxDC& dc)
{
	wxImage& image(static_cast<GLBufferExtraData&>(*m_pExtraData).image);
	dc.DrawBitmap(wxBitmap(image.Mirror(false)), vpos[0], vpos[1]);
}


GLDC::GLDC()
{
	m_aLights.push_back(GL_LIGHT1);
	m_aLights.push_back(GL_LIGHT2);
	m_aLights.push_back(GL_LIGHT3);
	m_aLights.push_back(GL_LIGHT4);
	m_aLights.push_back(GL_LIGHT0);

	Color = &_Color1;
}


void GLDC::Light(DLight& light, bool f)
{

	if (light.light_index<0 && f)
	{
		if (m_aLights.empty())
		{
			return;
		}
		light.light_index = m_aLights.back();
		m_aLights.pop_back();

		glLightfv(light.light_index, GL_AMBIENT, light.v4ambient.data());
		glLightfv(light.light_index, GL_DIFFUSE, light.v4deffuse.data());
		glLightfv(light.light_index, GL_SPECULAR, light.v4specular.data());
		glLightfv(light.light_index, GL_POSITION, light.v4position.data());
		glEnable(light.light_index);
	}
	if (light.light_index>0 && !f)
	{
		m_aLights.push_back(light.light_index);
		glDisable(light.light_index);
		light.light_index = -1;
	}
}

void GLDC::Reshape(const DVec2i& s_, const DVec2i& p_)
{

	vec2i s(s_);
	if (s[0]<16) s[0] = 16;
	if (s[1]<16) s[1] = 16;

	sz.b3bbox.set_x(p_[0],p_[0]+s[0]);
	sz.b3bbox.set_y(p_[1],p_[1]+s[1]);
	sz.b3bbox.set_z(-100, 100);

	if (m_b3BBox == sz.b3bbox) return;

	m_b3BBox = sz.b3bbox;

	glViewport(p_[0], p_[1], s[0],s[1]);			// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	::glOrtho(m_b3BBox.lo[0], m_b3BBox.hi[0], m_b3BBox.lo[1], m_b3BBox.hi[1], m_b3BBox.lo[2], m_b3BBox.hi[2] );

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();

	glDrawBuffer(GL_BACK);
}

void GLDC::Clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLDC::Clear(const DColor& c)
{
	m_v4BgColor = c;
	glClearColor(c.r, c.g, c.b, c.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLDC::LoadIdentity()
{
	::glLoadIdentity();
}

void GLDC::_Color1(const DColor& c)
{
	::glColor4ubv((GLubyte*)&c);
}

void GLDC::_Color2(const DColor&)
{

}



void GLDC::PushMatrix()
{
	::glPushMatrix();
}

void GLDC::PopMatrix()
{
	::glPopMatrix();
}

void GLDC::Blit(wxDC& dc)
{
	m_aBuffer[0].ReadPixels();
	m_aBuffer[0].Blit(dc);
}

void GLDC::RenderNode(DataNode* node)
{
	if (!node) return;
	if (m_nMode == RENDER_SELECT)
	{
		int val = m_aNodes.find2(node);
		_Color1(*(DColor*)&val);
	}

	node->DoRender(*this);
}


void GLDC::Mode(int mode)
{
	m_nMode = mode;

	sz.b3bbox = m_b3BBox;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDrawBuffer(GL_BACK);


	if (mode == RENDER_SELECT)
	{
		Color = &_Color2;
		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);
		m_aNodes.clear();
		m_aNodes.insert(NULL);

		
	}
	else
	{
		Color = &_Color1;
		glDisable(GL_LIGHTING);
	}

}

int GLDC::Mode()
{
	return m_nMode;
}

void GLDC::RenderModel(DataModel* model)
{
	Clear();	

	DataNode* node = model ? model->GetRootNode() : NULL;
	if (node)
	{
		Mode(GLDC::RENDER_SET_REALSIZE);
		node->DoRender(*this);
		Mode(GLDC::RENDER_SOLID);
		node->DoRender(*this);
	}
	SwapBuffers();
}

void GLDC::RenderSelect(DataModel* model)
{
	Clear();	
	Mode(GLDC::RENDER_SELECT);

	DataNode* node = model ? model->GetRootNode() : NULL;
	if (node)
	{
		node->DoRender(*this);
	}
	
	//m_aBuffer[1].ReadPixels();
	//Mode(GLDC::RENDER_SOLID);

}

DataNode* GLDC::HitTest(unsigned x, unsigned y)
{
	unsigned v(0);
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &v);
	if (v < 1 || v >= m_aNodes.size())
	{
		return NULL;
	}
	return m_aNodes.get(v);
}


EW_LEAVE

