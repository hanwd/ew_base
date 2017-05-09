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
			8, 0, 8, 0, 8, 0,               // color bits and shift bits (ignored)
			8, 0,                           // alpha bits and shift (ignored)
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

		int pixelFormat = ::ChoosePixelFormat((HDC)m_hDC, &pfd);
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
	::glDrawBuffer(GL_BACK);
}

void GLContext::SwapBuffers()
{
	::glFlush();
	::SwapBuffers((HDC)m_hDC);
}

class GLBufferExtraData : public ObjectData
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
		glLightfv(light.light_index, GL_DIFFUSE, light.v4diffuse.data());
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

	bi.b3bbox.set_x(p_[0],p_[0]+s[0]);
	bi.b3bbox.set_y(p_[1],p_[1]+s[1]);

	double dz = 3.0*std::max(std::max(s[0], s[1]),100);
	bi.b3bbox.set_z(-dz, dz);

	if (m_b3BBox == bi.b3bbox) return;

	::glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	::glEnable(GL_COLOR_MATERIAL);
	::glEnable(GL_NORMALIZE);
	::glDisable(GL_BLEND);
	::glEnable(GL_DEPTH_TEST);
	::glDepthFunc(GL_LEQUAL);

	m_b3BBox = bi.b3bbox;

	glViewport(p_[0], p_[1], s[0],s[1]);			// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	::glOrtho(m_b3BBox.lo[0], m_b3BBox.hi[0], m_b3BBox.lo[1], m_b3BBox.hi[1], m_b3BBox.lo[2], m_b3BBox.hi[2] );

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();

}

