#include "ewc_base/data/data_model.h"
#include "ewc_base/wnd/impl_wx/iwnd_modelview.h"

EW_ENTER

class DisableKillFoucsHandler : public wxEvtHandler
{
public:
	DisableKillFoucsHandler()
	{
		Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(DisableKillFoucsHandler::OnKillFocus));
	}
	void OnKillFocus(wxFocusEvent&){}	
};



class DataCanvas : public wxDataViewCtrl
{
public:

	DisableKillFoucsHandler handler;

	DataModel& Target;
	DataCanvas(wxWindow* w,DataModel& t)
		:wxDataViewCtrl(w, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES | wxDV_VERT_RULES | wxDV_MULTIPLE)
		,Target(t)
	{
		AssociateModel(&Target);
		for(size_t i=0;i<Target.GetColumnCount();i++)
		{
			AppendColumn(IValueColumn::CreateDataViewColumn(*Target.m_aColumnInfo[i],i));
		}

		Target.m_aView.insert(this);

		// disable kill_focus_event
		if (m_targetWindow)
		{
			m_targetWindow->PushEventHandler(&handler);
		}

		this->SetFocus();

	}

	bool DoUpdateDataNode(wxDataViewItem item)
	{

		node=(DataNode*)item.GetID();
		return node != NULL;
	}


	DataNode* node;

	void OnContextMenu(wxDataViewEvent &evt)
	{		
		if (!DoUpdateDataNode(evt.GetItem()))
		{
			return;
		}

		DataModel::ms_pActive = &Target;
		node->OnItemMenu(this);
	}

	void OnItemActivated(wxDataViewEvent &evt)
	{

		if (!DoUpdateDataNode(evt.GetItem()))
		{
			return;
		}

		DataModel::ms_pActive = &Target;
		if (node->OnActivate())
		{
			return;
		}

		//if (node->IsContainer())
		//{
		//	if (IsExpanded(evt.GetItem()))
		//	{
		//		Collapse(evt.GetItem());
		//	}
		//	else
		//	{
		//		Expand(evt.GetItem());
		//	}
		//}
	}

	void OnCommandEvents(wxCommandEvent& evt)
	{

	}

	void OnItemCollapsing(wxDataViewEvent &evt)
	{
		DataNode* node=(DataNode*)evt.GetItem().GetID();
		if (node)
		{
			node->OnToggle(false);
		}
	}

	void OnItemExpanding(wxDataViewEvent &evt)
	{
		DataNode* node=(DataNode*)evt.GetItem().GetID();
		if (node)
		{
			node->OnToggle(true);
		}
	}

	void OnSelectionChanged(wxDataViewEvent &evt)
	{
		wxDataViewItemArray sel;
		GetSelections(sel);

		DataSelection& selctor(Target.GetSelector());

		if (selctor.SetSelection(sel))
		{
			selctor.UpdateSelection();
		}
		else
		{
			SetSelections(selctor.GetSelections());
		}
	}

	void OnHeaderClick(wxDataViewEvent &evt)
	{
		
	}



	~DataCanvas()
	{
		if (m_targetWindow && m_targetWindow != m_targetWindow->GetEventHandler())
		{
			m_targetWindow->PopEventHandler();
		}

		Target.m_aView.erase(this);
	}

	DECLARE_EVENT_TABLE();

};

BEGIN_EVENT_TABLE(DataCanvas, wxDataViewCtrl)
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, DataCanvas::OnContextMenu)
	EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, DataCanvas::OnItemActivated)
	EVT_DATAVIEW_ITEM_COLLAPSING(wxID_ANY, DataCanvas::OnItemCollapsing)
	EVT_DATAVIEW_ITEM_EXPANDING(wxID_ANY, DataCanvas::OnItemExpanding)
	EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, DataCanvas::OnSelectionChanged)
	EVT_DATAVIEW_COLUMN_HEADER_CLICK(wxID_ANY, DataCanvas::OnHeaderClick)
	//EVT_MENU(wxID_ANY, DataCanvas::OnCommandEvents)
END_EVENT_TABLE()

wxDataViewCtrl* DataModel::CreateDataView(wxWindow* p)
{
	return new DataCanvas(p,*this);
}

DataModel::DataModel() :dpm(*this)
{
	m_pAttributeManager.reset(new DAttributeManager);
	m_pSelection.reset(new DataSelection(*this));
}

DataModel::~DataModel()
{

}
	
void DataModel::EnsuerVisible(wxDataViewItem item)
{
	for(bst_set<wxDataViewCtrl*>::iterator it=m_aView.begin();it!=m_aView.end();++it)
	{
		(*it)->EnsureVisible(item);
	}
}

void DataModel::ExpandItem(wxDataViewItem item)
{
	for(bst_set<wxDataViewCtrl*>::iterator it=m_aView.begin();it!=m_aView.end();++it)
	{
		(*it)->EnsureVisible(item);
		(*it)->Expand(item);
	}
}



bool DataModel::SetValue(const wxVariant &variant,const wxDataViewItem &item,unsigned int col)
{
	return false;
}

DataSelection& DataModel::GetSelector() const
{
	return *m_pSelection;
}

bool DataModel::HasContainerColumns(const wxDataViewItem&) const
{
	return true;
}

wxDataViewItem DataModel::GetParent( const wxDataViewItem &item ) const
{
	DataNode* node=(DataNode*)item.GetID();
	if(!node||node->parent==m_pRoot.get()) wxDataViewItem();

	return wxDataViewItem(node->parent);
}

void DataModel::GetValue( wxVariant &variant,const wxDataViewItem &item, unsigned int col ) const
{
	DataNode* node=(DataNode*)item.GetID();
	if(!node||col>=m_aColumnInfo.size()) return;
	return m_aColumnInfo[col]->GetValue(variant,node);
}

