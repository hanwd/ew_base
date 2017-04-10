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



DataNodeVariant::DataNodeVariant(DataNode* p,const String& s,const Variant& v):DataNode(p,s),value(v)
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


void DataNodeVariant::UpdateGroup()
{
	if (flags.get(DataNode::FLAG_TOUCHED)) return;

	flags.add(DataNode::FLAG_TOUCHED);
	CallableWrapT<VariantTable>* ptable=dynamic_cast<CallableWrapT<VariantTable>*>(value.kptr());
	if(!ptable) return;

	const VariantTable& table(ptable->value);

	subnodes.clear_and_destroy();
	for(size_t i=0;i<table.size();i++)
	{
		DataNodeVariant* pv=new DataNodeVariant(this,table.get(i).first,table.get(i).second);
		subnodes.push_back(pv);
	}

}


DataModelTable::DataModelTable()
{
	m_aColumnInfo.append(new DataColumnName);
	m_aColumnInfo.append(new DataColumnVariant);
}

void DataModelTable::Update(VariantTable& table)
{
	Update(table,NULL,m_tRoot.subnodes,0);
}

void DataModelTable::Update(VariantTable& table, DataNode* parent, DataNodeArray& children, int depth)
{
	typedef std::pair<Variant,DataNodeVariant*> nodeinfo;
	indexer_map<String,nodeinfo> hmap;

	size_t n_new=table.size();
	size_t n_old=children.size();

	wxDataViewItemArray items_add;
	wxDataViewItemArray items_del;

	for(size_t i=0;i<n_new;i++)
	{
		hmap[table.get(i).first].first=table.get(i).second;
	}
	for(size_t i=0;i<n_old;i++)
	{
		DataNodeVariant* node=(DataNodeVariant*)children[i];
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
			if(it.second) items_del.push_back(wxDataViewItem(it.second));

			it.second=new DataNodeVariant(parent,hmap.get(i).first,table.get(i).second);
			it.second->depth=depth;
			items_add.push_back(wxDataViewItem(it.second));
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
				Update(ptable->value,it.second,it.second->subnodes,depth+1);
			}
		}			
	}


	for(size_t i=n_new;i<hmap.size();i++)
	{
		items_del.push_back(wxDataViewItem(hmap.get(i).second.second));
	}
	ItemsDeleted(wxDataViewItem(parent),items_del);

	children.resize(n_new);
	for(size_t i=0;i<n_new;i++)
	{
		children[i]=hmap.get(i).second.second;
	}
	ItemsAdded(wxDataViewItem(parent), items_add);

	for (auto it = items_del.begin(); it != items_del.end(); ++it)
	{
		delete (DataNode*)(*it).GetID();
	}

}

EW_LEAVE
