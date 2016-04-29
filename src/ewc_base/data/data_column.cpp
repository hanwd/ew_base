#include "ewc_base/data/data_column.h"

EW_ENTER

DataColumn::DataColumn(const String& n,int t,int w)
{
	name=n;
	type=t;
	width=w;
}

EW_LEAVE
