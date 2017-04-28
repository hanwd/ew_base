#include "ewc_base/wnd/impl_wx/iwnd_modelview.h"


EW_ENTER

IWnd_modelview::IWnd_modelview(wxWindow* p, const WndPropertyEx& h)
:wxWindow(p, h.id(), h, h, wxFULL_REPAINT_ON_RESIZE | wxWANTS_CHARS | wxBORDER_NONE)
{

	
	this->Connect(wxEVT_PAINT, wxPaintEventHandler(IWnd_modelview::OnPaint));
	this->Connect(wxEVT_SIZE, wxSizeEventHandler(IWnd_modelview::OnSizeEvent));
	this->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(IWnd_modelview::OnMouseEvent));
	this->Connect(wxEVT_LEFT_UP, wxMouseEventHandler(IWnd_modelview::OnMouseEvent));
	this->Connect(wxEVT_LEFT_DCLICK, wxMouseEventHandler(IWnd_modelview::OnMouseEvent));
	this->Connect(wxEVT_MIDDLE_DOWN, wxMouseEventHandler(IWnd_modelview::OnMouseEvent));
	this->Connect(wxEVT_MIDDLE_UP, wxMouseEventHandler(IWnd_modelview::OnMouseEvent));
	this->Connect(wxEVT_MIDDLE_DCLICK, wxMouseEventHandler(IWnd_modelview::OnMouseEvent));
	this->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(IWnd_modelview::OnMouseEvent));
	this->Connect(wxEVT_RIGHT_UP, wxMouseEventHandler(IWnd_modelview::OnMouseEvent));
	this->Connect(wxEVT_RIGHT_DCLICK, wxMouseEventHandler(IWnd_modelview::OnMouseEvent));
	this->Connect(wxEVT_MOTION, wxMouseEventHandler(IWnd_modelview::OnMouseEvent));
	this->Connect(wxEVT_MOUSEWHEEL, wxMouseEventHandler(IWnd_modelview::OnMouseEvent));

	SetBackgroundStyle(wxBG_STYLE_CUSTOM);

	gt.pview=this;

}


void IWnd_modelview::ImageUpdate()
{

	if (!pmodel)
	{
		return;
	}

	wxSize sz = GetClientSize();
	dc.SetCurrent(this);
	dc.Reshape(sz);
	dc.Color(DColor(255, 0, 0));
	dc.RenderModel(pmodel);

	dc.SaveBuffer("model");
}



void IWnd_modelview::OnSizeEvent(wxSizeEvent& evt)
{
	Refresh();
}

void IWnd_modelview::OnPaint(wxPaintEvent& evt)
{
	wxPaintDC wxdc(this);
	if (!pmodel)
	{
		return;
	}

	if (gt.flags.get(GLParam::IMAGE_CACHED))
	{
		dc.LoadBuffer("model",wxdc);
		if (gt.v2pos1 != gt.v2pos2)
		{
			wxdc.SetPen(*wxRED_PEN);
			wxdc.DrawLine( gt.v2pos1[0], gt.v2pos1[1], gt.v2pos2[0], gt.v2pos1[1]);
			wxdc.DrawLine( gt.v2pos2[0], gt.v2pos1[1], gt.v2pos2[0], gt.v2pos2[1]);
			wxdc.DrawLine( gt.v2pos2[0], gt.v2pos2[1], gt.v2pos1[0], gt.v2pos2[1]);
			wxdc.DrawLine( gt.v2pos1[0], gt.v2pos2[1], gt.v2pos1[0], gt.v2pos1[1]);
		}
		
	}
	else
	{
		wxSize sz = GetClientSize();
		dc.SetCurrent(this);
		dc.Reshape(sz);
		dc.Color(DColor(255, 0, 0));
		dc.RenderModel(pmodel);
		dc.SwapBuffers();
	}


}

void IWnd_modelview::OnMouseEvent(wxMouseEvent& evt)
{
	if (evt.LeftDown())
	{
		this->SetFocus();
	}
	gt.OnMouseEvent(evt);
}

void IWnd_modelview::OnKeyEvent(wxKeyEvent& evt)
{

}

template<>
class ValidatorW<IWnd_modelview> : public ValidatorGroup
{
public:
	LitePtrT<IWnd_modelview> pWindow;
	ValidatorW(IWnd_modelview* w) :pWindow(w){}
};


template<>
class WndInfoT<IWnd_modelview> : public WndInfoBaseT<IWnd_modelview>
{
public:

	WndInfoT(const String& s) :WndInfoBaseT<IWnd_modelview>(s)
	{

	}

	Validator* CreateValidator(wxWindow* w)
	{
		return new ValidatorW<IWnd_modelview>((IWnd_modelview*)w);
	}

	void Update(WndMaker& wk, wxWindow* pw, EvtBase* pv)
	{
		wk.vd_set(CreateValidatorEx(pw, pv));
	}

};

template<>
void WndInfoManger_Register<IWnd_modelview>(WndInfoManger& imgr, const String& name)
{
	static WndInfoT<IWnd_modelview> info(name);
	imgr.Register(&info);
}

EW_LEAVE
