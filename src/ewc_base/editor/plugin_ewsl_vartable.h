
#include "ewc_base/wnd/impl_wx/iwnd_dataview.h"
#include "ewc_base/data/data_model.h"

EW_ENTER


class DataNodeVariant : public DataNode
{
public:

	DataNodeVariant(wxDataViewItem p,const String& s,const Variant& v);

	Variant value;
	String label;

	bool UpdateLabel();

	virtual unsigned int GetChildren(wxDataViewItemArray &children);

};

class DataModelTable : public DataModel
{
public:

	DataModelTable();

	void Update(VariantTable& table);
	void Update(VariantTable& table,wxDataViewItem parent,wxDataViewItemArray& children,int depth);

};



EW_LEAVE
