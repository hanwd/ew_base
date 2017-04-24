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
			dc.RenderNode(subnodes[i]);
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

template<>
class DataNodeSymbolT<FigCoord2D> : public DataNodeSymbolT<FigCoord>
{
public:

	typedef DataNodeSymbolT<FigCoord> basetype;

	DataNodeSymbolT(DataNode* n, CallableSymbol* p) :basetype(n, p)
	{

	}

	GLDC::BBoxInfo bi;

	vec4d plane[6];

	void DoRender(GLDC& dc)
	{
		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{
			dc.bi.b3axis.set_x(-100.0, +100.0);
			dc.bi.b3axis.set_y(-100.0, +100.0);
			dc.bi.b3axis.set_z(-1.0, +1.0);

			dc.bi.b3bbox.lo += 70;
			dc.bi.b3bbox.hi -= 45;
			bi = dc.bi;

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
			int gl_type = dc.Mode() == GLDC::RENDER_SELECT ? GL_QUADS : GL_LINE_LOOP;

			glBegin(gl_type);
				glVertex2d(bi.b3bbox.lo[0], bi.b3bbox.lo[1]);
				glVertex2d(bi.b3bbox.hi[0], bi.b3bbox.lo[1]);
				glVertex2d(bi.b3bbox.hi[0], bi.b3bbox.hi[1]);
				glVertex2d(bi.b3bbox.lo[0], bi.b3bbox.hi[1]);
			glEnd();


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

		String text;
		long k1=::floor(dmin/br);
		long k2=::ceil(dmax/br);
		long dk=(k2-k1);
		if(dk<=0) dk=1;

		//int tmax=1;
	
		std::vector<long> vectk;
		vectk.push_back(k1-1);
		vectk.push_back(k1+0);
		vectk.push_back(k1+1);
		vectk.push_back(k2-1);
		vectk.push_back(k2+0);
		vectk.push_back(k2+1);

		for(int i=0;i<(int)vectk.size();i++)
		{
			long k=vectk[i];
			text.Printf("%g",br*double(k));
			double v1ts = DoGetTextExtend(dc, text);
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
				text.Printf("%g",brx*double(k));
				double v1ts = DoGetTextExtend(dc, text);
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

		long y1=floor(dmin/br);
		long y2=ceil(dmax/br);

		AxisUnitD::Tick tk;
		tk.flags.add(AxisUnitD::Tick::TICK_MAIN);
		for(long y=y1; y<=y2; y++)
		{
			double d=br*double(y);
			if(d>=dmin&&d<=dmax)
			{
				tk.m_nValue=d;
				tk.m_sLabel.Printf("%g",d);
				ticks.push_back(tk);
			}
		}

		tk.flags.del(AxisUnitD::Tick::TICK_MAIN);

		double mm=15.0;

		long tm=(long)(bw/mm);

		for(long y=y1; y<=y2; y++)
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
							tk.m_nValue=d;
							tk.m_sLabel.Printf("%g",d);
							ticks.push_back(tk);
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
							tk.m_nValue=d;
							tk.m_sLabel.Printf("%g",d);
							ticks.push_back(tk);
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
							tk.m_nValue=d;
							tk.m_sLabel.Printf("%g",d);
							ticks.push_back(tk);
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
							tk.m_nValue=d;
							tk.m_sLabel.Printf("%g",d);
							ticks.push_back(tk);
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

	DataPtrT<FigData2D> m_pItem;



	DataNodeSymbolT(DataNode* n, CallableSymbol* p) :basetype(n, p)
	{
		if (!m_pItem.cast_and_set(p)) return;
	}

	box3d b3bbox;
	mat4d m4data;

	arr_1t<vec3d> cached_data;

	virtual void DoUpdateCachedData(GLDC& dc)
	{
		bool data_updated = false;

		arr_1t<double> &adata(m_pItem->m_aTdata);
		arr_1t<double> &bdata(m_pItem->m_aValue);

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
			else if (m_pItem->m_nDataType == FigData2D::TYPE_POLAR)
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

			data_updated = true;
		}

		if (data_updated || b3bbox != dc.bi.b3bbox)
		{
			b3bbox = dc.bi.b3bbox;
			valid_index.clear();
			valid_index.push_back(0);
			valid_index.push_back(n);
		}
	}



	void DoRender(GLDC& dc)
	{
		if (!m_pItem) return;

		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{
			DoUpdateCachedData(dc);
		}
		else if (dc.Mode() == GLDC::RENDER_SOLID||dc.Mode() == GLDC::RENDER_SELECT)
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

	GLDC dc;

	bool first;

	~MvcViewFigure()
	{
		model->DecRef();
	}

	IWnd_bookbase* m_pBook;
	AutoPtrT<wxWindow> m_pMyView;

	bool OnCreated()
	{
		m_pBook = dynamic_cast<IWnd_bookbase*>(WndManager::current().evtmgr["Wnd.Variable"].GetWindow());
		if (m_pBook)
		{
			m_pMyView.reset(model->CreateDataView(m_pBook));
		}
		return true;
	}

	MvcViewFigure(MvcModel& tar):basetype(tar)
	{
		this->Connect(wxEVT_PAINT, wxPaintEventHandler(MvcViewFigure::OnPaint));
		this->Connect(wxEVT_SIZE, wxSizeEventHandler(MvcViewFigure::OnSizeEvent));
		this->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(MvcViewFigure::OnMouseEvent));

		model = new DataModelSymbol();

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

		first = true;

	}

	DataModelSymbol* model;
	AtomicSpin spin;

	void OnSizeEvent(wxSizeEvent&)
	{
		Refresh();
	}

	void OnMouseEvent(wxMouseEvent& evt)
	{

		dc.SetCurrent(m_pCanvas);
		dc.Reshape(m_pCanvas->GetClientSize());
		dc.RenderSelect(model);

		DataNode* p=dc.HitTest(evt.GetX(), m_pCanvas->GetClientSize().y - evt.GetY());

		if (p)
		{
			this_logger().LogMessage("selected node: %s", p->name);
		}
	}

	void OnPaint(wxPaintEvent&)
	{
		wxSize sz = m_pCanvas->GetClientSize();
		wxPaintDC wxdc(m_pCanvas);
		dc.SetCurrent(m_pCanvas);
		dc.Reshape(sz);
		dc.Color(DColor(255, 0, 0));
		dc.RenderModel(model);
	}

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


	LitePtrT<wxWindow> m_pCanvas;

	void OnDestroy()
	{
		basetype::OnDestroy();
	}

	wxWindow* CreateCanvas(wxWindow* w)
	{
		fn.SetExts(_hT("Text Files")+"(*.txt) | *.txt");
		m_pCanvas.reset(new wxWindow(w,-1));
		m_pCanvas->SetBackgroundStyle(wxBG_STYLE_CUSTOM);
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
