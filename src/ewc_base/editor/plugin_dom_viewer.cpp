#include "ewc_base/plugin/plugin_plugineditor.h"
#include "ewc_base/editor/plugin_dom_viewer.h"
#include "ewc_base/plugin/plugin_manager.h"

#include "ewc_base/mvc/mvc_book.h"
#include "ewc_base/mvc/mvc_model.h"
#include "ewc_base/mvc/mvc_view.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/wnd/wnd_updator.h"
#include "ewc_base/wnd/wnd_glcontext.h"
#include "ewc_base/wnd/impl_wx/iwnd_bookbase.h"
#include "ewc_base/wnd/impl_wx/iwnd_modelview.h"
#include "ewc_base/evt/evt_option.h"
#include "ewc_base/data/data_node.h"
#include "ewc_base/data/data_model.h"

#include "ewa_base/domdata.h"

#include "gl/gl.h"

EW_ENTER

template<>
class DataNodeSymbolT<DFigure> : public DataNodeSymbol
{
public:

	DataNodeSymbolT(DataNode* n, DObject* p) :DataNodeSymbol(n, p)
	{

	}

	void DoRender(GLDC& dc)
	{
		for (size_t i = 0; i < subnodes.size(); i++)
		{
			GLMatrixLocker locker(dc);
			dc.RenderNode(subnodes[i]);
		}
	}
};


template<>
class DataNodeSymbolT<DObjectBox> : public DataNodeSymbol
{
public:
public:
	typedef DataNodeSymbol basetype;
	DataNodeSymbolT(DataNode* n, DObject* p) :basetype(n, p)
	{
		line_main.color.set(200,200,200);
	}

	box3d b3bbox0;
	box3d b3bbox1;
	box3d b3margin;
	GLClipInfo clinfo;

	DLineStyle line_main;

	vec3d v3pos,v3shf,v3size;

	void DoSetRealSize(const box3d& b3bbox)
	{
		b3bbox0=b3bbox1=b3bbox;
		b3bbox1.lo+=b3margin.lo;
		b3bbox1.hi-=b3margin.hi;
	}

	void DoSetRealSize(const vec3d& v3pos,const vec3d& v3shf,const vec3d& v3size)
	{
		b3bbox1.lo=b3bbox1.hi=v3pos;
		b3bbox1.lo-=0.5*v3size;
		b3bbox1.hi+=0.5*v3size;

		b3bbox1.lo[0]+=0.5*v3shf[0]*v3size[0];
		b3bbox1.hi[0]+=0.5*v3shf[0]*v3size[0];
		b3bbox1.lo[1]+=0.5*v3shf[1]*v3size[1];
		b3bbox1.hi[1]+=0.5*v3shf[1]*v3size[1];

		b3bbox0=b3bbox1;
		b3bbox0.lo-=b3margin.lo;
		b3bbox0.hi+=b3margin.hi;
	}


};


template<>
class DataNodeSymbolT<DText> : public DataNodeSymbolT<DObjectBox>
{
public:
	typedef DataNodeSymbolT<DObjectBox> basetype;

	DataPtrT<DText> value;

	DataNodeSymbolT(DataNode* n, DObject* p) :basetype(n, p)
	{
		b3margin.lo.set2(5,5);
		b3margin.hi.set2(5,5);
		value.cast_and_set(p);
	}

	DFontStyle font1;
	DColor color;

	void DoUpdateAttribute(GLDC& dc)
	{
		basetype::DoUpdateAttribute(dc);
		dc.attrupdator.Update("text", font1);
	}

	void DoRender(GLDC& dc)
	{
		if (!value) return;

		dc.UpdateAttribute(this);

		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{
			dc.attrupdator.SetObject(value.get());

			vec3d v3shf=value->m_v3Shf;
			vec3d v3pos;
			vec3d v3size;

			v3pos[0] =0.5*((value->m_v3Pos[0]+1.0) * dc.bi.b3bbox.hi[0] + (1.0 - value->m_v3Pos[0])*dc.bi.b3bbox.lo[0]) + value->m_v3Pxl[0];
			v3pos[1] =0.5*((value->m_v3Pos[1]+1.0) * dc.bi.b3bbox.hi[1] + (1.0 - value->m_v3Pos[1])*dc.bi.b3bbox.lo[1]) + value->m_v3Pxl[1];

			dc.SetFont(font1);
			vec2i v2size=dc.GetExtend(value->m_sText);
			v3size.set2(v2size[0],v2size[1]);

			DoSetRealSize(v3pos,v3shf,v3size);

			return;
		}


		if (dc.Mode() == GLDC::RENDER_TEXT||dc.Mode() == GLDC::RENDER_SELECT)
		{
			dc.SetFont(font1);
			dc.PrintText(value->m_sText,b3bbox1.center());
		}
	}

};

template<>
class DataNodeSymbolT<DCoord> : public DataNodeSymbolT<DObjectBox>
{
public:
public:
	typedef DataNodeSymbolT<DObjectBox> basetype;

	DataNodeSymbolT(DataNode* n, DObject* p) :basetype(n, p)
	{

	}

	box3d b3axis0,b3axis1;
};


template<>
class DataNodeSymbolT<DTable> : public DataNodeSymbol
{
public:
public:
	typedef DataNodeSymbol basetype;
	DataNodeSymbolT(DataNode* n, DObject* p) :basetype(n, p)
	{

	}

	box3d b3bbox0;

	void DoRender(GLDC& dc)
	{
		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{
			b3bbox0 = dc.bi.b3bbox;
		}

		if(subnodes.size()<1)
		{
			basetype::DoRender(dc);
			return;
		}

		size_t n=subnodes.size()-1;
		for (size_t i = 0; i < subnodes.size(); i++)
		{
			dc.bi.b3axis.lo[1]=(double(i)*b3bbox0.hi[1]+double(n-i)*b3bbox0.lo[1])/double(n);
			dc.bi.b3axis.hi[1]=(double(i+1)*b3bbox0.hi[1]+double(n-i-1)*b3bbox0.lo[1])/double(n);
			dc.RenderNode(subnodes[i]);
		}

	}
};

template<>
class DataNodeSymbolT<DTableRow> : public DataNodeSymbol
{
public:
public:
	typedef DataNodeSymbol basetype;
	DataNodeSymbolT(DataNode* n, DObject* p) :basetype(n, p)
	{

	}


	GLBBoxInfo bi0;

