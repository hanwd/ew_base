#include "ewc_base/data/data_model.h"

EW_ENTER

class DataCanvas : public wxDataViewCtrl
{
public:
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
	}

	~DataCanvas()
	{
		Target.m_aView.erase(this);
	}



};

wxDataViewCtrl* DataModel::CreateDataView(wxWindow* p)
{
	return new DataCanvas(p,*this);
}

DataModel::DataModel()
{
	
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

bool DataModel::IsContainer( const wxDataViewItem &item ) const
{
	DataNode* node=(DataNode*)item.GetID();
	if(!node) return true;
	return node->flags.get(DataNode::FLAG_IS_GROUP);
}

void DataModel::GetValue( wxVariant &variant,const wxDataViewItem &item, unsigned int col ) const
{
	DataNode* node=(DataNode*)item.GetID();
	if(!node||col>=m_aColumnInfo.size()) return;
	return m_aColumnInfo[col]->GetValue(variant,node);
}

unsigned int DataModel::GetChildren(const wxDataViewItem &item, wxDataViewItemArray &children) const
{
	DataChangedParam dpm(*this);

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
	DataChangedParam dpm(*this);
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

class CallableSymbolArray : public CallableSymbol
{
public:
	arr_1t<DataPtrT<CallableSymbol> > value;
	void set(CallableSymbol* p){ value.assign(1,p); }

	virtual bool DoGetChildren(arr_1t<DataPtrT<CallableSymbol> >* p)
	{ 
		if (p) *p = value;
		return false;
	}
};

void DataModelSymbol::Update(CallableSymbol* pitem)
{
	if (!m_pRoot)
	{
		m_pRoot.reset(new DataNodeSymbol(NULL, new CallableSymbolArray));
		m_pRoot->flags.add(DataNode::FLAG_IS_GROUP | DataNode::FLAG_TOUCHED);
	}

	((CallableSymbolArray*)((DataNodeSymbol*)m_pRoot.get())->value.get())->set(pitem);

	DataChangedParam dpm(*this);
	m_pRoot->OnChanged(dpm);

}


EW_LEAVE
