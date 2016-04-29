#include "plugin_ewsl_vartable.h"


EW_ENTER

class DataColumnVariant : public DataColumn
{
public:
	DataColumnVariant():DataColumn("value"){}

    virtual void GetValue(wxVariant &variant,DataNode* node) const
	{
		WxImpl<String>::set(variant,((DataNodeVariant*)node)->label);
	}
};



DataNodeVariant::DataNodeVariant(wxDataViewItem p,const String& s,const Variant& v):DataNode(p,s),value(v)
{		
	if(dynamic_cast<CallableWrapT<VariantTable>*>(value.kptr())!=NULL)
	{
		flags.add(FLAG_IS_GROUP);
	}
	UpdateLabel();
}


bool DataNodeVariant::UpdateLabel()
{
	String tmp=pl_cast<String>::g(value);
	if(tmp==label) return false;
	label=tmp;
	return true;
}

unsigned int DataNodeVariant::GetChildren(wxDataViewItemArray &children)
{
	if(!flags.get(DataNode::FLAG_TOUCHED))
	{
		flags.add(DataNode::FLAG_TOUCHED);
		CallableWrapT<VariantTable>* ptable=dynamic_cast<CallableWrapT<VariantTable>*>(value.kptr());
		if(!ptable) return 0;

		const VariantTable& table(ptable->value);

		subnodes.clear_and_destroy();
		for(size_t i=0;i<table.size();i++)
		{
			DataNodeVariant* pv=new DataNodeVariant(wxDataViewItem(this),table.get(i).first,table.get(i).second);
			subnodes.push_back(pv);
		}		
	}

	children=subnodes;
	return children.size();
}


DataModelTable::DataModelTable()
{
	m_aColumnInfo.append(new DataColumnName);
	m_aColumnInfo.append(new DataColumnVariant);
}

void DataModelTable::Update(VariantTable& table)
{
	//if(m_tRoot.subnodes.empty())
	//{
	//	m_tRoot.subnodes.push_back(new DataNodeVariant(wxDataViewItem(),"variables",Variant()));
	//	m_tRoot.subnodes[0]->flags.add(DataNode::FLAG_IS_GROUP|DataNode::FLAG_TOUCHED);
	//	ItemsAdded(m_tRoot.parent,m_tRoot.subnodes);
	//}
	//Update(table,wxDataViewItem(m_tRoot.subnodes[0]),m_tRoot.subnodes[0]->subnodes,0);

	Update(table,wxDataViewItem(),m_tRoot.subnodes,0);
}

void DataModelTable::Update(VariantTable& table,wxDataViewItem parent,wxDataViewItemArray& children,int depth)
{
	typedef std::pair<Variant,DataNodeVariant*> nodeinfo;
	indexer_map<String,nodeinfo> hmap;

	size_t n_new=table.size();
	size_t n_old=children.size();

	DataNodeArray items_add;
	DataNodeArray items_del;

	for(size_t i=0;i<n_new;i++)
	{
		hmap[table.get(i).first].first=table.get(i).second;
	}
	for(size_t i=0;i<n_old;i++)
	{
		DataNodeVariant* node=(DataNodeVariant*)children[i].GetID();
		hmap[node->name].second=node;
	}

	for(size_t i=0;i<n_new;i++)
	{

		nodeinfo& it(hmap.get(i).second);

		CallableWrapT<VariantTable>* ptable=dynamic_cast<CallableWrapT<VariantTable>*>(table.get(i).second.kptr());

		bool g1=ptable!=NULL;
		bool g2=it.second && it.second->flags.get(DataNode::FLAG_IS_GROUP);

		if(!it.second||g1!=g2)
		{
			if(it.second) items_del.push_back(it.second);

			it.second=new DataNodeVariant(parent,hmap.get(i).first,table.get(i).second);
			it.second->depth=depth;
			items_add.push_back(it.second);
		}
		else
		{
			it.second->value=table.get(i).second;
			if(it.second->UpdateLabel())
			{
				ItemChanged(wxDataViewItem(it.second));
			}

			if(g1 && it.second->flags.get(DataNode::FLAG_TOUCHED))
			{
				Update(ptable->value,wxDataViewItem(it.second),it.second->subnodes,depth+1);
			}
		}			
	}


	for(size_t i=n_new;i<hmap.size();i++)
	{
		items_del.push_back(hmap.get(i).second.second);
	}
	ItemsDeleted(parent,items_del);

	children.resize(n_new);
	for(size_t i=0;i<n_new;i++)
	{
		children[i]=wxDataViewItem(hmap.get(i).second.second);
	}
	ItemsAdded(parent,items_add);

	items_del.clear_and_destroy();

}

EW_LEAVE