	void DoRender(GLDC& dc)
	{
		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{
			bi0.b3bbox = dc.bi.b3bbox;
		}

		if(subnodes.size()<1)
		{
			basetype::DoRender(dc);
			return;
		}

		GLBBoxInfo bi1=dc.bi;

		size_t n=subnodes.size();
		for (size_t i = 0; i < n; i++)
		{
			dc.bi=bi1;
			dc.bi.b3bbox.lo[0]=(double(i)*bi0.b3bbox.hi[0]+double(n-i)*bi0.b3bbox.lo[0])/double(n);
			dc.bi.b3bbox.hi[0]=(double(i+1)*bi0.b3bbox.hi[0]+double(n-i-1)*bi0.b3bbox.lo[0])/double(n);
			dc.RenderNode(subnodes[i]);
		}

	}
};

template<>
class DataNodeSymbolT<DTableCell> : public DataNodeSymbol
{
public:
public:
	typedef DataNodeSymbol basetype;
	DataNodeSymbolT(DataNode* n, DObject* p) :basetype(n, p)
	{

	}
};


template<>
class DataNodeSymbolT<DCoord2D> : public DataNodeSymbolT<DCoord>
{
public:

	typedef DataNodeSymbolT<DCoord> basetype;

	DataNodeSymbolT(DataNode* n, DObject* p) :basetype(n, p)
	{
		b3axis1.set_x(-100.0, +100.0);
		b3axis1.set_y(-100.0, +100.0);
		b3axis1.set_z(-1.0, +1.0);

		b3axis0 = b3axis1;

		b3margin.lo=70;
		b3margin.hi=45;
	}

	virtual DataPtrT<GLToolData> GetToolData();

	void DoUpdateAttribute(GLDC& dc)
	{
		basetype::DoUpdateAttribute(dc);
		dc.attrupdator.Update("line.bbox.coord", line_main);
	}

	void DoRender(GLDC& dc)
	{
		
		dc.UpdateAttribute(this);

		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{

			DoSetRealSize(dc.bi.b3bbox);
			
			dc.bi.b3axis = b3axis1;
			dc.bi.b3bbox = b3bbox1;

			clinfo.set(dc.bi.b3bbox);

			vec3d v3p(dc.bi.b3bbox.lo);
			vec3d v3w(dc.bi.b3bbox.width());

			dc.bi.m4data.LoadIdentity();
			dc.bi.m4data.Translate(v3p);
			dc.bi.m4data.Scale(v3w);
			dc.bi.m4data.Scale(1.0 / dc.bi.b3axis.width());
			dc.bi.m4data.Translate(-dc.bi.b3axis.lo);

			for (size_t i = 0; i < subnodes.size(); i++)
			{
				subnodes[i]->DoRender(dc);
			}

			return;
		}


		LockState<GLClipInfo> lock_ci(dc.ci,clinfo);

		if (dc.Mode() == GLDC::RENDER_SELECT)
		{

			glDepthMask(false);
			glBegin(GL_QUADS);
			glVertex2d(b3bbox0.lo[0], b3bbox0.lo[1]);
			glVertex2d(b3bbox0.hi[0], b3bbox0.lo[1]);
			glVertex2d(b3bbox0.hi[0], b3bbox0.hi[1]);
			glVertex2d(b3bbox0.lo[0], b3bbox0.hi[1]);
			glEnd();
			glDepthMask(true);
		}

		if (!b3bbox1.is_valid())
		{
			return;
		}
		
		if (dc.Mode() == GLDC::RENDER_SOLID || dc.Mode() == GLDC::RENDER_SELECT)
		{
			//dc.Color(DColor(255,0,0));	
			basetype::DoRender(dc);
		}
		
		if (dc.Mode() == GLDC::RENDER_TEXT)
		{

			basetype::DoRender(dc);

			dc.LineStyle(line_main);
			glBegin(GL_LINE_LOOP);
				glVertex2d(b3bbox1.lo[0], b3bbox1.lo[1]);
				glVertex2d(b3bbox1.hi[0], b3bbox1.lo[1]);
				glVertex2d(b3bbox1.hi[0], b3bbox1.hi[1]);
				glVertex2d(b3bbox1.lo[0], b3bbox1.hi[1]);
			glEnd();

		}

	}
};


class DLLIMPEXP_EWC_BASE GLToolDataCoord2d : public GLToolData
{
public:

	DataNodeSymbolT<DCoord2D>* pnode;

	GLToolDataCoord2d(DataNodeSymbolT<DCoord2D>* pnode_) :pnode(pnode_)
	{

	}


	box3d b3bbox2,b3axis2;

	virtual int OnDraging(GLTool& gt)
	{
		gt.flags.add(GLParam::BTN_IS_MOVED);

		if (gt.btn_id == 2)
		{
			vec2i dvec = gt.v2pos2 - gt.v2pos1;
			dvec[1] *= -1;

			double d1 = double(dvec[0])*b3axis2.x_width() / double(b3bbox2.x_width());
			double d2 = double(dvec[1])*b3axis2.y_width() / double(b3bbox2.y_width());

			pnode->b3axis1.lo[0] = b3axis2.lo[0] - d1;
			pnode->b3axis1.hi[0] = b3axis2.hi[0] - d1;
			pnode->b3axis1.lo[1] = b3axis2.lo[1] - d2;
			pnode->b3axis1.hi[1] = b3axis2.hi[1] - d2;

			return GLParam::FLAG_REFRESH;
		}
		else
		{
			return GLParam::FLAG_CACHING|GLParam::FLAG_REFRESH;
		}
	}

	int OnBtnUp(GLTool& gt)
	{
		if (gt.flags.get(GLParam::BTN_IS_DOWN))
		{
			if (gt.btn_id == 1 && gt.v2pos1[0]!=gt.v2pos2[0] && gt.v2pos1[1]!=gt.v2pos2[1])
			{

				gt.v2pos1[1] = gt.v2size[1] - gt.v2pos1[1];
				gt.v2pos2[1] = gt.v2size[1] - gt.v2pos2[1];

				vec2d p1, p2;

				p1[0] = pnode->b3axis1.lo[0] + double(gt.v2pos1[0] - pnode->b3bbox1.lo[0])*pnode->b3axis1.x_width() / double(pnode->b3bbox1.x_width());
				p1[1] = pnode->b3axis1.lo[1] + double(gt.v2pos1[1] - pnode->b3bbox1.lo[1])*pnode->b3axis1.y_width() / double(pnode->b3bbox1.y_width());
				p2[0] = pnode->b3axis1.lo[0] + double(gt.v2pos2[0] - pnode->b3bbox1.lo[0])*pnode->b3axis1.x_width() / double(pnode->b3bbox1.x_width());
				p2[1] = pnode->b3axis1.lo[1] + double(gt.v2pos2[1] - pnode->b3bbox1.lo[1])*pnode->b3axis1.y_width() / double(pnode->b3bbox1.y_width());


				pnode->b3axis1.set_x(std::min(p1[0], p2[0]), std::max(p1[0],p2[0]));
				pnode->b3axis1.set_y(std::min(p1[1], p2[1]), std::max(p1[1],p2[1]));

			}
			return GLParam::FLAG_RELEASE | GLParam::FLAG_REFRESH;
		}
		return 0;
	}