bool DataModel::IsContainer( const wxDataViewItem &item ) const
{
	DataNode* node=(DataNode*)item.GetID();
	if(!node) return true;

	if (!node->flags.get(DataNode::FLAG_TOUCHED))
	{
		node->TouchNode(dpm,0);
	}
	
	return node->flags.get(DataNode::FLAG_IS_GROUP);
}

unsigned int DataModel::GetChildren(const wxDataViewItem &item, wxDataViewItemArray &children) const
{

	DataNode* node=(DataNode*)item.GetID();
	if(node)
	{
		node->TouchNode(dpm, 0);
	}
	else
	{
		node = (DataNode*)m_pRoot.get();
		if (!node) return 0;
	}

	node->flags.add(DataNode::FLAG_ND_OPENED);

	size_t n = node->subnodes.size();
	children.resize(n);
	for (size_t i = 0; i < n;i++)
	{
		children[i] = wxDataViewItem(node->subnodes[i]);
	}
	return n;

}

void DataModel::SetRootNode(DataNode* p)
{
	Cleared();
	m_pRoot.reset(p);
	if (m_pRoot)
	{
		m_pRoot->OnChanged(dpm);
	}
}


wxDataViewColumn* DataModel::CreateColumn(unsigned col)
{
	return IValueColumn::CreateDataViewColumn(*m_aColumnInfo[col],col);
}

unsigned int DataModel::GetColumnCount() const
{
	return m_aColumnInfo.size();
}

wxString DataModel::GetColumnType( unsigned int col ) const
{
	if(col>=m_aColumnInfo.size()) return "";
	return str2wx(m_aColumnInfo[col]->name);
}

class DObjectRoot : public DObject
{
public:
	arr_1t<DataPtrT<DObject> > value;
	void set(DObject* p){ value.assign(1,p); }

	virtual bool DoGetChildren(DChildrenState& cs)
	{ 
		cs.set_array(value);
		return true;
	}
};

class DataNodeSymbolRoot : public DataNodeSymbol
{
public:

	DataNodeSymbolRoot(DataModel& t) :DataNodeSymbol(NULL, new DObjectRoot), model(t)
	{

	}

	virtual DataModel* DoGetModel()
	{
		return &model; 
	}

	DataModel& model;
};


void RealExpandChildren(DataModel& view, wxDataViewItem parent, unsigned depth)
{

	view.EnsuerVisible(parent);

	if (depth == 0)
	{
		return;
	}

	wxDataViewItemArray children;
	unsigned nd = view.GetChildren(parent, children);
	for (size_t i = 0; i < nd; i++)
	{
		RealExpandChildren(view, children[i], depth - 1);
	}
}


void DataModel::ExpandChildren(unsigned depth)
{
	wxDataViewItem parent;
	RealExpandChildren(*this, parent, depth);

	wxDataViewItemArray children;
	unsigned nd = GetChildren(parent, children);
	if (nd > 0)
	{
		EnsuerVisible(children[0]);
	}
}

void DataModel::Update(DObject* pitem)
{

	dpm.state.pdoc.reset(pitem);

	Cleared();
	m_pRoot.reset(NULL);
	m_pRoot.reset(new DataNodeSymbolRoot(*this));
	m_pRoot->flags.add(DataNode::FLAG_IS_GROUP | DataNode::FLAG_ND_OPENED);

	DataNodeSymbol* pnode=static_cast<DataNodeSymbol*>(m_pRoot.get());
	((DObjectRoot*)(pnode)->value.get())->set(pitem);
	pnode->TouchNode(dpm,-1);

	wxDataViewItemArray items;
	for (size_t i = 0; i < pnode->subnodes.size(); i++)
	{
		items.push_back(wxDataViewItem(pnode->subnodes[i]));
	}
	wxDataViewModel::ItemsAdded(wxDataViewItem(NULL),items);

	m_pRoot->OnChanged(dpm);

}


class DataNodeVariantRoot : public DataNodeVariant
{
public:

	DataNodeVariantRoot(DataModel& t) :DataNodeVariant(NULL, std::make_pair(String(), Variant())), model(t)
	{

	}

	virtual DataModel* DoGetModel()
	{
		return &model;
	}

	DataModel& model;
};

void DataModel::SetCanvas(IWnd_modelview* p)
{
	m_pCanvas.reset(p);
}

void DataModel::OnDocumentUpdated()
{
	GetRootNode()->OnChanged(dpm);
	GetSelector().UpdateSelection();
}

void DataModel::DoUpdateSelection()
{
	wxDataViewItemArray& arr(GetSelector().GetSelections());

	for (auto it = m_aView.begin(); it != m_aView.end(); ++it)
	{
		(*it)->SetSelections(arr);
		if (arr.size() == 1)
		{
			(*it)->EnsureVisible(arr[0]);
		}
	}

	if (m_pCanvas)
	{
		m_pCanvas->PendingRefresh();
	}
}

DataModel* DataModel::ms_pActive = NULL;

void DataModel::Update(VariantTable& table)
{

	if (m_pRoot && dynamic_cast<DataNodeVariantRoot*>(m_pRoot.get()) == NULL)
	{
		Cleared();
		m_pRoot.reset(NULL);
	}

	if (!m_pRoot)
	{
		m_pRoot.reset(new DataNodeVariantRoot(*this));
		m_pRoot->flags.add(DataNode::FLAG_IS_GROUP | DataNode::FLAG_TOUCHED | DataNode::FLAG_ND_OPENED);
	}

	((DataNodeVariant*)m_pRoot.get())->value.ref<VariantTable>() = table;

	m_pRoot->OnChanged(dpm);

}


EW_LEAVE
