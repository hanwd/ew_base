#include "ewc_base/plugin/plugin_plugineditor.h"
#include "ewc_base/editor/plugin_figure_viewer.h"
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

#include "ewa_base/figure.h"


#include "gl/gl.h"

EW_ENTER

template<>
class DataNodeSymbolT<FigFigure> : public DataNodeSymbol
{
public:

	DataNodeSymbolT(DataNode* n, CallableSymbol* p) :DataNodeSymbol(n, p)
	{

	}

	void DoRender(GLDC& dc)
	{
		if (!flags.get(FLAG_TOUCHED))
		{
			// ensure subnodes are generated
			_TouchNode(-1);
		}

		for (size_t i = 0; i < subnodes.size(); i++)
		{
			::glPushMatrix();
			try
			{
				dc.RenderNode(subnodes[i]);
			}
			catch (...)
			{

			}
			::glPopMatrix();
		}
	}
};

template<>
class DataNodeSymbolT<FigCoord> : public DataNodeSymbol
{
public:
public:
	typedef DataNodeSymbol basetype;
	DataNodeSymbolT(DataNode* n, CallableSymbol* p) :basetype(n, p)
	{
	}
};


class DLLIMPEXP_EWC_BASE GLToolDataCoord2d : public GLToolData
{
public:

	GLToolDataCoord2d(GLDC::BBoxInfo& bi0_, GLDC::BBoxInfo& bi1_) :bi0(bi0_), bi1(bi1_)
	{

	}

	GLDC::BBoxInfo& bi0;
	GLDC::BBoxInfo& bi1;
	GLDC::BBoxInfo bi2;

	virtual int OnDraging(GLTool& gt)
	{
		gt.flags.add(GLParam::BTN_IS_MOVED);

		if (gt.btn_id == 2)
		{
			vec2i dvec = gt.v2pos2 - gt.v2pos1;
			dvec[1] *= -1;

			double d1 = double(dvec[0])*bi2.b3axis.x_width() / double(bi2.b3bbox.x_width());
			double d2 = double(dvec[1])*bi2.b3axis.y_width() / double(bi2.b3bbox.y_width());

			bi1.b3axis.lo[0] = bi2.b3axis.lo[0] - d1;
			bi1.b3axis.hi[0] = bi2.b3axis.hi[0] - d1;
			bi1.b3axis.lo[1] = bi2.b3axis.lo[1] - d2;
			bi1.b3axis.hi[1] = bi2.b3axis.hi[1] - d2;

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

				p1[0] = bi1.b3axis.lo[0] + double(gt.v2pos1[0] - bi1.b3bbox.lo[0])*bi1.b3axis.x_width() / double(bi1.b3bbox.x_width());
				p1[1] = bi1.b3axis.lo[1] + double(gt.v2pos1[1] - bi1.b3bbox.lo[0])*bi1.b3axis.y_width() / double(bi1.b3bbox.y_width());
				p2[0] = bi1.b3axis.lo[0] + double(gt.v2pos2[0] - bi1.b3bbox.lo[0])*bi1.b3axis.x_width() / double(bi1.b3bbox.x_width());
				p2[1] = bi1.b3axis.lo[1] + double(gt.v2pos2[1] - bi1.b3bbox.lo[0])*bi1.b3axis.y_width() / double(bi1.b3bbox.y_width());


				bi1.b3axis.set_x(std::min(p1[0], p2[0]), std::max(p1[0],p2[0]));
				bi1.b3axis.set_y(std::min(p1[1], p2[1]), std::max(p1[1],p2[1]));

			}
			return GLParam::FLAG_RELEASE | GLParam::FLAG_REFRESH;
		}
		return 0;
	}

	int OnBtnCancel(GLTool& gt)
	{
		bi1 = bi2;
		return GLToolData::OnBtnCancel(gt);
	}

	virtual int OnBtnDown(GLTool& gt)
	{
		bi2 = bi1;
		if (gt.btn_id == 1||gt.btn_id==2)
		{
			return GLParam::FLAG_CAPTURE;
		}
		else if (gt.btn_id == 3)
		{
			bi1 = bi0;
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

		vec3d wd = bi1.b3axis.width();
		double dd = 0.5* gt.wheel*3.0 / (100.0 + ::fabs(3.0*gt.wheel));
		if (dd < 0.0)
		{
			dd = dd / (1.0 - dd);
		}

		bi1.b3axis.lo[0] += wd[0] * dd;
		bi1.b3axis.hi[0] -= wd[0] * dd;
		bi1.b3axis.lo[1] += wd[1] * dd;
		bi1.b3axis.hi[1] -= wd[1] * dd;

		return GLParam::FLAG_REFRESH;
	}


	virtual int OnBtnDClick(GLTool&)
	{
		return 0;
	}
};