	int OnBtnCancel(GLTool& gt)
	{
		pnode->b3bbox1=b3bbox2;
		pnode->b3axis1=b3axis2;
		return GLToolData::OnBtnCancel(gt);
	}

	virtual int OnBtnDown(GLTool& gt)
	{
		b3bbox2=pnode->b3bbox1;
		b3axis2=pnode->b3axis1;

		if (gt.btn_id == 1||gt.btn_id==2)
		{
			return GLParam::FLAG_CAPTURE;
		}
		else if (gt.btn_id == 3)
		{
			pnode->b3bbox1=pnode->b3bbox0;
			pnode->b3axis1=pnode->b3axis0;
			return GLParam::FLAG_REFRESH;
		}
		else
		{
			return 0;
		}
	}

	virtual int OnWheel(GLTool& gt)
	{
		if (gt.flags.get(GLParam::BTN_IS_DOWN))
		{
			return 0;
		}

		vec3d wd = pnode->b3axis1.width();
		double dd = 0.5* gt.wheel*3.0 / (100.0 + ::fabs(3.0*gt.wheel));
		if (dd < 0.0)
		{
			dd = dd / (1.0 - dd);
		}

		pnode->b3axis1.lo[0] += wd[0] * dd;
		pnode->b3axis1.hi[0] -= wd[0] * dd;
		pnode->b3axis1.lo[1] += wd[1] * dd;
		pnode->b3axis1.hi[1] -= wd[1] * dd;

		return GLParam::FLAG_REFRESH;
	}


	virtual int OnBtnDClick(GLTool&)
	{
		return 0;
	}
};


DataPtrT<GLToolData> DataNodeSymbolT<DCoord2D>::GetToolData()
{
	return new GLToolDataCoord2d(this);
}

template<>
class DataNodeSymbolT<DFigDataManager> : public DataNodeSymbol
{
public:
	typedef DataNodeSymbol basetype;
	DataNodeSymbolT(DataNode* n, DObject* p) :basetype(n, p)
	{
	}

	void DoRender(GLDC& dc)
	{
		GLClipLocker lock1(dc, dc.ci);
		DataNodeSymbol::DoRender(dc);
	}
};


template<>
class DataNodeSymbolT<DFigDataModel> : public DataNodeSymbol
{
public:
	typedef DataNodeSymbol basetype;
	DataNodeSymbolT(DataNode* n, DObject* p) :basetype(n, p)
	{
	}

	void DoRender(GLDC& dc)
	{
		GLClipLocker lock1(dc, dc.ci);
		GLMatrixLocker lock2(dc, dc.bi.m4data);
		dc.bi.m4data.LoadIdentity();

		DataNodeSymbol::DoRender(dc);
	}
};


class DLLIMPEXP_EWC_BASE GLToolDataCoord3d : public GLToolData
{
public:

	GLToolDataCoord3d(mat4d& mt0_, mat4d& mt1_) :mt0(mt0_), mt1(mt1_)
	{
		mt2 = mt1;
	}


	mat4d& mt0;
	mat4d& mt1;
	mat4d mt2;

	virtual int OnDraging(GLTool& gt)
	{
		gt.flags.add(GLParam::BTN_IS_MOVED);

		if (gt.btn_id == 2)
		{
			mt1 = mt2;

			vec2i dvec = gt.v2pos2 - gt.v2pos1;
			dvec[1] *= -1;
			vec3d pt(dvec[0], dvec[1],0);

			mat4d m4d;
			m4d.LoadIdentity();
			m4d.Translate(pt);

			mt1 = m4d*mt1;

			return GLParam::FLAG_REFRESH;
		}
		else if (gt.btn_id == 1)
		{
			mt1 = mt2;

			vec2i dvec = gt.v2pos2 - gt.v2pos1;
			dvec[1] *= -1;
			vec3d pt(dvec[0], dvec[1],0);

			double L1=pt.length();
			if (L1 != 0.0)
			{
				pt /= L1;
				std::swap(pt[0], pt[1]);
				pt[0] *= -1.0;

				mat4d m4d;
				m4d.LoadIdentity();
				m4d.Rotate(0.5*L1, pt);
				mt1 = m4d*mt1;

			}
			

			return GLParam::FLAG_REFRESH;
		}
		else
		{
			return GLParam::FLAG_CACHING | GLParam::FLAG_REFRESH;
		}
	}

	int OnBtnUp(GLTool& gt)
	{
		if (gt.flags.get(GLParam::BTN_IS_DOWN))
		{
			if (gt.btn_id == 1 && gt.v2pos1[0] != gt.v2pos2[0] && gt.v2pos1[1] != gt.v2pos2[1])
			{


			}
			return GLParam::FLAG_RELEASE | GLParam::FLAG_REFRESH;
		}
		return 0;
	}

	int OnBtnCancel(GLTool& gt)
	{

		return GLToolData::OnBtnCancel(gt);
	}

	virtual int OnBtnDown(GLTool& gt)
	{

		if (gt.btn_id == 1 || gt.btn_id == 2)
		{
			return GLParam::FLAG_CAPTURE;
		}
		else if (gt.btn_id == 3)
		{
			return GLParam::FLAG_REFRESH;
		}
		else
		{
			return 0;
		}
	}

	virtual int OnWheel(GLTool& gt)
	{
		if (gt.flags.get(GLParam::BTN_IS_DOWN))
		{
			return 0;
		}


		double dd =  gt.wheel*3.0 / (100.0 + ::fabs(3.0*gt.wheel));
		if (dd < 0.0)
		{
			dd = dd / (1.0 - dd);
		}

		mt1.Scale(1.0 + dd);

		return GLParam::FLAG_REFRESH;
	}


	virtual int OnBtnDClick(GLTool&)
	{
		return 0;
	}
};

