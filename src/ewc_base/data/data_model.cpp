#include "ewc_base/data/data_model.h"

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
		:wxDataViewCtrl(w,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxDV_HORIZ_RULES|wxDV_VERT_RULES)
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

	}


	~DataCanvas()
	{
		if (m_targetWindow && m_targetWindow != m_targetWindow->GetEventHandler())
		{
			m_targetWindow->PopEventHandler();
		}

		Target.m_aView.erase(this);
	}



};

wxDataViewCtrl* DataModel::CreateDataView(wxWindow* p)
{
	return new DataCanvas(p,*this);
}

DataModel::DataModel() :dpm(*this)
{
	m_pAttributeManager.reset(new DAttributeManager);
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
		node->OnChanged(dpm);
	}
	else
	{
		node = (DataNode*)m_pRoot.get();
		if (!node) return 0;
	}

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

void DataModel::Update(DObject* pitem)
{
	if (m_pRoot && dynamic_cast<DataNodeSymbolRoot*>(m_pRoot.get()) == NULL)
	{
		Cleared();
		m_pRoot.reset(NULL);
	}

	if (!m_pRoot)
	{
		m_pRoot.reset(new DataNodeSymbolRoot(*this));
		m_pRoot->flags.add(DataNode::FLAG_IS_GROUP);
	}
	else
	{
		m_pRoot->flags.del(DataNode::FLAG_TOUCHED);
	}

	
	DataNodeSymbol* pnode=static_cast<DataNodeSymbol*>(m_pRoot.get());
	((DObjectRoot*)(pnode)->value.get())->set(pitem);

	pnode->TouchNode(dpm,-1);

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
		m_pRoot->flags.add(DataNode::FLAG_IS_GROUP | DataNode::FLAG_TOUCHED);
	}

	((DataNodeVariant*)m_pRoot.get())->value.ref<VariantTable>() = table;
	DataChangedParam dpm(*this);
	m_pRoot->OnChanged(dpm);

}


EW_LEAVE
