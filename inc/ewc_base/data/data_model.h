#ifndef __H_EW_UI_DATA_MODEL__
#define __H_EW_UI_DATA_MODEL__

#include "ewc_base/data/data_node.h"
#include "ewc_base/data/data_column.h"
EW_ENTER


class DLLIMPEXP_EWC_BASE DataModel : public wxDataViewModel
{
public:

	friend class DataCanvas;

	wxDataViewCtrl* CreateDataView(wxWindow* p);

	DataModel();
	~DataModel();

	void EnsuerVisible(wxDataViewItem item);
	void ExpandItem(wxDataViewItem item);

	virtual unsigned int GetColumnCount() const;
	wxDataViewColumn* CreateColumn(unsigned col);

	virtual wxString GetColumnType(unsigned int col) const;

	virtual void GetValue(wxVariant &variant, const wxDataViewItem &item, unsigned int col) const;
	virtual bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col);

	virtual bool IsContainer(const wxDataViewItem &item) const;
	virtual unsigned int GetChildren(const wxDataViewItem &item, wxDataViewItemArray &children) const;
	virtual wxDataViewItem GetParent(const wxDataViewItem &item) const;

	virtual void AddColumn(DataColumn* p) {if(p) m_aColumnInfo.push_back(p);}

	virtual void SetRootNode(DataNode* p);
	DataNode* GetRootNode(){ return m_pRoot.get(); }


protected:

	arr_1t<DataPtrT<DataColumn> > m_aColumnInfo;
	bst_set<wxDataViewCtrl*> m_aView;
	AutoPtrT<DataNode> m_pRoot;

};

class DataModelSymbol : public DataModel
{
public:
	void Update(DObject* p);
};


EW_LEAVE
#endif
