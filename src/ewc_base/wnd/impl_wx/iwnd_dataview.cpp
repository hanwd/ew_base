#include "ewc_base/wnd/impl_wx/iwnd_dataview.h"


EW_ENTER


void IValueColumn::var2val(wxVariant& variant,String& v) const
{
	v = WxImpl<String>::get(variant);
}

void IValueColumn::var2val(wxVariant& variant,float& v) const
{
	double d;
	var2val(variant, d);
	v = d;
}

void IValueColumn::var2val(wxVariant& variant,double& v) const
{
	String s = WxImpl<String>::get(variant);
	char ch = s.c_str()[0];
	if ((ch >= '0'&&ch <= '9') || ch=='.'||ch=='-'||ch=='+')
	{
		v = variant.GetDouble();
	}
	else if (s == "NAN")
	{
		v = _Nan._Double;
	}
	else if (s == "INF")
	{
		v = _Inf._Double;
	}
	else
	{
		v = 0.0;
	}
	
}

void IValueColumn::var2val(wxVariant& variant,int64_t& v) const
{
	v=variant.GetLong();
}

void IValueColumn::var2val(wxVariant& variant,int32_t& v) const
{
	v=variant.GetLong();
}

void IValueColumn::var2val(wxVariant& variant,bool& v) const
{
	v=variant.GetBool();
}

void IValueColumn::var2val(wxVariant& variant, Variant& v) const
{
	v.reset<String>((const char*)variant.GetString().c_str());
}

void IValueColumn::val2var(wxVariant& variant,const String& v) const
{
	variant=v.c_str();
}

void IValueColumn::val2var(wxVariant& variant,float v) const
{
	val2var(variant, double(v));
}

void IValueColumn::val2var(wxVariant& variant,double v) const
{
	if (std::isnan(v))
	{
		variant = "NAN";
	}
	else if (ndot < 0)
	{
		val2var(variant, String::Format("%f", v));
	}
	else
	{
		val2var(variant, String::Format("%.*f", ndot, v));
	}
}

void IValueColumn::val2var(wxVariant& variant,int64_t v) const
{
	variant=(long)v;
}

void IValueColumn::val2var(wxVariant& variant,int32_t v) const
{
	variant=(long)v;
}

void IValueColumn::val2var(wxVariant& variant,bool v) const
{
	variant=(bool)v;
}	

void IValueColumn::val2var(wxVariant& variant, Variant& v) const
{
	variant = variant_cast<String>(v).c_str();
}



class mvcListRender : public wxDataViewTextRenderer
{
public:
	mvcListRender(const wxString &varianttype = wxT("string"),
		wxDataViewCellMode mode = wxDATAVIEW_CELL_EDITABLE,
		int align = wxDVR_DEFAULT_ALIGNMENT,
		bool selectall=false
		);


	virtual bool WXActivateCell(const wxRect& cell,
                              wxDataViewModel *model,
                              const wxDataViewItem & item,
                              unsigned int col,
                              const wxMouseEvent* mouseEvent
		);

protected:
	bool bSelectAll;

};

class mvcExprRender : public mvcListRender
{
public:
	typedef mvcListRender basetype;

	mvcExprRender(const wxString &varianttype = wxT("string"),
		wxDataViewCellMode mode = wxDATAVIEW_CELL_EDITABLE,
		int align = wxDVR_DEFAULT_ALIGNMENT,
		bool selectall=false
		):mvcListRender(varianttype,mode,align,selectall){}


	virtual bool WXActivateCell(const wxRect& cell,
                              wxDataViewModel *model,
                              const wxDataViewItem & item,
                              unsigned int col,
                              const wxMouseEvent* mouseEvent
		);


};



mvcListRender::mvcListRender(const wxString &varianttype,
	wxDataViewCellMode mode,
	int align,
	bool selectall
	)
	:wxDataViewTextRenderer(varianttype,mode,align)
{
	bSelectAll=selectall;
}

