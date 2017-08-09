#include "ewc_base/data/data_selection.h"
#include "ewc_base/data/data_model.h"
#include "ewc_base/wnd/wnd_glcontext.h"
#include "ewc_base/wnd/impl_wx/iwnd_modelview.h"

EW_ENTER

DataSelection::DataSelection(DataModel& t) :Target(t)
{

	m_pNodeParent = NULL;
}


void DataSelection::SetSelection(DataNode* nd)
{
	if (nd)
	{
		DataNode* p = nd->GetParent();
		if (p != m_pNodeParent)
		{
			m_pNodeParent = p;
			//if(m_nDepthInit>=0)
			//{
			//	m_nDepth=m_pNodeParent?m_pNodeParent->GetDepth()+1:0;
			//}
		}

		m_setCurrent.clear();
		m_setCurrent.insert(nd);
	}
	else
	{
		m_setCurrent.clear();
	}
}



bool DataSelection::UpdateSelection()
{
	if (apply())
	{
		Target.DoUpdateSelection();
		_update_active_nodes();
		return true;
	}
	else
	{
		return false;
	}
}


void DataSelection::reset()
{
	clear();
	apply();
}

void DataSelection::Remove(DataNode* sel)
{
	auto it = m_setCurrent.find(sel);
	if (it!= m_setCurrent.end())
	{
		m_setCurrent.erase(it);
	}
}

void DataSelection::Remove(const wxDataViewItemArray &sel_)
{
	for (size_t i = 0; i<sel_.size(); i++)
	{
		DataNode* sel = (DataNode*)sel_[i].GetID();
		if (m_setCurrent.find(sel) != m_setCurrent.end())
		{
			m_setCurrent.erase(sel);
		}
	}
}

void DataSelection::SetSelection(const wxDataViewItem &sel)
{
	DataNode* nd = (DataNode*)sel.GetID();
	SetSelection(nd);
}

bool DataSelection::SetSelection(const wxDataViewItemArray& sel)
{
	clear();
	bool flag = true;
	for (int i = 0; i<(int)sel.size(); i++)
	{
		DataNode* nd = (DataNode*)sel[i].GetID();
		if (!AddSelection(nd))
		{
			flag = false;
		}
	}
	if (!flag) revert();
	return flag;
}

class DataArrayItem : public ObjectData
{
public:
	wxDataViewItemArray value;
};

wxDataViewItemArray& DataSelection::GetSelections()
{
	if (!m_pArrayItems)
	{
		m_pArrayItems.reset(new DataArrayItem);
	}

	wxDataViewItemArray& sel(static_cast<DataArrayItem*>(m_pArrayItems.get())->value);
	sel.clear();

	for (iterator it = m_setCurrent.begin(); it != m_setCurrent.end(); ++it)
	{
		sel.Add(wxDataViewItem(*it));
	}
	return sel;
}


bool DataSelection::AddSelection(DataNode* nd)
{
	if (!nd) return false;
	if (m_setCurrent.empty())
	{
		SetSelection(nd);
		return true;
	}

	if (!m_pNodeParent || !m_pNodeParent->AllowMultiSelection())
	{
		return false;
	}

	while (nd)
	{
		DataNode* pd = nd->GetRealParent();
		if (pd == m_pNodeParent)
		{
			m_setCurrent.insert(nd);
			return true;
		}

		nd = pd;
	}
	
	return false;
}

bool DataSelection::IsSelected(DataNode* it)
{
	return m_setCurrent.find(it) != m_setCurrent.end();
}



bool DataSelection::IsActive(DataNode* node)
{
	if (!m_pNodeParent) return true;
	//if (!m_pNodeParent->OGL_Leaf()) return true;

	return m_setActiveNodes.empty() || node == m_setActiveNodes.front();
}

bool DataSelection::IsGroupActive(DataNode* node)
{
	if (m_setActiveNodes.empty()) return true;
	return std::find(m_setActiveNodes.begin(), m_setActiveNodes.end(), node) != m_setActiveNodes.end();
}


