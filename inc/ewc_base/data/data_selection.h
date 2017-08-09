#ifndef __H_EW_UI_DATA_SELECTION__
#define __H_EW_UI_DATA_SELECTION__

#include "ewc_base/data/data_node.h"

class wxDataViewItem;
class wxDataViewItemArray;


EW_ENTER

class DataSelection : public ObjectData
{
public:

	DataSelection(DataModel& t);

	typedef bst_set<DataNode*>::iterator iterator;

	void SetSelection(DataNode* nd);
	bool AddSelection(DataNode* nd);
	void Remove(DataNode* nd);

	void SetSelection(const wxDataViewItem &sel);
	bool SetSelection(const wxDataViewItemArray& sel);
	void Remove(const wxDataViewItemArray &items);
	wxDataViewItemArray& GetSelections();

	DataNode* node();
	DataNode* parent();

	void clear_and_reset_parent();

	virtual bool apply();
	virtual void revert();

	virtual void clear();
	virtual void reset();

	int size() const;
	bool empty() const;

	virtual bool UpdateSelection();

	bool IsSelected(DataNode* it);

	iterator begin(){ return m_setCurrent.begin(); }
	iterator end(){ return m_setCurrent.end(); }

	iterator sel_begin(){ return m_setSelected.begin(); }
	iterator sel_end(){ return m_setSelected.end(); }

	bool OnItemMenu(wxWindow* w);
	void OnCommandEvents(wxCommandEvent& evt);

	int depth();
	void depth(int d);


	bool IsActive(DataNode* node);
	bool IsGroupActive(DataNode* node);

protected:

	DataModel& Target;

	int m_nHitFlag;
	int m_nDepth;
	bst_set<DataNode*> m_setSelected;
	bst_set<DataNode*> m_setCurrent;
	arr_1t<DataNode*> m_setActiveNodes;
	DataNode* m_pNodeParent;

	DataPtrT<ObjectData> m_pArrayItems;

	void _update_active_nodes();
};


EW_LEAVE

#endif