bool mvcListRender::WXActivateCell(const wxRect& cell,
                              wxDataViewModel *model,
                              const wxDataViewItem & item,
                              unsigned int col,
                              const wxMouseEvent* mouseEvent
		)
{
	(void)model;
	(void)col;
	
	if(mouseEvent==NULL)
	{
		return false;
	}

	if(!wxDataViewTextRenderer::StartEditing(item,cell)) return false;

	if(!bSelectAll)
	{
		return true;
	}

	wxTextCtrl* ctrl = dynamic_cast<wxTextCtrl*>(GetEditorCtrl());
	if (ctrl)
	{
		ctrl->SetInsertionPointEnd();
		ctrl->SelectAll();
	}

	return true;
}



bool mvcExprRender::WXActivateCell(const wxRect& ,
                              wxDataViewModel *,
                              const wxDataViewItem &,
                              unsigned int ,
                              const wxMouseEvent* 
		)
{


	return false;
}


class dmCheckboxRender : public wxDataViewCustomRenderer
{
public:

	dmCheckboxRender(const wxString &varianttype = wxT("long"),
		wxDataViewCellMode mode = wxDATAVIEW_CELL_INERT,
		int align = wxDVR_DEFAULT_ALIGNMENT)
		:wxDataViewCustomRenderer(varianttype,mode,align)
	{

	}

	bool SetValue( const wxVariant &value )
	{
		val=value.GetLong();
		return true;
	}

	bool GetValue( wxVariant &value ) const
	{
		value=val;
		return true;
	}

	bool Render( wxRect cell, wxDC *dc, int  )
	{
		int flags = 0;

		switch(val)
		{
		case 0: break;
		case 2:	flags|=wxCONTROL_DISABLED; break;
		case 3: flags|=wxCONTROL_DISABLED;
		case 1: flags|=wxCONTROL_CHECKED; break;
		default: return true;
		};

		//if (GetMode() != wxDATAVIEW_CELL_ACTIVATABLE)
		//	flags |= wxCONTROL_DISABLED;

		wxRendererNative::Get().DrawCheckBox(
			GetOwner()->GetOwner(),
			*dc,
			cell,
			flags );

		return true;
	}

	virtual bool LeftClick( wxPoint WXUNUSED(cursor),
		wxRect ,
		wxDataViewModel *model,
		const wxDataViewItem & item,
		unsigned int col )
	{
		long value=1;

		switch(val)
		{
		case 0: value=1; break;
		case 1: value=0; break;
		default: return true;
		}

		wxVariant variant = value;
		model->SetValue( variant, item, col);
		//model->ValueChanged( item, col );
#ifdef EW_WINDOWS
		this->GetOwner()->GetOwner()->OnColumnChange(col);
#endif
		//model->Resort();
		return true;
	}

	wxSize GetSize() const
	{
		return wxRendererNative::Get().GetCheckBoxSize(NULL);
	}

private:
	long val; //0 invisible 1 visible  other NA

};

wxDataViewColumn* IValueColumn::CreateDataViewColumn(IValueColumn& info,size_t n)
{

	wxDataViewRenderer* render=NULL;

	int _nAlign=wxALIGN_LEFT;//wxDVR_DEFAULT_ALIGNMENT;
	if(info.type==0)
	{
		render=new mvcListRender("string",wxDATAVIEW_CELL_INERT,_nAlign);
	}		
	else if(info.type==COLUMNTYPE_CHECKBOX)
	{
		wxDataViewCellMode flag=info.IsReadonly()?wxDATAVIEW_CELL_INERT:wxDATAVIEW_CELL_ACTIVATABLE;
		render=new dmCheckboxRender ( "bool", flag );
	}
	else if(info.type==COLUMNTYPE_ICON)
	{
		render=new wxDataViewBitmapRenderer();
	}
	else
	{
		wxDataViewCellMode flag=info.IsReadonly()?wxDATAVIEW_CELL_INERT:wxDATAVIEW_CELL_EDITABLE;
		render=new mvcListRender("string",flag,_nAlign);
	}	

	wxDataViewColumn *column = new wxDataViewColumn(info.name.c_str(),render,n,info.width);

	return column;
}

