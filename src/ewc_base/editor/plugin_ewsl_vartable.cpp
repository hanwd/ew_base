#include "plugin_ewsl_vartable.h"


EW_ENTER




DataModelTable::DataModelTable()
{
	m_aColumnInfo.append(new DataColumnName);
	m_aColumnInfo.append(new DataColumnValue);
}


void DataModelTable::Update(VariantTable& table)
{
	if (!m_pRoot)
	{
		m_pRoot.reset(new DataNodeVariant(NULL,std::make_pair(String(),Variant())));
		m_pRoot->flags.add(DataNode::FLAG_IS_GROUP | DataNode::FLAG_TOUCHED);
	}

	((DataNodeVariant*)m_pRoot.get())->value.ref<VariantTable>() = table;
	DataChangedParam dpm(*this);
	m_pRoot->OnChanged(dpm);

}


EW_LEAVE