template<>
class DataNodeSymbolT<DCoord3D> : public DataNodeSymbolT<DCoord>
{
public:

	typedef DataNodeSymbolT<DCoord> basetype;

	DataNodeSymbolT(DataNode* n, DObject* p) :basetype(n, p)
	{
		b3axis0.set_x(-100.0, +100.0);
		b3axis0.set_y(-100.0, +100.0);
		b3axis0.set_z(-100.0, +100.0);

		b3axis1 = b3axis0;

		light0.v4position.set4(0.0f, 0.0f, 200.0f, 0.0f);
		light0.v4ambient.set4(0.2f, 0.2f, 0.2f, 1.0f);
		light0.v4diffuse.set4(0.8f,0.8f,0.8f,1.0f);
		light0.v4specular.set4(0.1f, 0.1f, 0.1f, 1.0f);

		nscale = 1.0;

	}

	virtual DataPtrT<GLToolData> GetToolData()
	{
		return new GLToolDataCoord3d(mt0,mt1);
	}

	
	mat4d mt0, mt1;

	DLight light0;

	vec4d plane[6];

	double nscale;

	DFontStyle font;

	DLineStyle line_bbox;

	void DoUpdateAttribute(GLDC& dc)
	{
		basetype::DoUpdateAttribute(dc);
		dc.attrupdator.Update("line.bbox", line_bbox);
	}


	void DoRender(GLDC& dc)
	{

		dc.UpdateAttribute(this);

		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{
			DoSetRealSize(dc.bi.b3bbox);
			dc.bi.b3bbox=b3bbox1;
			clinfo.set(b3bbox1);

			if (dc.bi.b3axis.is_valid() && dc.bi.b3axis!=b3axis0)
			{
				b3axis1=b3axis0 = dc.bi.b3axis;
			}

			double _nLength = b3axis0.width().length();
			double _nXWidth = b3bbox1.x_width();
			double _nYWidth=b3bbox1.y_width();
			nscale = 0.9*::sqrt(_nXWidth*_nXWidth+_nYWidth*_nYWidth)/_nLength;

			basetype::DoRender(dc);
			return;
		}

		vec3d c3 = 0.5*(dc.bi.b3bbox.lo + dc.bi.b3bbox.hi);
		dc.bi.m4data.LoadIdentity();
		dc.bi.m4data.Translate(vec3d(c3[0], c3[1], 0.0));
		dc.bi.m4data=dc.bi.m4data*mt1;		dc.bi.m4data.Scale(nscale);

		LockState<GLClipInfo> lock_ci(dc.ci,clinfo);

		if (dc.Mode() == GLDC::RENDER_SELECT)
		{
	
			glDepthMask(false);

			glBegin(GL_QUADS);
			glVertex2d(b3bbox1.lo[0], b3bbox1.lo[1]);
			glVertex2d(b3bbox1.hi[0], b3bbox1.lo[1]);
			glVertex2d(b3bbox1.hi[0], b3bbox1.hi[1]);
			glVertex2d(b3bbox1.lo[0], b3bbox1.hi[1]);
			glEnd();
			glDepthMask(true);

			basetype::DoRender(dc);

			return;
		}


		if (dc.Mode() == GLDC::RENDER_SOLID)
		{

			ew::GLStatusLocker lock(GL_LIGHTING, true);

			dc.Color(DColor(255, 0, 0));
			::glTranslated(c3[0], c3[1], 0.0);
			dc.Light(light0, true);
			::glTranslated(-c3[0], -c3[1], 0.0);

			basetype::DoRender(dc);

			dc.Light(light0, false);
		}

		if (dc.Mode() == GLDC::RENDER_TEXT)
		{

			if (value->flags.get(DCoord3D::FLAG_SHOW_BOX))
			{
				GLMatrixLocker lock(dc, dc.bi.m4data);

				dc.LineStyle(line_bbox);

				::glBegin(GL_LINES);
				::glVertex3d(b3axis0.lo[0],b3axis0.lo[1],b3axis0.lo[2]);
				::glVertex3d(b3axis0.hi[0],b3axis0.lo[1],b3axis0.lo[2]);
				::glVertex3d(b3axis0.lo[0],b3axis0.hi[1],b3axis0.lo[2]);
				::glVertex3d(b3axis0.hi[0],b3axis0.hi[1],b3axis0.lo[2]);
				::glVertex3d(b3axis0.lo[0],b3axis0.lo[1],b3axis0.lo[2]);
				::glVertex3d(b3axis0.lo[0],b3axis0.hi[1],b3axis0.lo[2]);
				::glVertex3d(b3axis0.hi[0],b3axis0.lo[1],b3axis0.lo[2]);
				::glVertex3d(b3axis0.hi[0],b3axis0.hi[1],b3axis0.lo[2]);
				::glVertex3d(b3axis0.lo[0],b3axis0.lo[1],b3axis0.hi[2]);
				::glVertex3d(b3axis0.hi[0],b3axis0.lo[1],b3axis0.hi[2]);
				::glVertex3d(b3axis0.lo[0],b3axis0.hi[1],b3axis0.hi[2]);
				::glVertex3d(b3axis0.hi[0],b3axis0.hi[1],b3axis0.hi[2]);
				::glVertex3d(b3axis0.lo[0],b3axis0.lo[1],b3axis0.hi[2]);
				::glVertex3d(b3axis0.lo[0],b3axis0.hi[1],b3axis0.hi[2]);
				::glVertex3d(b3axis0.hi[0],b3axis0.lo[1],b3axis0.hi[2]);
				::glVertex3d(b3axis0.hi[0],b3axis0.hi[1],b3axis0.hi[2]);

				::glVertex3d(b3axis0.lo[0],b3axis0.lo[1],b3axis0.lo[2]);
				::glVertex3d(b3axis0.lo[0],b3axis0.lo[1],b3axis0.hi[2]);
				::glVertex3d(b3axis0.hi[0],b3axis0.lo[1],b3axis0.lo[2]);
				::glVertex3d(b3axis0.hi[0],b3axis0.lo[1],b3axis0.hi[2]);
				::glVertex3d(b3axis0.lo[0],b3axis0.hi[1],b3axis0.lo[2]);
				::glVertex3d(b3axis0.lo[0],b3axis0.hi[1],b3axis0.hi[2]);
				::glVertex3d(b3axis0.hi[0],b3axis0.hi[1],b3axis0.lo[2]);
				::glVertex3d(b3axis0.hi[0],b3axis0.hi[1],b3axis0.hi[2]);

				::glEnd();
			}

			basetype::DoRender(dc);

		}

	}
};