wxDataViewColumn* IValueDataModel::CreateColumn(size_t n)
{
	if(!m_pData) return NULL;
	if(n>=m_pData->aColumnInfo.size()) return NULL;

	IValueColumn& info(*m_pData->aColumnInfo[n]);

	return IValueColumn::CreateDataViewColumn(info,n);
}






BEGIN_EVENT_TABLE(IWnd_dataview,wxDataViewCtrl)
EVT_SIZE(IWnd_dataview::OnSize)
//EVT_MENU(ID_MU_DELETE,IWnd_dataview::OnDeleteItem)
EVT_MENU_RANGE(ID_MU_BEGIN,ID_MU_END,IWnd_dataview::OnCommandEvents)
EVT_DATAVIEW_COLUMN_HEADER_CLICK(wxID_ANY,IWnd_dataview::OnColumnHeaderClick)
EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, IWnd_dataview::OnContextMenu)
EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, IWnd_dataview::OnActivated )
EVT_DATAVIEW_ITEM_START_EDITING(wxID_ANY,IWnd_dataview::OnItemEditingStarting)
EVT_DATAVIEW_ITEM_EDITING_STARTED(wxID_ANY,IWnd_dataview::OnItemEditingStarted)
EVT_DATAVIEW_ITEM_EDITING_DONE(wxID_ANY,IWnd_dataview::OnItemEditingDone)
EVT_DATAVIEW_ITEM_VALUE_CHANGED(wxID_ANY,IWnd_dataview::OnItemValueChanged)
EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY,IWnd_dataview::OnSelectionChanged)
END_EVENT_TABLE()


bool IWnd_dataview::FinishByReturnKey;

class tdEnterDetector : public wxWindow
{
public:

	static bool flag;

	void OnChar(wxKeyEvent& evt)
	{
		if(evt.GetKeyCode()==WXK_RETURN)
		{
			IWnd_dataview::FinishByReturnKey=true;
		}
		evt.Skip();
	}

	void OnFocus(wxFocusEvent& evt)
	{

	}

	void OnLostFocus(wxFocusEvent& evt)
	{

	}

	~tdEnterDetector()
	{

	}


};


IWnd_dataview::IWnd_dataview(wxWindow* p,const WndPropertyEx& h)
:wxDataViewCtrl(p,h.id(),h,h,wxDV_MULTIPLE|wxDV_ROW_LINES|(h.flags().get(IDefs::IWND_NOHEADER)?wxDV_NO_HEADER:0))
{
	model=new IValueDataModel;

	AssociateModel(model);
	//GetMainWindow()->PushEventHandler(new tdEnterDetector);

	GetMainWindow()->Connect(wxEVT_CHAR_HOOK, wxCharEventHandler(tdEnterDetector::OnChar));
}

void IWnd_dataview::ReCreateColumns()
{
	model->Reset(0);
	size_t n=GetColumnCount();
	for(size_t i=0;i<n;i++)
	{
		DeleteColumn(GetColumn(n-i-1));
	}

	for(size_t i=0;i<model->GetColumnCount();i++)
	{
		AppendColumn(model->CreateColumn(i));
	}

	model->Reset(model->GetRowCount());
}

bool IWnd_dataview::ValueChanged(int row,int col)
{
	(void)&row;
	(void)&col;
	return true;
}

void IWnd_dataview::OnActivated( wxDataViewEvent &evt )
{
	this->EditItem(evt.GetItem(),evt.GetDataViewColumn());
}


IWnd_dataview::~IWnd_dataview()
{
	model->DecRef();
}



void IWnd_dataview::OnSize(wxSizeEvent &evt)
{
	evt.Skip();
}


