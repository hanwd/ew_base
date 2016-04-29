#include "ewc_base/data/data_array.h"
#include "ewc_base/data/data_node.h"

EW_ENTER


DataNodeArray::node_type& DataNodeArray::operator[](size_t i)
{
	return *(node_type*)&(wxDataViewItemArray::operator[](i));
}

void DataNodeArray::push_back(DataNode* d)
{
	wxDataViewItemArray::Add(wxDataViewItem(d));
}

void DataNodeArray::clear_and_destroy()
{
	for(size_t i=0;i<(*this).size();i++)
	{
		DataNode* p=(*this)[i];
		delete p;
	}
	wxDataViewItemArray::clear();
}


EW_LEAVE