template<>
class DataNodeSymbolT<DAxisUnitD> : public DataNodeSymbol
{
public:
	typedef DataNodeSymbol basetype;

	DataPtrT<DAxisUnitD> value;

	DataNodeSymbolT(DataNode* n, DObject* p) :basetype(n, p)
	{
		value.cast_and_set(p);
		wwdir=value?value->m_nDirection:0;
		v3lbdir[wwdir]=1.0;		
		lbmax = value &&  value->flags.get(DAxisUnitD::FLAG_LABEL_DIR_MAX);
		lbmin = true;

	}

	double DoGetExtend(GLDC& dc,const String& text)
	{
		vec2i v2s=dc.GetExtend(text);
		return (v3lbdir[0]*v2s[0]+v3lbdir[1]*v2s[1])+4.0;
	}

	int wwdir;
	box1d range;
	arr_1t<DAxisUnitD::Tick> ticks;
	vec3d v3lbdir;

	bool lbmax;
	bool lbmin;

	DLineStyle line_tick,line_main;

	DFontStyle font_text;

	GLBBoxInfo bi;

	void DoUpdateAttribute(GLDC& dc)
	{
		basetype::DoUpdateAttribute(dc);
		dc.attrupdator.Update("line.axis.tick", line_tick);
		dc.attrupdator.Update("line.axis.main", line_main);	
		dc.attrupdator.Update("text.tick", font_text);
	}

	void DoRender(GLDC& dc)
	{
		if(!value) return;

		dc.UpdateAttribute(this);

		dc.SetFont(font_text);

		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{
			DoGenerate(dc);
		}
		else if (dc.Mode() == GLDC::RENDER_TEXT || dc.Mode() == GLDC::RENDER_SELECT)
		{
			DoRenderN(dc);
		}
	}

	void DoRenderN(GLDC& dc)
	{
		box3d b3axis(bi.b3axis);
		box3d b3bbox(bi.b3bbox);

		::glDisable(GL_LINE_STIPPLE);
		::glLineWidth(1.0);
		::glBegin(GL_LINES);
			::glVertex2d(b3bbox.lo[0],b3bbox.lo[1]);
			::glVertex2d(b3bbox.hi[0],b3bbox.lo[1]);
		::glEnd();		

		arr_1t<DAxisUnitD::Tick>& _aTicks(ticks);

		int udir=wwdir;
		int vdir=udir==0?1:0;

		double shfdir=+1.0;

		vec3d v3pos1,v3pos2;
		v3pos1[vdir]=b3bbox.lo[vdir];
		v3pos2[vdir]=b3bbox.hi[vdir];

		vec3d v3lbshf=wwdir==0?vec3d(0,-shfdir,0):vec3d(-shfdir,0.0);
		vec3d v3lbpxl=wwdir==0?vec3d(0,-shfdir*5,0):vec3d(-shfdir*5,0.0);

		dc.LineStyle(line_tick);

		::glBegin(GL_LINES);
		for(int j=0;j<(int)_aTicks.size();j++)
		{
			DAxisUnitD::Tick &tick(_aTicks[j]);
			double pos=(tick.m_nValue-b3axis.lo[udir])*(b3bbox.hi[udir]-b3bbox.lo[udir])/(b3axis.hi[udir]-b3axis.lo[udir])+b3bbox.lo[udir];
			v3pos2[udir]=v3pos1[udir]=pos;


			if (lbmin)
			{
				::glVertex2dv(v3pos1.data());
				if(tick.flags.get(DAxisUnitD::Tick::TICK_MAIN))
				{
					::glVertex2dv((v3pos1+v3lbpxl).data());
				}
				else
				{
					::glVertex2dv((v3pos1+v3lbpxl*0.5).data());
				}
			}
			if (lbmax)
			{
				::glVertex2dv(v3pos2.data());
				if(tick.flags.get(DAxisUnitD::Tick::TICK_MAIN))
				{
					::glVertex2dv((v3pos2-v3lbpxl).data());
				}
				else
				{
					::glVertex2dv((v3pos2-v3lbpxl*0.5).data());
				}
			}

		}
		::glEnd();


		dc.LineStyle(line_main);

		if(value->flags.get(DAxisUnitD::FLAG_SHOW_MESH_MAIN))
		{

			::glBegin(GL_LINES);
			for(int j=0;j<(int)_aTicks.size();j++)
			{
				DAxisUnitD::Tick &tick(_aTicks[j]);
				double pos=(tick.m_nValue-b3axis.lo[udir])*(b3bbox.hi[udir]-b3bbox.lo[udir])/(b3axis.hi[udir]-b3axis.lo[udir])+b3bbox.lo[udir];

				if( tick.flags.get(DAxisUnitD::Tick::TICK_MAIN))
				{
					v3pos1[udir]=v3pos2[udir]=pos;
					::glVertex2dv(v3pos1.data());
					::glVertex2dv(v3pos2.data());
				}
			}
			::glEnd();
		}


		for(size_t j=0;j<_aTicks.size();j++)
		{
			DAxisUnitD::Tick &tick(_aTicks[j]);
			if(!tick.flags.get(DAxisUnitD::Tick::TICK_MAIN)) continue;

			v3pos1[udir]=(tick.m_nValue-b3axis.lo[udir])*(b3bbox.hi[udir]-b3bbox.lo[udir])/(b3axis.hi[udir]-b3axis.lo[udir])+b3bbox.lo[udir];
			dc.PrintText(tick.m_sLabel,v3pos1,v3lbshf,v3lbpxl);
	
		}

	}

	DAxisUnitD::Tick tk;

	void GenerateTick(double v)
	{
		tk.set(v);
		ticks.push_back(tk);
	}

