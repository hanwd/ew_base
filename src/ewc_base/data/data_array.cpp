#include "ewc_base/data/data_array.h"
#include "ewc_base/data/data_node.h"

EW_ENTER

void DataNodeArray::clear_and_destroy()
{
	for(size_t i=0;i<(*this).size();i++)
	{
		delete (*this)[i];
	}
	clear();
}


EW_LEAVE