void GLDC::Clear()
{
	if (m_nMode == RENDER_SELECT)
	{
		glClearColor(0, 0, 0, 0);
	}
	else
	{
		glClearColor(m_v4BgColor.r, m_v4BgColor.g, m_v4BgColor.b, m_v4BgColor.a);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLDC::Clear(const DColor& c)
{
	m_v4BgColor = c;
	Clear();
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

void GLDC::MultMatrix(const mat4d& m4)
{
	::glMultMatrixd(m4.data());
}


void GLDC::SaveBuffer(const String& id)
{
	m_mapBuffers[id].ReadPixels();
}



void GLDC::LoadBuffer(const String& id, wxDC& dc)
{
	bst_map<String, GLBuffer>::iterator it = m_mapBuffers.find(id);
	if (it == m_mapBuffers.end())
	{
		return;
	}
	(*it).second.Blit(dc);
}


void GLDC::LineWidth(double d)
{
	::glLineWidth(d);
}

void GLDC::LineStyle(const DLineStyle& style)
{
	if (m_nMode == RENDER_SELECT)
	{
		if (style.ntype == DLineStyle::LINE_NONE)
		{
			LineWidth(0.0);
			::glLineStipple(1, 0x0000);
			::glEnable(GL_LINE_STIPPLE);
		}
		else
		{
			LineWidth(style.nsize + 1.5);
			::glDisable(GL_LINE_STIPPLE);
		}
	}
	else
	{
		if (style.color.a != 0)
		{
			Color(style.color);
		}

		LineWidth(style.nsize);
		switch (style.ntype)
		{
		case DLineStyle::LINE_NONE:
			::glLineStipple(1, 0x0000);
			::glEnable(GL_LINE_STIPPLE);
			break;
		case DLineStyle::LINE_DOT1:
			::glLineStipple(1, 0xAAAA);
			::glEnable(GL_LINE_STIPPLE);
			break;
		case DLineStyle::LINE_DOT2:
			::glLineStipple(2, 0xAAAA);
			::glEnable(GL_LINE_STIPPLE);
			break;
		case DLineStyle::LINE_DASH1:
			::glLineStipple(1, 0x00FF);
			::glEnable(GL_LINE_STIPPLE);
			break;
		case DLineStyle::LINE_DASH2:
			::glLineStipple(2, 0x00FF);
			::glEnable(GL_LINE_STIPPLE);
			break;
		case DLineStyle::LINE_DASH3:
			::glLineStipple(1, 0x087F);
			::glEnable(GL_LINE_STIPPLE);
			break;
		case DLineStyle::LINE_DASH4:
			::glLineStipple(1, 0x227F);
			::glEnable(GL_LINE_STIPPLE);
			break;
		default:
			::glLineStipple(1, 0xFFFF);
			::glDisable(GL_LINE_STIPPLE);
			break;
		};
	}

}

void GLDC::Mode(int mode)
{
	m_nMode = mode;

	bi.b3bbox = m_b3BBox;
	bi.b3axis.load_min();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	

	if (mode == RENDER_SET_REALSIZE)
	{
		attrupdator.ResetIndex();
	}

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



void GLDC::RenderNode(DataNode* node)
{
	if (!node) return;
	if (m_nMode == RENDER_SELECT)
	{
		int val = m_aNodes.find2(node);
		DColor& color(*(DColor*)&val);

		//val = (val+31113) * 1314111;
		//color.a = 255;

		_Color1(color);
	}

	node->DoRender(*this);
}


void GLDC::RenderModel(DataModel* model)
{
	


	if (!model)
	{
		return;
	}

	attrupdator.SetManager(model->m_pAttributeManager.get());
	attrupdator.SetObject(NULL);
	attrupdator.Update("color.background", m_v4BgColor);

	Mode(GLDC::RENDER_CALC_MINSIZE);

	Clear();

	DataNode* node = model ? model->GetRootNode() : NULL;
	if (node)
	{

		Mode(GLDC::RENDER_SET_REALSIZE);
		node->DoRender(*this);
		Mode(GLDC::RENDER_SOLID);
		node->DoRender(*this);
		Mode(GLDC::RENDER_ALPHA);
		node->DoRender(*this);
		Mode(GLDC::RENDER_TEXT);
		node->DoRender(*this);
	}
	
}

void GLDC::RenderSelect(DataModel* model)
{
	Clear();

	if (!model)
	{
		return;
	}

	attrupdator.SetManager(model->m_pAttributeManager.get());

	Mode(GLDC::RENDER_SELECT);
	DataNode* node = model ? model->GetRootNode() : NULL;
	if (node)
	{
	
		node->DoRender(*this);
	}
}

DataNode* GLDC::HitTest(unsigned x, unsigned y)
{
	
	unsigned v[9];

	glReadPixels(x-1, y-1, 3, 3, GL_RGBA, GL_UNSIGNED_BYTE, v);

	DataNode* pnode = NULL;
	int depth = 0;

	for (size_t i = 0; i < 9; i++)
	{
		if (v[i] < 1 || v[i] >= m_aNodes.size())
		{
			continue;
		}
		DataNode* p=m_aNodes.get(v[i]);
		if (p->depth >= depth)
		{
			depth = p->depth;
			pnode = p;
		}
	}

	return pnode;

}


class GLFontDataImpl : public ObjectData
{
public:


	GLFontDataImpl()
	{
		hFont = NULL;
		hDC = NULL;

		BITMAPINFO *binf = (BITMAPINFO *)&hBitmapHeader;

		binf->bmiHeader.biSize = sizeof(binf->bmiHeader);	/**< 修改结构信息 */
		binf->bmiHeader.biWidth = 0;
		binf->bmiHeader.biHeight = 0;
		binf->bmiHeader.biPlanes = 1;
		binf->bmiHeader.biBitCount = 1;				 /**< 单色 */
		binf->bmiHeader.biCompression = BI_RGB;		/**< 颜色方式 */
		binf->bmiHeader.biSizeImage = 0;
		binf->bmiHeader.biXPelsPerMeter = 1;
		binf->bmiHeader.biYPelsPerMeter = 1;
		binf->bmiHeader.biClrUsed = 0;
		binf->bmiHeader.biClrImportant = 0;
	}

	GLFontDataImpl(const DFontStyle& font)
	{
		hFont = NULL;
		hDC = NULL;
		update(font);
	}

	~GLFontDataImpl()
	{
		reset();
	}

	void reset()
	{
		if (hFont)
		{
			::DeleteObject(hFont);
			hFont = NULL;
		}

		if (hDC)
		{
			::DeleteDC(hDC);
			hDC = NULL;
		}
	}

	void update(const DFontStyle& font)
	{

		//WINGDIAPI HFONT   WINAPI CreateFontA( __in int cHeight, 
		//									 __in int cWidth, 
		//									 __in int cEscapement, 
		//									 __in int cOrientation, 
		//									 __in int cWeight, 
		//									 __in DWORD bItalic,
		//									 __in DWORD bUnderline, 
		//									 __in DWORD bStrikeOut, 
		//									 __in DWORD iCharSet, 
		//									 __in DWORD iOutPrecision, 
		//									 __in DWORD iClipPrecision,
		//									 __in DWORD iQuality,
		//									 __in DWORD iPitchAndFamily, 
		//									 __in_opt LPCSTR pszFaceName);

		reset();

		bVertical=font.flags.get(DFontStyle::STYLE_VERTICAL);

		hFont = ::CreateFontW(
			font.nsize,
			0,											//字体宽度 
			bVertical ? 900 : 0,						//字体的旋转角度  
			0,											//字体底线的旋转角度  
			90,//font.nsize,							//字体的重量 
			font.flags.get(DFontStyle::STYLE_ITALIC),	//是否使用斜体
			font.flags.get(DFontStyle::STYLE_UNDERLINE),//是否使用下划线
			font.flags.get(DFontStyle::STYLE_STRIDE),	//是否使用删除线
			GB2312_CHARSET,								//设置字符集 
			OUT_TT_PRECIS,								//输出精度  
			CLIP_DEFAULT_PRECIS,						//裁剪精度 
			ANTIALIASED_QUALITY,						//输出质量 
			FF_DONTCARE | DEFAULT_PITCH,				//Family And Pitch 
			IConv::to_wide(font.sname).c_str());		//字体名称

		if (!hFont) return;
		hDC = ::CreateCompatibleDC(NULL);
		if (!hDC) return;
		::SelectObject((HDC)hDC, hFont);
	}


	HFONT hFont;
	HDC hDC;
	bool bVertical;

	struct
	{
		BITMAPINFOHEADER bih;
		RGBQUAD col[2];
	}hBitmapHeader;
};


GLStatusLocker::GLStatusLocker(int cap_) : cap(cap_)
{
	status = GL_TRUE == glIsEnabled(cap);
}

GLStatusLocker::GLStatusLocker(int cap_, bool enable_) : cap(cap_)
{
	status = GL_TRUE == glIsEnabled(cap);
	if (enable_) glEnable(cap);
	else glDisable(cap);
}

GLStatusLocker::~GLStatusLocker()
{
	if (status)
	{
		glEnable(cap);
	}
	else
	{
		glDisable(cap);
	}
}


class GLDC::TextData
{
public:

	TextData(const String& text, GLFontDataImpl* pdata)
	{
		if (pdata)
		{
			dc =pdata->hDC;
			bv = pdata->bVertical;
			bi = (BITMAPINFO *)&pdata->hBitmapHeader;

			bi->bmiHeader.biSize = sizeof(bi->bmiHeader);	/**< 修改结构信息 */
			bi->bmiHeader.biWidth = 0;
			bi->bmiHeader.biHeight = 0;
			bi->bmiHeader.biPlanes = 1;
			bi->bmiHeader.biBitCount = 1;				 /**< 单色 */
			bi->bmiHeader.biCompression = BI_RGB;		/**< 颜色方式 */
			bi->bmiHeader.biSizeImage = 0;
			bi->bmiHeader.biXPelsPerMeter = 1;
			bi->bmiHeader.biYPelsPerMeter = 1;
			bi->bmiHeader.biClrUsed = 0;
			bi->bmiHeader.biClrImportant = 0;

		}
		else
		{
			dc = NULL;
		}


		sb = IConv::to_wide(text);
		wchar_t *p0 = sb.c_str();
		wchar_t *p2 = p0 + sb.size();
		for (wchar_t* p1 = p0; p1 != p2;p1++)
		{
			if (*p1 == '\n')
			{
				*p1 = 0;
				if (p1>p0 && p1[-1] == '\r')
				{
					p1[-1] = 0;
					wc.push_back(std::make_pair(p0, p1 - p0-1));
				}
				else
				{
					wc.push_back(std::make_pair(p0, p1 - p0));
				}

				p0 = p1 + 1;
			}
		}
		wc.push_back(std::make_pair(p0, p2 - p0));

	}

	StringBuffer<wchar_t> sb;
	arr_1t<std::pair<wchar_t*,size_t> > wc;
	bool bv;

	HDC dc;
	BITMAPINFO *bi;
};

vec2i GLDC::GetExtend(const String& text)
{
	TextData tdata(text, (GLFontDataImpl*)m_pFontData.get());
	return DoGetExtend(tdata);
}

vec2i GLDC::DoGetExtend(const TextData& data)
{

	vec2i size;
	if (!data.dc) return size;

	size_t n = data.wc.size();
	for (size_t i = 0; i<n; i++)
	{
		SIZE sz;
		if (data.wc[i].second!=0)
		{
			::GetTextExtentPoint32W(data.dc, data.wc[i].first, data.wc[i].second, &sz);
		}
		else
		{
			::GetTextExtentPoint32W(data.dc, L".",1, &sz);
		}
		size[0] = std::max<int>(size[0], sz.cx);
		size[1] = std::max<int>(size[1], sz.cy);
	}
	size[1] *= n;

	if (data.bv)
	{
		std::swap(size[0], size[1]);
	}

	return size;
}


void GLDC::PrintText(const String& text, const vec3d& pos,const vec3d& shf, const vec3d& pxl)
{
	TextData tdata(text, (GLFontDataImpl*)m_pFontData.get());
	return DoPrintText(tdata,pos,shf,pxl);
}


 void GLDC::DoPrintText(const TextData& data,const vec3d& pos,const vec3d& shf, const vec3d& pxl)
{

	if (!data.dc) return;


	if(m_tFontStyle.color.a!=0)
	{
		GLDC::Color(m_tFontStyle.color);
	}
	
	vec2i size = DoGetExtend(data);

	HBITMAP hBitmap = ::CreateBitmap(size[0], size[1], 1, 1, NULL);
	HBITMAP hOldBmp = (HBITMAP)SelectObject(data.dc, hBitmap);
	BITMAP bm;

	::SetBkColor(data.dc, RGB(0, 0, 0));
	::SetTextColor(data.dc, RGB(255, 255, 255));
	::SetBkMode(data.dc, OPAQUE);

	if (data.bv)
	{
		int nw = size[0] / data.wc.size();
		for (size_t i = 0; i<data.wc.size(); i++)
		{
			::TextOutW(data.dc, nw*i, size[1], data.wc[i].first, data.wc[i].second);
		}
	}
	else
	{
		int nh = size[1] / data.wc.size();
		for (size_t i = 0; i<data.wc.size(); i++)
		{
			::TextOutW(data.dc, 0, nh*i, data.wc[i].first, data.wc[i].second);
		}
	}


	::GetObject(hBitmap, sizeof(bm), &bm);

	SIZE sz;
	sz.cx = (bm.bmWidth + 31) & (~31);
	sz.cy = bm.bmHeight;
	int bufsize = sz.cx * sz.cy / 8;

	data.bi->bmiHeader.biWidth = bm.bmWidth;
	data.bi->bmiHeader.biHeight = bm.bmHeight;
	data.bi->bmiHeader.biSizeImage = bufsize;

	m_aBitmapCachedData.reserve(bufsize);

	unsigned char* pBmpBits = m_aBitmapCachedData.data();
	memset(pBmpBits, 0, sizeof(unsigned char)*bufsize);

	::GetDIBits(data.dc, hBitmap, 0, bm.bmHeight, pBmpBits, data.bi, DIB_RGB_COLORS);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glRasterPos3d(pos[0], pos[1], pos[2]);

	int xorg = ::wxRound((0.5 - 0.5*shf[0])*size[0] - pxl[0]);
	int yorg = ::wxRound((0.5 - 0.5*shf[1])*size[1] - pxl[1]);



	glBitmap(sz.cx, sz.cy, xorg, yorg, 0.0, 0.0, pBmpBits);

	::SelectObject(data.dc, hOldBmp);
	if (hBitmap)::DeleteObject(hBitmap);

}

void GLDC::EnterGroup()
{
	glPushMatrix();
	glPushAttrib(GL_COLOR_BUFFER_BIT);
}

void GLDC::LeaveGroup()
{
	glPopAttrib();
	glPopMatrix();
}

void GLDC::UpdateAttribute(DataNodeSymbol* node)
{
	if (!node->flags.get(DataNode::FLAG_ATTRIBUTED))
	{
		attrupdator.SetObject(node->value.get());
		node->DoUpdateAttribute(*this);
		node->flags.add(DataNode::FLAG_ATTRIBUTED);
	}
}

void GLDC::SetFont(const DFontStyle& font)
{

	if(m_tFontStyle==font && m_pFontData)
	{
		return;
	}

	m_tFontStyle=font;
	m_pFontData.reset(new GLFontDataImpl(font));
}


void GLDC::Enable(int id, bool f)
{
	if (f)
	{
		::glEnable(id);
	}
	else
	{
		::glDisable(id);
	}
}

GLClipLocker::GLClipLocker(GLDC& dc_, const GLClipInfo& ci_) :dc(dc_), ci0(ci_)
{
	ci0 = dc.ClipPlane(ci0);
}

GLClipLocker::~GLClipLocker()
{
	ci0 = dc.ClipPlane(ci0);
}

GLClipInfo GLDC::ClipPlane(const GLClipInfo& ci_)
{
	GLClipInfo ci0(m_tClipInfo);m_tClipInfo = ci_;

	for (int i = 0; i < 4; i++)
	{
		::glClipPlane(GL_CLIP_PLANE0 + i, m_tClipInfo.plane[i].data());
		Enable(GL_CLIP_PLANE0 + i,m_tClipInfo.enable);
	}

	return ci0;
}

GLMatrixLocker::GLMatrixLocker(GLDC& dc_) :dc(dc_){ dc.PushMatrix(); }
GLMatrixLocker::GLMatrixLocker(GLDC& dc_, const mat4d& m4_) :dc(dc_){ dc.PushMatrix(); dc.MultMatrix(m4_); }
GLMatrixLocker::~GLMatrixLocker(){ dc.PopMatrix(); }

GLGroupLocker::GLGroupLocker(GLDC& dc_) :dc(dc_){ dc.EnterGroup(); }
GLGroupLocker::~GLGroupLocker(){ dc.LeaveGroup(); }

class gl_bitmap
{
public:
	int w, h;
	int bufsize;
	int xorg, yorg;
	int xmov, ymov;

	gl_bitmap()
	{
		w = h = 0;
		bits = NULL;
		xorg = yorg = xmov = ymov = 0;
	}

	unsigned char* ptr()
	{
		return bits;
	}

	~gl_bitmap()
	{
		delete[] bits;
	}

	void resize(int w_, int h_, int x_ = -1, int y_ = -1);

	void draw(const vec3d& pos);

	void settype(float w, int t);

	void setbit(int x, int y, bool f);

	void setall(bool f);

private:
	unsigned char* bits;

};

void gl_bitmap::resize(int w_, int h_, int x_, int y_)
{
	delete[] bits;
	w = w_;
	h = h_;
	bufsize = w*h / 8 + 1;
	xorg = x_ >= 0 ? x_ : w_ / 2;
	yorg = y_ >= 0 ? y_ : h_ / 2;
	bits = new unsigned char[bufsize];
	memset(bits, 0, bufsize);
}

void gl_bitmap::setall(bool f)
{
	int val = f ? -1 : 0;
	::memset(bits, val, bufsize);
}

void gl_bitmap::setbit(int x, int y, bool f)
{
	EW_ASSERT(x >= 0 && x<w&&y >= 0 && y<h);

	int p = y*w + x;
	int b = p / 8;
	int s = 7 - (p % 8);
	unsigned char &c(bits[b]);
	unsigned char m = 1 << s;

	if (f)
	{
		c = c | m;
	}
	else
	{
		c = c&~m;
	}


}

void gl_bitmap::settype(float w, int t)
{
	if (t == 0)
	{
		resize(8, 8);
		return;
	}

	if (w>7.0) w = 7.0;

	//int iw=::wxRound(fabs(w));
	//if(iw>7) iw=7;
	//if(iw<1) iw=1;

	int iw = w;
	if (iw>3)
	{
		resize(16, 16);
	}
	else
	{
		resize(8, 8);
	}
	int i1 = xorg - iw;
	int i2 = xorg + iw;

	for (int i = i1; i <= i2; i++)
	{
		for (int j = i1; j <= i2; j++)
		{
			setbit(i, j, true);
		}
	}

}

void gl_bitmap::draw(const vec3d& pos)
{
	if (!bits) return;
	::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	::glRasterPos3d(pos[0], pos[1], pos[2]);
	::glBitmap(w, h, xorg, yorg, xmov, ymov, bits);
}


EW_LEAVE