	void DoGenerate(GLDC& dc)// , double lo, double hi, double wd)
	{
		bi = dc.bi;

		double lo = bi.b3axis.lo[wwdir];
		double hi = bi.b3axis.hi[wwdir];
		double wd = bi.b3bbox.hi[wwdir] - bi.b3bbox.lo[wwdir];

		range.set_x(lo,hi);
		ticks.clear();

		if(wd<10.0)
		{
			return;
		}
	
		double dmax=std::max(lo,hi);
		double dmin=std::min(lo,hi);

		double ws=28.0;

		double br0=(dmax-dmin)*ws/::fabs(wd);
		double br=::pow(10.0,floor(::log10(br0)));
	
		double v1ws=16.0;

		double bw0=br/(dmax-dmin)*(wd);

		//String text;
		long k1=::floor(dmin/br);
		long k2=::ceil(dmax/br);
		long dk=(k2-k1);
		if(dk<=0) dk=1;

		//int tmax=1;
	
		arr_1t<long> vectk;
		vectk.push_back(k1-1);
		vectk.push_back(k1+0);
		vectk.push_back(k1+1);
		vectk.push_back(k2-1);
		vectk.push_back(k2+0);
		vectk.push_back(k2+1);

		for(int i=0;i<(int)vectk.size();i++)
		{
			long k=vectk[i];
			tk.set(br*double(k));
			double v1ts = DoGetExtend(dc, tk.m_sLabel);
			if(v1ts>v1ws) v1ws=v1ts;
		}

		if(br0/br>4.0||ws/bw0>4.0)
		{
			double brx=br*10;
			long k1=::floor(dmin/brx);
			long k2=::ceil(dmax/brx);

			vectk.clear();
			vectk.push_back(k1-1);
			vectk.push_back(k1+0);
			vectk.push_back(k1+1);
			vectk.push_back(k2-1);
			vectk.push_back(k2+0);
			vectk.push_back(k2+1);

			for(int i=0;i<(int)vectk.size();i++)
			{
				long k=vectk[i];
				tk.set(brx*double(k));
				double v1ts = DoGetExtend(dc, tk.m_sLabel);
				if(v1ts>v1ws) v1ws=v1ts;
			}
		}

		ws=v1ws;
		ws+=8;
		if(ws<30) ws=30;

		double bw=br/(dmax-dmin)*(wd);

		int flag=1;
		while(flag<50)
		{
			double _tmp=flag;
			if(bw*_tmp*1.0>=ws)
			{
				flag*=1;
				break;			
			}

			if(bw*_tmp*2.0>=ws)
			{
				flag*=2;
				break;			
			}

			if(bw*_tmp*5.0>=ws)
			{
				flag*=5;
				break;			
			}

			if(flag>=10)
			{
				flag=50;
				break;
			}

			flag*=10;
		}


		br=br*double(flag);
		bw=bw*double(flag);

		if(flag>=10) flag=flag/10;
		if(flag>=10) flag=flag/10;

		int64_t y1 = dmin / br;
		int64_t y2 = dmax / br;

		if (y2>y1 && dmax > br*double(y2))
		{
			y2 = y2 + 1;
		}

		tk.flags.add(DAxisUnitD::Tick::TICK_MAIN);
		for(int64_t y=y1; y<=y2; y++)
		{
			double d=br*double(y);
			if(d>=dmin&&d<=dmax)
			{
				GenerateTick(d);
			}
		}

		tk.flags.del(DAxisUnitD::Tick::TICK_MAIN);

		double mm=15.0;

		long tm=(long)(bw/mm);

		for(int64_t y=y1; y<=y2; y++)
		{
			{
				long tt=10;
				if(tm>=tt&&(flag==1))
				{
					for(long i=1;i<tt;i++)
					{
						double d=br*(double(y)+double(i)/double(tt));
						if(d>=dmin&&d<=dmax)
						{
							GenerateTick(d);
						}
					}
					continue;
				}
			}
			{
				long tt=5;
				if(tm>=tt&&(flag==5||flag==1))
				{
					for(long i=1;i<tt;i++)
					{
						double d=br*(double(y)+double(i)/double(tt));
						if(d>=dmin&&d<=dmax)
						{
							GenerateTick(d);
						}
					}
					continue;
				}
			}
			{
				long tt=4;
				if(tm>=tt&&(flag==2))
				{
					for(long i=1;i<tt;i++)
					{
						double d=br*(double(y)+double(i)/double(tt));
						if(d>=dmin&&d<=dmax)
						{
							GenerateTick(d);
						}
					}
					continue;
				}
			}
			{
				long tt=2;
				if(tm>=tt&&(flag==1||flag==2||flag==5))
				{
					for(long i=1;i<tt;i++)
					{
						double d=br*(double(y)+double(i)/double(tt));
						if(d>=dmin&&d<=dmax)
						{
							GenerateTick(d);
						}
					}
					continue;
				}
			}		

		}
	}
};

template<>
class DataNodeSymbolT<DAxis> : public DataNodeSymbol
{
public:
	typedef DataNodeSymbol basetype;

	DataPtrT<DAxis> value;

	DataNodeSymbolT(DataNode* n, DObject* p) :basetype(n, p)
	{
		value.cast_and_set(p);	
	}

	DLineStyle line_main;
	DLineStyle line_axis_x;
	DLineStyle line_axis_y;
	DLineStyle line_axis_z;

	box3d b3bbox0;

	void DoRender_SPHERE(GLDC& dc)
	{

	}

	void DoRender_3D_BOX(GLDC& dc)
	{

	}

	void DoRender_3D_DIR(GLDC& dc)
	{

		if (dc.Mode() == GLDC::RENDER_TEXT || dc.Mode() == GLDC::RENDER_SELECT)
		{
			GLClipLocker locker1(dc, dc.ci);
			GLMatrixLocker locker2(dc,dc.bi.m4data);

			::glBegin(GL_LINES);
			dc.LineStyle(line_axis_x);
			::glVertex3d(-1.0e6,0.0,0.0);
			::glVertex3d(+1.0e6,0.0,0.0);
			dc.LineStyle(line_axis_y);
			::glVertex3d(0.0,-1.0e6,0.0);
			::glVertex3d(0.0,+1.0e6,0.0);
			dc.LineStyle(line_axis_z);
			::glVertex3d(0.0,0.0,-1.0e6);
			::glVertex3d(0.0,0.0,+1.0e6);
			::glEnd();
		}		
	}

	void DoUpdateAttribute(GLDC& dc)
	{
		basetype::DoUpdateAttribute(dc);
		dc.attrupdator.Update("line.axis_dir.x", line_axis_x);
		dc.attrupdator.Update("line.axis_dir.y", line_axis_y);	
		dc.attrupdator.Update("line.axis_dir.z", line_axis_z);
	}

	void DoRender(GLDC& dc)
	{
		if(!value||value->m_nMode==DAxis::MODE_NONE)
		{
			return;
		}

		dc.UpdateAttribute(this);

		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{
			b3bbox0=dc.bi.b3bbox;
		}

		if(value->m_nMode==DAxis::MODE_3D_DIR)
		{
			DoRender_3D_DIR(dc);
		}
		else if(value->m_nMode==DAxis::MODE_SPHERE)
		{
			DoRender_SPHERE(dc);
		}
		else if(value->m_nMode==DAxis::MODE_3D_BOX)
		{
			DoRender_3D_BOX(dc);
		}
		else
		{
			basetype::DoRender(dc);
		}



	}
};

