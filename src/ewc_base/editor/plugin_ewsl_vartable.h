
#include "ewc_base/wnd/impl_wx/iwnd_dataview.h"
#include "ewc_base/data/data_model.h"

EW_ENTER


class DataNodeVariant : public DataNode
{
public:

	DataNodeVariant(DataNode* p,const String& s,const Variant& v);

	Variant value;
	String label;

	bool UpdateLabel();
	void UpdateGroup();

};

class DataModelTable : public DataModel
{
public:

	DataModelTable();

	void Update(VariantTable& table);
	void Update(VariantTable& table,DataNode* parent,DataNodeArray& children,int depth);

};



EW_LEAVE
