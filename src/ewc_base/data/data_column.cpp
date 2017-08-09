#include "ewc_base/data/data_column.h"

EW_ENTER

DataColumn::DataColumn(const String& n,int t,int w)
{
	name=n;
	type=t;
	width=w;
}

DataColumnType::DataColumnType(int wd) 
	:DataColumn(_hT("type"), COLUMNTYPE_STRING,wd)
{

}

void DataColumnType::GetValue(wxVariant &variant, DataNode* node) const
{
	WxImpl<String>::set(variant, node->GetObjectName());
}


DataColumnName::DataColumnName(int wd) 
	:DataColumn(_hT("name"), COLUMNTYPE_STRING, wd)
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