template<>
class DataNodeSymbolT<FigData> : public DataNodeSymbol
{
public:
	typedef DataNodeSymbol basetype;
	DataNodeSymbolT(DataNode* n, DObject* p) :basetype(n, p)
	{
	}
};



template<>
class DataNodeSymbolT<DFigData2D> : public DataNodeSymbolT<FigData>
{
public:

	typedef DataNodeSymbolT<FigData> basetype;

	arr_1t<unsigned> valid_index;

	DataPtrT<DFigData2D> value;



	DataNodeSymbolT(DataNode* n, DObject* p) :basetype(n, p)
	{
		if (!value.cast_and_set(p)) return;
	}

	box3d b3bbox;
	mat4d m4data;

	arr_1t<vec3d> cached_data;

	virtual void DoUpdateCachedData(GLDC& dc)
	{
		bool _b_is_data_updated = false;

		arr_1t<double> &adata(value->m_aTdata);
		arr_1t<double> &bdata(value->m_aValue);

		size_t n =bdata.size();

		if (m4data != dc.bi.m4data || cached_data.size() != bdata.size())
		{

			cached_data.resize(n);
			m4data = dc.bi.m4data;

			if (adata.size() != bdata.size())
			{
				for (size_t i = 0; i < n; i++)
				{
					cached_data[i] = m4data*vec3d(i, bdata[i], 0);
				}
			}
			else if (value->m_nDataType == DFigData2D::TYPE_POLAR)
			{
				for (size_t i = 0; i < n; i++)
				{
					double r = bdata[i];
					double a = adata[i];
					cached_data[i] = m4data*vec3d(r*cosdeg(a), r*sindeg(a), 0);
				}
			}
			else
			{
				for (size_t i = 0; i < n; i++)
				{
					cached_data[i] = m4data*vec3d(adata[i], bdata[i], 0);
				}
			}	

			_b_is_data_updated = true;
		}

		if (_b_is_data_updated || b3bbox != dc.bi.b3bbox)
		{
			b3bbox = dc.bi.b3bbox;
			valid_index.clear();
			valid_index.push_back(0);
			valid_index.push_back(n);
		}
	}

	DLineStyle line_main;


	void DoUpdateAttribute(GLDC& dc)
	{
		basetype::DoUpdateAttribute(dc);
		dc.attrupdator.Update2("line.data", line_main);
	}

	void DoRender(GLDC& dc)
	{
		if (!value) return;
		dc.UpdateAttribute(this);

		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{
			DoUpdateCachedData(dc);
		}
		
		if (dc.Mode() == GLDC::RENDER_SOLID||dc.Mode() == GLDC::RENDER_SELECT)
		{
			dc.LineStyle(line_main);

			for (size_t j = 1; j < valid_index.size(); j += 2)
			{
				glBegin(GL_LINE_STRIP);
				for (unsigned i = valid_index[j - 1]; i < valid_index[j]; i++)
				{
					glVertex3dv(cached_data[i].data());
				}
				glEnd();
			}
		}


	}

};


template<>
class DataNodeSymbolT<DFigData3D> : public DataNodeSymbolT<FigData>
{
public:

	typedef DataNodeSymbolT<FigData> basetype;


	DataPtrT<DFigData3D> value;
	mat4d m4data;

	DataNodeSymbolT(DataNode* n, DObject* p) :basetype(n, p)
	{
		if (!value.cast_and_set(p)) return;
	}

	mat4d m4;

	void Vertex(const vec3d& v)
	{
		::glVertex3dv(v.data());
	}

	void DoRender(GLDC& dc)
	{
		if (!value) return;

		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{
			return;
		}

		GLMatrixLocker lock2(dc, dc.bi.m4data);

		if (dc.Mode() == GLDC::RENDER_SOLID||dc.Mode() == GLDC::RENDER_SELECT)
		{

			arr_1t<vec3d> vv;
			vv.push_back(vec3d(-1, -1, -1));
			vv.push_back(vec3d(+1, -1, -1));
			vv.push_back(vec3d(-1, +1, -1));
			vv.push_back(vec3d(+1, +1, -1));
			vv.push_back(vec3d(-1, -1, +1));
			vv.push_back(vec3d(+1, -1, +1));
			vv.push_back(vec3d(-1, +1, +1));
			vv.push_back(vec3d(+1, +1, +1));
			
			::glScaled(50.0, 50.0, 50.0);
			

			dc.Color(DColor(255,0,0));

			::glBegin(GL_TRIANGLES);
			
			::glNormal3d(0, 0, -1);
			Vertex(vv[0]);
			Vertex(vv[2]);
			Vertex(vv[1]);

			Vertex(vv[1]);
			Vertex(vv[2]);
			Vertex(vv[3]);

			::glNormal3d(0, 0, +1);
			Vertex(vv[4]);
			Vertex(vv[5]);
			Vertex(vv[6]);

			Vertex(vv[6]);
			Vertex(vv[5]);
			Vertex(vv[7]);

			::glNormal3d(-1, 0, 0);
			Vertex(vv[0]);
			Vertex(vv[4]);
			Vertex(vv[2]);

			Vertex(vv[2]);
			Vertex(vv[4]);
			Vertex(vv[6]);

			::glNormal3d(+1, 0, 0);
			Vertex(vv[1]);
			Vertex(vv[3]);
			Vertex(vv[5]);

			Vertex(vv[5]);
			Vertex(vv[3]);
			Vertex(vv[7]);

			::glNormal3d(0, -1, 0);
			Vertex(vv[0]);
			Vertex(vv[1]);
			Vertex(vv[4]);

			Vertex(vv[4]);
			Vertex(vv[1]);
			Vertex(vv[5]);

			::glNormal3d(0, +1, 0);
			Vertex(vv[2]);
			Vertex(vv[6]);
			Vertex(vv[3]);

			Vertex(vv[3]);
			Vertex(vv[6]);
			Vertex(vv[7]);

			::glEnd();

		}


	}

};



class MvcViewFigure : public MvcViewEx
{
public:
	typedef MvcViewEx basetype;

	
	bool first;

	~MvcViewFigure()
	{
		pmodel->DecRef();
	}

	IWnd_bookbase* m_pBook;
	AutoPtrT<wxWindow> m_pMyView;