void DataSelection::_update_active_nodes()
{
	m_setActiveNodes.clear();

	DataNode* p = parent();
	if (!p) return;
	if (depth() <= 2)
	{
		return;
	}

	//if (!p->flags.get(FLAG_SHOWABLE))
	//{
	//	return;
	//}

	while (p)
	{
		m_setActiveNodes.push_back(p);
		p = p->GetParent();
	}
}


bool DataSelection::empty() const
{
	return m_setCurrent.empty();
}


int DataSelection::size() const
{
	return m_setCurrent.size();
}

DataNode* DataSelection::parent()
{
	return m_pNodeParent;
}

DataNode* DataSelection::node()
{
	if (m_setCurrent.empty())
	{
		return NULL;
	}

	return *m_setCurrent.begin();
}

void DataSelection::OnCommandEvents(wxCommandEvent& evt)
{
	if (m_setCurrent.size() == 0)
	{
		return;
	}
	
	//if (m_setCurrent.size() == 1)
	//{
	//	(*m_setCurrent.begin())->OnCommandEvents(evt);
	//}
	//else if (m_pNodeParent)
	//{
	//	m_pNodeParent->OnCommandEvents(evt);
	//}
}

bool DataSelection::OnItemMenu(wxWindow* w)
{
	if (m_setCurrent.size() == 0)
	{
		if (!m_setSelected.empty())
		{
			apply();
			w->Refresh();
		}
		return false;
	}

	if (m_setCurrent.size() == 1)
	{
		if (m_setSelected.find(node()) == m_setSelected.end())
		{
			apply();
			w->Refresh();
		}
		else
		{
			m_setCurrent = m_setSelected;
		}
	}

	try
	{
		if (m_setCurrent.size() == 1)
		{
			node()->OnItemMenu(w);
			return true;
		}
		else if (m_pNodeParent)
		{
			m_pNodeParent->OnGroupMenu(w);
			return true;
		}
		else
		{
			return false;
		}
	}
	catch (std::exception& e)
	{
		this_logger().LogError(e.what());
		return false;
	}

}

void DataSelection::revert()
{
	m_setCurrent = m_setSelected;
	if (m_setCurrent.empty())
	{
		m_pNodeParent = NULL;
	}
	else
	{
		m_pNodeParent = (*m_setCurrent.begin())->GetParent();
	}
}

bool DataSelection::apply()
{

	bst_set<DataNode*> _set1, _set2;

	bool changed = false;

	for (iterator it = m_setSelected.begin(); it != m_setSelected.end(); ++it)
	{
		iterator pt = m_setCurrent.find(*it);
		if (pt == m_setCurrent.end())
		{
			(*it)->OnSelected(false);
			changed = true;
		}
	}

	for (iterator it = m_setCurrent.begin(); it != m_setCurrent.end(); ++it)
	{
		iterator pt = m_setSelected.find(*it);
		if (pt == m_setSelected.end())
		{
			(*it)->OnSelected(true);
			changed = true;
		}
	}

	m_setSelected.clear();
	m_setSelected = m_setCurrent;

	//if(m_nDepthInit>=0)
	{
		m_nDepth = m_pNodeParent ? m_pNodeParent->GetDepth() + 1 : 0;
	}


	//if (m_nHitFlag == HITTEST_DCLICK)
	//{
	//	m_nHitFlag = -1;
	//	return true;
	//}

	return changed;

}

void DataSelection::clear_and_reset_parent()
{
	m_setCurrent.clear();
	m_pNodeParent = NULL;
	apply();
}

void DataSelection::clear()
{
	//m_pNodeParent=NULL;
	m_setCurrent.clear();
}

int DataSelection::depth()
{
	return m_nDepth;
}

void DataSelection::depth(int d)
{
	if (d<0)
	{
		m_pNodeParent = NULL;
		return;
	}
	m_nDepth = d;
}

EW_LEAVE