void IWnd_dataview::OnSelectionChanged(wxDataViewEvent &evt)
{
	wxDataViewItemArray arr;
	int n=this->GetSelections(arr);
	if(n>1)
	{
		int r=(int)model->GetRowCount()-1;
		for(int i=0;i<n;i++)
		{
			if(model->GetRow(arr[i])==r)
			{
				this->SetSelections(m_aLastSelection);
				return;
			}
		}
	}
	m_aLastSelection=arr;
}

void IWnd_dataview::OnContextMenu( wxDataViewEvent &evt )
{
	if(this->m_aLastSelection.size()==1)
	{
		wxDataViewItem item = evt.GetItem();
		int r=(int)model->GetRowCount()-1;

		int row=model->GetRow(item);
		if (row<r)
		{
			wxMenu vec_menu(_("ContexMenu"));
			vec_menu.Append(ID_MU_DELETE, _("Delete"));
			if(row>0)
			{
				vec_menu.Append(ID_MU_MOVE_UP, _("MoveUp"));
				vec_menu.Append(ID_MU_MOVE_TOP, _("MoveTop"));
			}
			if(row+1<r)
			{
				vec_menu.Append(ID_MU_MOVE_DOWN, _("MoveDown"));
				vec_menu.Append(ID_MU_MOVE_BOTTOM, _("MoveBottom"));
			}
			this->PopupMenu(&vec_menu);
		}
		else if(row==r)
		{
			wxMenu vec_menu(_("ContexMenu"));
			vec_menu.Append(ID_MU_NEW_ITEM, _("NewItem"));
			this->PopupMenu(&vec_menu);
		}
	}
	else
	{
		wxMenu vec_menu(_("ContexMenu"));
		vec_menu.Append(ID_MU_DELETE, _("Delete"));
		this->PopupMenu(&vec_menu);
	}

}

void IWnd_dataview::OnCommandEvents(wxCommandEvent& evt)
{
	int evtid=evt.GetId();
	if(evtid==ID_MU_DELETE)
	{
		model->DeleteItems(m_aLastSelection);
		m_aLastSelection.clear();
		return;
	}
	
	if(evtid==ID_MU_NEW_ITEM)
	{
		int row=model->GetRowCount()-1;
		int col=model->GetColNewItem();
		if(row>=0&&col>=0)
		{
			EditItem(model->GetItem(row),GetColumn(col));
		}
		return;
	}

	if(m_aLastSelection.size()!=1)
	{
		return;
	}

	int row=model->GetRow(m_aLastSelection[0]);
	if(evtid==ID_MU_MOVE_UP)
	{
		if(model->MoveItem(row,row-1))
		{
			ValueChanged(-1,-1);	
		}
	}
	else if(evtid==ID_MU_MOVE_DOWN)
	{
		if(model->MoveItem(row,row+1))
		{
			ValueChanged(-1,-1);	
		}
	}
	else if(evtid==ID_MU_MOVE_TOP)
	{

		if(model->MoveItem(row,0))
		{
			ValueChanged(-1,-1);	
		}
	}
	else if(evtid==ID_MU_MOVE_BOTTOM)
	{
		if(model->MoveItem(row,(int)model->GetRowCount()-2))
		{
			ValueChanged(-1,-1);	
		}
	}


}

void IWnd_dataview::OnColumnHeaderClick(wxDataViewEvent &evt)
{
	int col = evt.GetDataViewColumn()->GetModelColumn();
	if (model->m_pData->OnColumnHeaderClick(col))
	{
		model->Reset(model->GetRowCount());
	}
}

void IWnd_dataview::OnItemEditingStarting(wxDataViewEvent &evt)
{

	int row=this->GetRowByItem(evt.GetItem());
	int col=evt.GetDataViewColumn()->GetModelColumn();
	if(!model->TestEdit(row,col))
	{
		evt.Veto();
	}
}


void IWnd_dataview::OnItemEditingStarted(wxDataViewEvent &)
{
	FinishByReturnKey=false;
}