	bool OnCreated()
	{
		m_pBook = dynamic_cast<IWnd_bookbase*>(WndManager::current().evtmgr["Wnd.Variable"].GetWindow());
		if (m_pBook)
		{
			m_pMyView.reset(pmodel->CreateDataView(m_pBook));
		}
		return true;
	}

	static DObject* CreateFigure1()
	{

		DCoord* c = new DCoord2D;
		c->m_sId = "coord2d";

		DFigData2D* d = new DFigData2D;
		d->m_sId = "data1";
		c->m_pDataManager->m_aItems.append(d);

		d = new DFigData2D;
		d->m_sId = "data2";

		c->m_pDataManager->m_aItems.append(d);

		DText* t = new DText;
		t->m_sText = "title";
		t->m_sId = "title";
		t->m_v3Pos[1] = 1.0;
		t->m_v3Shf[1] = 1.0;
		t->m_v3Pxl[1] = 8.0;
		c->m_aItems.append(t);

		t = new DText;
		t->m_sText = "xlabel";
		t->m_sId = "xlabel";
		t->m_v3Pos[1] = -1.0;
		t->m_v3Shf[1] = -1.0;
		t->m_v3Pxl[1] = -23.0;
		c->m_aItems.append(t);


		t = new DText;
		t->m_sText = "ylabel";
		t->m_sId = "ylabel";
		t->m_v3Pos[0] = -1.0;
		t->m_v3Shf[0] = -1.0;
		t->m_v3Pxl[0] = -23.0;

		t->m_pAttribute.reset(new DAttribute);
		t->m_pAttribute->SetValue("font_flags.text", (int)DFontStyle::STYLE_VERTICAL);

		c->m_aItems.append(t);

		return c;
	}

	static DObject* CreateFigure2()
	{

		DCoord* c = new DCoord3D;
		c->m_sId = "coord3d";

		FigData* d = new DFigData3D;
		d->m_sId = "data1";

		c->m_pDataManager->m_aItems.append(d);		

		return c;

	}

	static DataModel* CreateDataModel2()
	{

		DataModel* model = new DataModel();
		model->AddColumn(new DataColumnName);
		model->AddColumn(new DataColumnType);

		DObject* p=CreateFigure2();
		model->Update(p);

		return model;
	}


	static DataModel* CreateDataModel1()
	{

		DataModel* model = new DataModel();

		model->AddColumn(new DataColumnName);
		model->AddColumn(new DataColumnType);


		DObject* p1 = CreateFigure1();
		DObject* p2 = CreateFigure2();

		DTableRow* p3=new DTableRow("row");
		DTableCell* c1=new DTableCell("cell");
		DTableCell* c2=new DTableCell("cell");

		c1->m_aItems.append(p1);
		c2->m_aItems.append(p2);

		p3->m_aItems.append(c1);
		p3->m_aItems.append(c2);

		model->Update(p3);

		model->m_pAttributeManager.reset(new DAttributeManager);

		return model;
	}

	MvcViewFigure(MvcModel& tar) :basetype(tar)
	{
		static int g_n_figure = 0;
		pmodel = ++g_n_figure % 2 ? CreateDataModel1() : CreateDataModel2();

		first = true;

	}

	DataModel* pmodel;
	AtomicSpin spin;



	virtual bool OnWndEvent(IWndParam&,int)
	{
		return true;
	}

	bool DoActivate(WndManager& wm,int v)
	{
		if(!basetype::DoActivate(wm,v)) return false;
		if(v==0) return true;

		wm.evtmgr["Wnd.Variable"].CmdExecuteEx(v>0 ? 2 : -2);
		if (m_pBook) m_pBook->SelPage(v>0 ? m_pMyView.get() : NULL);

		wm.evtmgr["Figure"].flags.set(EvtBase::FLAG_HIDE_UI,v<0);

		wm.wup.mu_set("MenuBar.default");
		wm.wup.gp_add("OtherWindow");
		wm.wup.gp_add("Figure");

		wm.wup.sb_set(v>0?"StatusBar.figviewer":"StatusBar.default");


		return true;
	}


	LitePtrT<IWnd_modelview> m_pCanvas;

	void OnDestroy()
	{
		basetype::OnDestroy();
	}

	wxWindow* CreateCanvas(wxWindow* w)
	{

		fn.SetExts(_hT("Text Files")+"(*.txt) | *.txt");
		m_pCanvas.reset(new IWnd_modelview(w, WndProperty()));

		m_pCanvas->pmodel = pmodel;
		return m_pCanvas;
	}

};


DataPtrT<MvcModel> PluginDomViewer::CreateSampleModel()
{
	return new MvcModelT<MvcViewFigure>;
}



bool PluginDomViewer::OnAttach()
{

	DataNodeCreator::Register<DFigure>();
	DataNodeCreator::Register<DCoord2D>();
	DataNodeCreator::Register<DFigData2D>();
	DataNodeCreator::Register<DCoord3D>();
	DataNodeCreator::Register<DFigData3D>();
	DataNodeCreator::Register<DFigDataManager>();
	DataNodeCreator::Register<DFigDataModel>();

	DataNodeCreator::Register<DAxis>();

	DataNodeCreator::Register<DAxisUnitD>();
	DataNodeCreator::Register<DText>();

	DataNodeCreator::Register<DTable>();
	DataNodeCreator::Register<DTableRow>();
	DataNodeCreator::Register<DTableCell>();

	EvtManager& ec(wm.evtmgr);

	ec.gp_beg("StatusBar.figviewer","StatusBar");
		ec.gp_add("",-1);
		ec.gp_add("",60);
		ec.gp_add("",60);
		ec.gp_add("",60);
	ec.gp_end();

	ec.gp_beg("Menu.New");
		ec.gp_add(new EvtCommandNewEditorFile(*this,_kT("New.Figure")));
	ec.gp_end();


	ec.gp_beg(_kT("Figure"));
		
	ec.gp_end();

	ec.gp_beg("Menu.Extra");
		ec.gp_add("Figure");
	ec.gp_end();

	ec["Figure"].flags.add(EvtCommand::FLAG_HIDE_UI);


	AttachEvent("Config");

	return true;
}


bool PluginDomViewer::OnCfgEvent(int lv)
{


	return true;
}

PluginDomViewer::PluginDomViewer(WndManager& w):basetype(w,"Plugin.FigViewer")
{
	m_aExtension.insert(".ewd");

}

IMPLEMENT_IPLUGIN(PluginDomViewer)

EW_LEAVE
