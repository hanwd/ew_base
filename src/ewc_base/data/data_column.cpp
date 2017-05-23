#include "ewc_base/data/data_column.h"

EW_ENTER

DataColumn::DataColumn(const String& n,int t,int w)
{
	name=n;
	type=t;
	width=w;
}

DataColumnType::DataColumnType() 
:DataColumn(_hT("type"))
{

}

void DataColumnType::GetValue(wxVariant &variant, DataNode* node) const
{
	WxImpl<String>::set(variant, node->GetObjectName());
}


DataColumnName::DataColumnName() 
:DataColumn(_hT("name"))
{

}

void DataColumnName::GetValue(wxVariant &variant, DataNode* node) const
{
	WxImpl<String>::set(variant, node->name);
}



DataColumnValue::DataColumnValue() 
:DataColumn(_hT("value"))
{

}

void DataColumnValue::GetValue(wxVariant &variant, DataNode* node) const
{
	WxImpl<String>::set(variant, node->label);
}

EW_LEAVE