template<>
class DataNodeSymbolT<FigCoord2D> : public DataNodeSymbolT<FigCoord>
{
public:

	typedef DataNodeSymbolT<FigCoord> basetype;

	DataNodeSymbolT(DataNode* n, CallableSymbol* p) :basetype(n, p)
	{
		bi0.b3axis.set_x(-100.0, +100.0);
		bi0.b3axis.set_y(-100.0, +100.0);
		bi0.b3axis.set_z(-1.0, +1.0);

		bi1 = bi0;
	}

	virtual DataPtrT<GLToolData> GetToolData()
	{
		return new GLToolDataCoord2d(bi0,bi1);
	}



	GLDC::BBoxInfo bi0,bi1;

	vec4d plane[6];

	void DoRender(GLDC& dc)
	{
		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{

			dc.bi.b3bbox.lo += 70;
			dc.bi.b3bbox.hi -= 45;

			bi1.b3bbox = dc.bi.b3bbox;
			dc.bi.b3axis = bi1.b3axis;

			plane[0].set3(+1, 0, 0); plane[0][3] = -dc.bi.b3bbox.lo[0];
			plane[1].set3(-1, 0, 0); plane[1][3] = +dc.bi.b3bbox.hi[0];
			plane[2].set3(0, +1, 0); plane[2][3] = -dc.bi.b3bbox.lo[1];
			plane[3].set3(0, -1, 0); plane[3][3] = +dc.bi.b3bbox.hi[1];

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
		}
		
		if (dc.Mode() == GLDC::RENDER_SOLID || dc.Mode() == GLDC::RENDER_SELECT)
		{
			
			for (int i = 0; i < 4; i++)
			{
				::glClipPlane(GL_CLIP_PLANE0 + i, plane[i].data());
				::glEnable(GL_CLIP_PLANE0 + i);
			}

			dc.Color(DColor(255,0,0));

			basetype::DoRender(dc);

			for (int i = 0; i < 4; i++)
			{
				::glDisable(GL_CLIP_PLANE0 + i);
			}
		}
		
		if (dc.Mode() == GLDC::RENDER_TEXT || dc.Mode() == GLDC::RENDER_SELECT)
		{

			DFontStyle font;
			font.color.set(200,200,200);
			dc.SetFont(font);
		
			basetype::DoRender(dc);

			dc.Color(DColor(0, 0, 255));
			glBegin(dc.Mode() == GLDC::RENDER_SELECT ? GL_QUADS : GL_LINE_LOOP);
				glVertex2d(bi1.b3bbox.lo[0], bi1.b3bbox.lo[1]);
				glVertex2d(bi1.b3bbox.hi[0], bi1.b3bbox.lo[1]);
				glVertex2d(bi1.b3bbox.hi[0], bi1.b3bbox.hi[1]);
				glVertex2d(bi1.b3bbox.lo[0], bi1.b3bbox.hi[1]);
			glEnd();

		}

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

	GLDC::BBoxInfo bi2;

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
class DataNodeSymbolT<FigCoord3D> : public DataNodeSymbolT<FigCoord>
{
public:

	typedef DataNodeSymbolT<FigCoord> basetype;

	DataNodeSymbolT(DataNode* n, CallableSymbol* p) :basetype(n, p)
	{
		bi0.b3axis.set_x(-100.0, +100.0);
		bi0.b3axis.set_y(-100.0, +100.0);
		bi0.b3axis.set_z(-100.0, +100.0);

		bi1 = bi0;

		light0.v4position.set4(0.0f, 0.0f, 200.0f, 0.0f);
		light0.v4ambient.set4(0.2f, 0.2f, 0.2f, 1.0f);
		light0.v4diffuse.set4(0.8f,0.8f,0.8f,1.0f);
		light0.v4specular.set4(0.1f, 0.1f, 0.1f, 1.0f);

	}

	virtual DataPtrT<GLToolData> GetToolData()
	{
		return new GLToolDataCoord3d(mt0,mt1);
	}



	GLDC::BBoxInfo bi0, bi1;

	mat4d mt0, mt1;

	DLight light0;

	vec4d plane[6];

	void DoRender(GLDC& dc)
	{
		vec3d c3 = 0.5*(dc.bi.b3bbox.lo + dc.bi.b3bbox.hi);

		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{
			bi1 = dc.bi;
			return;
		}

		dc.bi.m4data = mt1;

		if (dc.Mode() == GLDC::RENDER_SOLID || dc.Mode() == GLDC::RENDER_SELECT)
		{
			if (dc.Mode() == GLDC::RENDER_SOLID)
			{
				::glEnable(GL_LIGHTING);
			}

			dc.Color(DColor(255, 0, 0));
			::glTranslated(c3[0], c3[1], 0.0);

			dc.Light(light0, true);
			basetype::DoRender(dc);
			::glTranslated(-c3[0], -c3[1], 0.0);

			dc.Light(light0, false);

			if (dc.Mode() == GLDC::RENDER_SOLID)
			{
				::glDisable(GL_LIGHTING);
			}
		}

		if (dc.Mode() == GLDC::RENDER_TEXT || dc.Mode() == GLDC::RENDER_SELECT)
		{

			DFontStyle font;
			font.color.set(200, 200, 200);
			dc.SetFont(font);

			::glTranslated(c3[0], c3[1], 0.0);
			basetype::DoRender(dc);
			::glTranslated(-c3[0], -c3[1], 0.0);

			dc.Color(DColor(0, 0, 255));
			glBegin(dc.Mode() == GLDC::RENDER_SELECT ? GL_QUADS : GL_LINE_LOOP);
			glVertex2d(bi1.b3bbox.lo[0], bi1.b3bbox.lo[1]);
			glVertex2d(bi1.b3bbox.hi[0], bi1.b3bbox.lo[1]);
			glVertex2d(bi1.b3bbox.hi[0], bi1.b3bbox.hi[1]);
			glVertex2d(bi1.b3bbox.lo[0], bi1.b3bbox.hi[1]);
			glEnd();

		}

	}
};

template<>
class DataNodeSymbolT<FigDataManager> : public DataNodeSymbol
{
public:
	typedef DataNodeSymbol basetype;
	DataNodeSymbolT(DataNode* n, CallableSymbol* p) :basetype(n, p)
	{
	}
};



template<>
class DataNodeSymbolT<AxisUnitD> : public DataNodeSymbol
{
public:
	typedef DataNodeSymbol basetype;

	DataPtrT<AxisUnitD> value;

	DataNodeSymbolT(DataNode* n, CallableSymbol* p) :basetype(n, p)
	{
		value.cast_and_set(p);
		wwdir=value?value->m_nDirection:0;
		v3lbdir[wwdir]=1.0;		
		lbmax = value &&  value->flags.get(AxisUnitD::FLAG_LABEL_DIR_MAX);
		lbmin = true;
	}

	double DoGetTextExtend(GLDC& dc,const String& text)
	{
		vec2i v2s=dc.GetTextExtend(text);
		return (v3lbdir[0]*v2s[0]+v3lbdir[1]*v2s[1])+4.0;
	}

	int wwdir;
	box1d range;
	arr_1t<AxisUnitD::Tick> ticks;
	vec3d v3lbdir;

	bool lbmax;
	bool lbmin;

	GLDC::BBoxInfo bi;

	void DoRender(GLDC& dc)
	{
		if(!value) return;

		value->FontText.color.a = 0;
		dc.SetFont(value->FontText);

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

		arr_1t<AxisUnitD::Tick>& _aTicks(ticks);

		int udir=wwdir;
		int vdir=udir==0?1:0;

		double shfdir=+1.0;

		vec3d v3pos1,v3pos2;
		v3pos1[vdir]=b3bbox.lo[vdir];
		v3pos2[vdir]=b3bbox.hi[vdir];

		vec3d v3lbshf=wwdir==0?vec3d(0,-shfdir,0):vec3d(-shfdir,0.0);
		vec3d v3lbpxl=wwdir==0?vec3d(0,-shfdir*5,0):vec3d(-shfdir*5,0.0);


		::glBegin(GL_LINES);
		for(int j=0;j<(int)_aTicks.size();j++)
		{
			AxisUnitD::Tick &tick(_aTicks[j]);
			double pos=(tick.m_nValue-b3axis.lo[udir])*(b3bbox.hi[udir]-b3bbox.lo[udir])/(b3axis.hi[udir]-b3axis.lo[udir])+b3bbox.lo[udir];
			v3pos2[udir]=v3pos1[udir]=pos;


			if (lbmin)
			{
				::glVertex2dv(v3pos1.data());
				if(tick.flags.get(AxisUnitD::Tick::TICK_MAIN))
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
				if(tick.flags.get(AxisUnitD::Tick::TICK_MAIN))
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



		if(value->flags.get(AxisUnitD::FLAG_SHOW_MESH_MAIN))
		{

			//dc.LineStyle(value->LineMain);
			::glBegin(GL_LINES);
			for(int j=0;j<(int)_aTicks.size();j++)
			{
				AxisUnitD::Tick &tick(_aTicks[j]);
				double pos=(tick.m_nValue-b3axis.lo[udir])*(b3bbox.hi[udir]-b3bbox.lo[udir])/(b3axis.hi[udir]-b3axis.lo[udir])+b3bbox.lo[udir];

				if( tick.flags.get(AxisUnitD::Tick::TICK_MAIN))
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
			AxisUnitD::Tick &tick(_aTicks[j]);
			if(!tick.flags.get(AxisUnitD::Tick::TICK_MAIN)) continue;

			v3pos1[udir]=(tick.m_nValue-b3axis.lo[udir])*(b3bbox.hi[udir]-b3bbox.lo[udir])/(b3axis.hi[udir]-b3axis.lo[udir])+b3bbox.lo[udir];
			dc.PrintText(tick.m_sLabel,v3pos1,v3lbshf,v3lbpxl);
	
		}

	}

	AxisUnitD::Tick tk;

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
			double v1ts = DoGetTextExtend(dc, tk.m_sLabel);
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
				double v1ts = DoGetTextExtend(dc, tk.m_sLabel);
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

		tk.flags.add(AxisUnitD::Tick::TICK_MAIN);
		for(int64_t y=y1; y<=y2; y++)
		{
			double d=br*double(y);
			if(d>=dmin&&d<=dmax)
			{
				GenerateTick(d);
			}
		}

		tk.flags.del(AxisUnitD::Tick::TICK_MAIN);

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
class DataNodeSymbolT<FigAxisD> : public DataNodeSymbol
{
public:
	typedef DataNodeSymbol basetype;
	DataNodeSymbolT(DataNode* n, CallableSymbol* p) :basetype(n, p)
	{
	}
};

template<>
class DataNodeSymbolT<FigData> : public DataNodeSymbol
{
public:
	typedef DataNodeSymbol basetype;
	DataNodeSymbolT(DataNode* n, CallableSymbol* p) :basetype(n, p)
	{
	}
};



template<>
class DataNodeSymbolT<FigData2D> : public DataNodeSymbolT<FigData>
{
public:

	typedef DataNodeSymbolT<FigData> basetype;

	arr_1t<unsigned> valid_index;

	DataPtrT<FigData2D> value;



	DataNodeSymbolT(DataNode* n, CallableSymbol* p) :basetype(n, p)
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
			else if (value->m_nDataType == FigData2D::TYPE_POLAR)
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



	void DoRender(GLDC& dc)
	{
		if (!value) return;

		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{
			DoUpdateCachedData(dc);
		}
		
		if (dc.Mode() == GLDC::RENDER_SOLID||dc.Mode() == GLDC::RENDER_SELECT)
		{
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
class DataNodeSymbolT<FigData3D> : public DataNodeSymbolT<FigData>
{
public:

	typedef DataNodeSymbolT<FigData> basetype;


	DataPtrT<FigData3D> value;


	DataNodeSymbolT(DataNode* n, CallableSymbol* p) :basetype(n, p)
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
		
		m4 = dc.bi.m4data;
		m4.Scale(20.0);

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


			::glPushMatrix();
			::glMultMatrixd(m4.data());
			

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

			::glPopMatrix();


		}


	}

};


template<>
class DataNodeSymbolT<FigText> : public DataNodeSymbol
{
public:
	typedef DataNodeSymbol basetype;
	DataNodeSymbolT(DataNode* n, CallableSymbol* p) :basetype(n, p)
	{

	}

	vec3d m_v3Position;

	void DoRender(GLDC& dc)
	{
		if (!value) return;
		FigText* p = static_cast<FigText*>(value.get());

		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{
			m_v3Position[0] =0.5*((p->m_v3Pos[0]+1.0) * dc.bi.b3bbox.hi[0] + (1.0 - p->m_v3Pos[0])*dc.bi.b3bbox.lo[0]) + p->m_v3Pxl[0];
			m_v3Position[1] =0.5*((p->m_v3Pos[1]+1.0) * dc.bi.b3bbox.hi[1] + (1.0 - p->m_v3Pos[1])*dc.bi.b3bbox.lo[1]) + p->m_v3Pxl[1];			
		}

		if (dc.Mode() == GLDC::RENDER_TEXT||dc.Mode() == GLDC::RENDER_SELECT)
		{
			dc.SetFont(p->m_tFont);
			dc.PrintText(p->m_sText, m_v3Position, p->m_v3Shf);
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


	static DataModelSymbol* CreateDataModel2()
	{

		DataModelSymbol* model = new DataModelSymbol();

		DataNodeCreator::Register<FigFigure>();
		DataNodeCreator::Register<FigCoord2D>();
		DataNodeCreator::Register<FigData2D>();
		DataNodeCreator::Register<FigCoord3D>();
		DataNodeCreator::Register<FigData3D>();
		DataNodeCreator::Register<FigDataManager>();
		DataNodeCreator::Register<FigAxisD>();
		DataNodeCreator::Register<AxisUnitD>();
		DataNodeCreator::Register<FigText>();

		model->AddColumn(new DataColumnName);
		model->AddColumn(new DataColumnType);

		FigFigure *p = new FigFigure;
		p->m_sId = "figure";

		FigCoord* c = new FigCoord3D;
		c->m_sId = "coord";

		FigData* d = new FigData3D;
		d->m_sId = "data1";

		c->m_pDataManager->m_aItems.append(d);		

		p->m_pItem.reset(c);

		model->Update(p);

		return model;
	}

	static DataModelSymbol* CreateDataModel1()
	{

		DataModelSymbol* model = new DataModelSymbol();

		DataNodeCreator::Register<FigFigure>();
		DataNodeCreator::Register<FigCoord2D>();
		DataNodeCreator::Register<FigData2D>();
		DataNodeCreator::Register<FigDataManager>();
		DataNodeCreator::Register<FigAxisD>();
		DataNodeCreator::Register<AxisUnitD>();
		DataNodeCreator::Register<FigText>();

		model->AddColumn(new DataColumnName);
		model->AddColumn(new DataColumnType);

		FigFigure *p = new FigFigure;
		p->m_sId = "figure";

		FigCoord* c = new FigCoord2D;
		c->m_sId = "coord";

		FigData* d = new FigData2D;
		d->m_sId = "data1";

		c->m_pDataManager->m_aItems.append(d);

		d = new FigData2D;
		d->m_sId = "data2";
		c->m_pDataManager->m_aItems.append(d);

		FigText* t = new FigText;
		t->m_sText = "title";
		t->m_sId = "title";
		t->m_v3Pos[1] = 1.0;
		t->m_v3Shf[1] = 1.0;
		t->m_v3Pxl[1] = 3.0;
		c->m_aItems.append(t);

		t = new FigText;
		t->m_sText = "xlabel";
		t->m_sId = "xlabel";
		t->m_v3Pos[1] = -1.0;
		t->m_v3Shf[1] = -1.0;
		t->m_v3Pxl[1] = -23.0;
		c->m_aItems.append(t);


		t = new FigText;
		t->m_sText = "ylabel";
		t->m_sId = "ylabel";
		t->m_v3Pos[0] = -1.0;
		t->m_v3Shf[0] = -1.0;
		t->m_v3Pxl[0] = -23.0;
		t->m_tFont.flags.add(DFontStyle::STYLE_VERTICAL);
		c->m_aItems.append(t);

		p->m_pItem.reset(c);

		model->Update(p);

		return model;
	}

	MvcViewFigure(MvcModel& tar) :basetype(tar)
	{
		static int g_n_figure = 0;
		pmodel = ++g_n_figure % 2 ? CreateDataModel1() : CreateDataModel2();

		first = true;

	}

	DataModelSymbol* pmodel;
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
		//m_pCanvas->SetBackgroundStyle(wxBG_STYLE_CUSTOM);
		return m_pCanvas;
	}

};


DataPtrT<MvcModel> PluginFigureViewer::CreateSampleModel()
{
	return new MvcModelT<MvcViewFigure>;
}



bool PluginFigureViewer::OnAttach()
{

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


bool PluginFigureViewer::OnCfgEvent(int lv)
{


	return true;
}

PluginFigureViewer::PluginFigureViewer(WndManager& w):basetype(w,"Plugin.FigViewer")
{
	m_aExtension.insert(".ewd");

}

IMPLEMENT_IPLUGIN(PluginFigureViewer)

EW_LEAVE
