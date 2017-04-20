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

#include "ewa_base/figure/fig.h"
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
			return;
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

	box3d bbox;

	vec4d plane[6];

	void DoRender(GLDC& dc)
	{
		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{
			dc.sz.b3axis.set_x(-100.0, +100.0);
			dc.sz.b3axis.set_y(-100.0, +100.0);
			dc.sz.b3axis.set_z(-1.0, +1.0);

			dc.sz.b3bbox.lo += 10;
			dc.sz.b3bbox.hi -= 10;
			bbox = dc.sz.b3bbox;

			plane[0].set3(+1, 0, 0); plane[0][3] = -dc.sz.b3bbox.lo[0];
			plane[1].set3(-1, 0, 0); plane[1][3] = +dc.sz.b3bbox.hi[0];
			plane[2].set3(0, +1, 0); plane[2][3] = -dc.sz.b3bbox.lo[1];
			plane[3].set3(0, -1, 0); plane[3][3] = +dc.sz.b3bbox.hi[1];

			vec3d v3p(dc.sz.b3bbox.lo);
			vec3d v3w(dc.sz.b3bbox.width());

			dc.sz.m4data.LoadIdentity();
			dc.sz.m4data.Translate(v3p);
			dc.sz.m4data.Scale(v3w);
			dc.sz.m4data.Scale(1.0 / dc.sz.b3axis.width());
			dc.sz.m4data.Translate(-dc.sz.b3axis.lo);

			for (size_t i = 0; i < subnodes.size(); i++)
			{
				subnodes[i]->DoRender(dc);
			}
		}
		else if (dc.Mode() == GLDC::RENDER_SOLID || dc.Mode() == GLDC::RENDER_SELECT)
		{
			int gl_type = dc.Mode() == GLDC::RENDER_SELECT ? GL_QUADS : GL_LINE_LOOP;

			glBegin(gl_type);
				glVertex2d(bbox.lo[0], bbox.lo[1]);
				glVertex2d(bbox.hi[0], bbox.lo[1]);
				glVertex2d(bbox.hi[0], bbox.hi[1]);
				glVertex2d(bbox.lo[0], bbox.hi[1]);
			glEnd();

			for (int i = 0; i < 4; i++)
			{
				::glClipPlane(GL_CLIP_PLANE0 + i, plane[i].data());
				::glEnable(GL_CLIP_PLANE0 + i);
			}


			for (size_t i = 0; i < subnodes.size(); i++)
			{
				dc.RenderNode(subnodes[i]);
			}

			for (int i = 0; i < 4; i++)
			{
				::glDisable(GL_CLIP_PLANE0 + i);
			}
		}


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

	box3d bbox;
	mat4d m4;

	arr_1t<vec3d> cached_data;


	void DoRender(GLDC& dc)
	{
		if (!m_pItem) return;

		if (dc.Mode() == GLDC::RENDER_SET_REALSIZE)
		{
			bool data_updated = false;
			size_t n = m_pItem->m_aValue.size();

			if (m4 != dc.sz.m4data || cached_data.size() != m_pItem->m_aValue.size())
			{

				cached_data.resize(n);
				m4 = dc.sz.m4data;

				if (m_pItem->m_aTdata.size() == n)
				{
					if (m_pItem->m_nDataType == FigData2D::TYPE_POLAR)
					{
						for (size_t i = 0; i < n; i++)
						{
							double r = m_pItem->m_aValue[i];
							double a = m_pItem->m_aTdata[i] * M_PI / 180.0;
							cached_data[i] = m4*vec3d(r*::cos(a), r*::sin(a), 0);
						}
					}
					else
					{
						for (size_t i = 0; i < n; i++)
						{
							cached_data[i] = m4*vec3d(m_pItem->m_aTdata[i], m_pItem->m_aValue[i], 0);
						}
					}

				}
				else
				{
					for (size_t i = 0; i < n; i++)
					{
						cached_data[i] = m4*vec3d(i, m_pItem->m_aValue[i], 0);
					}
				}

				data_updated = true;
			}

			if (data_updated || bbox != dc.sz.b3bbox)
			{
				bbox = dc.sz.b3bbox;
				valid_index.clear();
				valid_index.push_back(0);
				valid_index.push_back(n);
			}
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






class MvcViewFigure;

class RotateTask : public ITask
{
public:
	MvcViewFigure* pview;
	AtomicSpin mutex;

	RotateTask() :pview(NULL){}

	void reset(MvcViewFigure* p)
	{
		LockGuard<AtomicSpin> lock(mutex);
		pview = p;
	}

	virtual void svc(ITimerHolder& th);
};


class IFigureRotateHolder : public ITimerHolder
{
public:
	DataPtrT<RotateTask> ptask;

	void reset(MvcViewFigure *pview)
	{
		if (!ptask)
		{
			ptask.reset(new RotateTask);			
			(*(ITimerHolder*)this) = TimerQueue::current().putq(ptask.get(), TimeSpan::MilliSeconds(100));
		}

		ptask->reset(pview);
		if (pview)
		{
			redue(TimeSpan::MilliSeconds(10));
		}
		
	}

	~IFigureRotateHolder()
	{
		TimerQueue& tque(TimerQueue::current());
		ptask.reset(NULL);

	}
};


class MvcViewFigure : public MvcViewEx
{
public:
	typedef MvcViewEx basetype;

	IFigureRotateHolder th;
	GLDC dc;
	wxSize m_v2Size;
	float rt_zf;

	bool first;



	~MvcViewFigure()
	{
		model->DecRef();
	}

	void OnSize(wxSizeEvent& evt)
	{
		this->Refresh();
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
		this->Connect(wxEVT_SIZE, wxSizeEventHandler(MvcViewFigure::OnSize));
		this->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(MvcViewFigure::OnMouseEvent));

		rt_zf = 0;
		first = true;

		model = new DataModelSymbol();

		DataNodeCreator::Register<FigFigure>();
		DataNodeCreator::Register<FigCoord2D>();
		DataNodeCreator::Register<FigData2D>();

		model->AddColumn(new DataColumnName);
		model->AddColumn(new DataColumnType);

		FigFigure *p = new FigFigure;
		p->m_sId = "figure";

		FigCoord* c = new FigCoord2D;
		c->m_sId = "coord";

		FigData* d = new FigData2D;
		d->m_sId = "data1";

		c->m_aItems.append(d);
		d = new FigData2D;
		d->m_sId = "data2";
		c->m_aItems.append(d);

		p->m_pItem.reset(c);

		model->Update(p);

	}

	DataModelSymbol* model;

	AtomicSpin spin;

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
		wxPaintDC wxdc(m_pCanvas);
		dc.SetCurrent(m_pCanvas);
		dc.Reshape(m_pCanvas->GetClientSize());
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

		if (v < 0)
		{
			th.reset(NULL);
		}
		else
		{
			m_nRequestFresh = 0;
			th.reset(this);
		}

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


void RotateTask::svc(ITimerHolder& th)
{
	LockGuard<AtomicSpin> lock(mutex);
	if (!pview) return;

	if (pview->IsActive())
	{
		pview->rt_zf += 5.0f;
		//pview->Refresh();
	}

	th.redue(TimeSpan::MilliSeconds(50));
}

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