static wxString editing_val;

void IWnd_dataview::OnItemEditingDone(wxDataViewEvent &evt)
{
	editing_val=evt.GetValue().GetString();
}

void IWnd_dataview::OnItemValueChanged(wxDataViewEvent &evt)
{
	int row=GetRowByItem(evt.GetItem());
	int col=evt.GetColumn();

	if(!ValueChanged(row,col))
	{
		wxDataViewColumn* dvcol=GetColumn(col);
		if(!dvcol) return;

		EditItem(model->GetItem(row),dvcol);
		wxWindow *editctrl=dvcol->GetRenderer()->GetEditorCtrl();
		wxTextCtrl *ctrl=dynamic_cast<wxTextCtrl *>(editctrl);
		if(ctrl)
		{
			ctrl->SetValue(editing_val);
			ctrl->SetInsertionPointEnd();
			ctrl->SelectAll();
		}
		return;
	}

	if(FinishByReturnKey)
	{
		int row=GetRowByItem(evt.GetItem());
		int col=evt.GetColumn();
		if(model->NextItem(row,col))
		{
			EditItem(model->GetItem(row),GetColumn(col));
		}
	}
}


template<>
class ValidatorW<IWnd_dataview> : public Validator
{
public:
	LitePtrT<IWnd_dataview> pWindow;
	ValidatorW(IWnd_dataview* w):pWindow(w)
	{
		pWindow->m_pVald.reset(this);		
	}
};


class ValidatorW2 : public ValidatorW<IWnd_dataview>
{
public:
	typedef ValidatorW<IWnd_dataview> basetype;
	ValidatorW2(IWnd_dataview* w,EvtProxyArray* p):basetype(w),proxy(p)
	{
		pWindow->GetModel()->m_pData.reset(p);
		proxy->AttachListener(this);

		WndExecuteEx(IDefs::ACTION_TRANSFER2WINDOW);
	}

	DataPtrT<EvtProxyArray> proxy;

	virtual bool OnWndEvent(IWndParam& cmd,int phase)
	{
		if(cmd.action==IDefs::ACTION_TRANSFER2WINDOW && phase>0)
		{
			pWindow->ReCreateColumns();
			return true;
		}
		else if(cmd.action==IDefs::ACTION_UPDATECTRL)
		{
			pWindow->ReCreateColumns();
			return true;
		}
		return true;
	}


	virtual bool WndExecute(IWndParam& cmd)
	{
		cmd.iwvptr=this;

		if(cmd.action==IDefs::ACTION_VALUE_CHANGING)
		{
			return proxy->WndExecute(cmd);		
		}
		else if(cmd.action==IDefs::ACTION_VALUE_CHANGED)
		{
			return proxy->WndExecute(cmd);				
		}
		else if(cmd.action==IDefs::ACTION_TRANSFER2WINDOW)
		{
			return proxy->WndExecute(cmd);
		}
		else if(cmd.action==IDefs::ACTION_TRANSFER2MODEL)
		{
			return proxy->WndExecute(cmd);	
		}
		else
		{
			return basetype::WndExecute(cmd);
		}
	}

};

template<>
class WndInfoT<IWnd_dataview> : public WndInfoBaseT<IWnd_dataview> 
{
public:

	WndInfoT(const String& s):WndInfoBaseT<IWnd_dataview>(s)
	{

	}

	virtual Validator* CreateValidator(wxWindow* w,EvtProxyArray* p)
	{
		return new ValidatorW2((IWnd_dataview*)w,p);
	}

	Validator* CreateValidator(wxWindow* w)
	{
		return new ValidatorW<IWnd_dataview>((IWnd_dataview*)w);
	}

};


template<>
void WndInfoManger_Register<IWnd_dataview>(WndInfoManger& imgr,const String& name)
{
	static WndInfoT<IWnd_dataview> info(name);
	imgr.Register(&info);
}

EW_LEAVE

