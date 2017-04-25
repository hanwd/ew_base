#include "ewc_base/data/data_node.h"
#include "ewc_base/data/data_model.h"
#include "ewc_base/wnd/wnd_glcontext.h"
#include "ewc_base/wnd/impl_wx/iwnd_modelview.h"

EW_ENTER


DataNode::DataNode(DataNode* p,const String& n):parent(p),name(n)
{
	depth=p?p->depth+1:0;
	if (depth == 1)
	{
		parent = NULL;
	}
}

DataNode::~DataNode()
{
	subnodes.clear_and_destroy();
}

void DataNode::DoRender(GLDC& dc)
{
	//dc.EnterGroup();
	for (size_t i = 0; i < subnodes.size(); i++)
	{
		dc.RenderNode(subnodes[i]);
	}
	//dc.LeaveGroup();
}


void DataNode::OnChanged(DataChangedParam&)
{
	if (!flags.get(DataNode::FLAG_IS_GROUP))
	{
		return;
	}

	flags.add(DataNode::FLAG_TOUCHED);
}


DataNodeVariant::DataNodeVariant(DataNode* p, const std::pair<String, Variant>& v) :DataNode(p, v.first), value(v.second)
{
	if (dynamic_cast<const CallableWrapT<VariantTable>*>(value.kptr()) != NULL)
	{
		flags.add(FLAG_IS_GROUP);
	}
	UpdateLabel();
}


bool DataNodeVariant::UpdateLabel()
{
	String tmp = pl_cast<String>::g(value);

	if (tmp == label) return false;
	label = tmp;
	return true;
}

void DataNodeVariant::OnChanged(DataChangedParam& dpm)
{
	if (!flags.get(DataNode::FLAG_IS_GROUP))
	{
		return;
	}

	if (!flags.get(DataNode::FLAG_TOUCHED))
	{
		flags.add(DataNode::FLAG_TOUCHED);

		CallableWrapT<VariantTable>* ptable_node = dynamic_cast<CallableWrapT<VariantTable>*>(value.kptr());
		if (!ptable_node) return;
		VariantTable& table(ptable_node->value);
		subnodes.clear_and_destroy();
		for (size_t i = 0; i < table.size(); i++)
		{
			DataNodeVariant* pv = DataNodeCreator::Create(this, table.get(i));
			subnodes.push_back(pv);
		}

		return;
	}


	VariantTable& table(value.ref<VariantTable>());

	DataNodeVariant* node = this;

	DataNodeArray& oldnodes(node->subnodes);
	DataNode* parent = node == dpm.model.GetRootNode() ? NULL : node;

	wxDataViewItemArray items_add;
	wxDataViewItemArray items_del;

	DataNodeArray newnodes;
	newnodes.resize(table.size());

	for (auto it = oldnodes.begin(); it != oldnodes.end(); ++it)
	{
		int id = table.find1((*it)->name);
		if (id<0 || (*it)->flags.get(DataNode::FLAG_IS_GROUP) != (dynamic_cast<CallableWrapT<VariantTable>*>(table.get(id).second.kptr()) != NULL))
		{
			items_del.Add(wxDataViewItem(*it));
		}
		else
		{
			newnodes[id] = *it;
		}
	}

	for (size_t i = 0; i<table.size(); i++)
	{
		if (DataNodeVariant* p = (DataNodeVariant*)newnodes[i])
		{
			p->value = table.get(i).second;
			if (p->UpdateLabel())
			{
				dpm.model.ItemChanged(wxDataViewItem(p));
			}

			if (p->flags.get(DataNode::FLAG_IS_GROUP) && p->flags.get(DataNode::FLAG_TOUCHED))
			{
				p->OnChanged(dpm);
			}

		}
		else
		{
			newnodes[i] = new DataNodeVariant(parent, table.get(i));
			items_add.Add(wxDataViewItem(newnodes[i]));
		}
	}


	dpm.model.ItemsDeleted(wxDataViewItem(parent), items_del);
	oldnodes.swap(newnodes);
	dpm.model.ItemsAdded(wxDataViewItem(parent), items_add);

	for (auto it = items_del.begin(); it != items_del.end(); ++it)
	{
		delete (DataNode*)(*it).GetID();
	}
}



DataNodeSymbol::DataNodeSymbol(DataNode* n, CallableSymbol* p) :DataNode(n, p->m_sId), value(p)
{
	flags.set(FLAG_IS_GROUP, value->DoGetChildren(NULL));
}

const String& DataNodeSymbol::GetObjectName() const
{
	return value->GetObjectName();
}

bool DataNodeSymbol::UpdateLabel()
{
	if (name != value->m_sId)
	{
		name = value->m_sId;
		return true;
	}
	else
	{
		return false;
	}
}

void DataNodeSymbol::_TouchNode(unsigned depth)
{
	if (flags.get(DataNode::FLAG_TOUCHED))
	{
		return;
	}

	flags.add(DataNode::FLAG_TOUCHED);

	arr_1t<DataPtrT<CallableSymbol> > arr;
	value->DoGetChildren(&arr);

	EW_ASSERT(subnodes.empty());

	for (size_t i = 0; i<arr.size(); i++)
	{
		DataNode* pv = DataNodeCreator::Create(this, arr[i].get());
		if (!pv) continue;
		subnodes.push_back(pv);
	}

	if (depth==0)
	{
		return;
	}

	for (size_t i = 0; i < subnodes.size(); i++)
	{
		DataNodeSymbol* pv = dynamic_cast <DataNodeSymbol*>(subnodes[i]);
		if (pv)
		{
			pv->_TouchNode(depth - 1);
		}
	}

}

void DataNodeSymbol::OnChanged(DataChangedParam& dpm)
{
	if (!flags.get(DataNode::FLAG_IS_GROUP))
	{
		return;
	}

	if (!flags.get(DataNode::FLAG_TOUCHED))
	{		
		_TouchNode();
		return;
	}

	arr_1t<DataPtrT<CallableSymbol> > table;
	value->DoGetChildren(&table);

	typedef std::pair<CallableSymbol*, DataNodeSymbol*> nodeinfo;
	indexer_map<CallableSymbol*, nodeinfo> hmap;

	DataNode* parent = this;


	size_t n_new = table.size();
	size_t n_old = subnodes.size();

	wxDataViewItemArray items_add;
	wxDataViewItemArray items_del;

	for (size_t i = 0; i<n_new; i++)
	{
		hmap[table[i].get()].first = table[i].get();
	}
	for (size_t i = 0; i<n_old; i++)
	{
		DataNodeSymbol* node = (DataNodeSymbol*)subnodes[i];
		bool g1 = node->flags.get(FLAG_IS_GROUP);

		int idx = hmap.find1(node->value.get());
		if (idx < 0 || g1 != node->value->DoGetChildren(NULL))
		{
			items_del.Add(wxDataViewItem(node));
		}
		else
		{
			hmap[node->value.get()].second = node;
		}
	}

	for (size_t i = 0; i<n_new; i++)
	{

		nodeinfo& it(hmap.get(i).second);

		if (!it.second)
		{
			if (it.second = DataNodeCreator::Create(parent, table[i].get()))
			{
				items_add.push_back(wxDataViewItem(it.second));
			}
		}
		else
		{
			if (it.second->UpdateLabel())
			{
				dpm.model.ItemChanged(wxDataViewItem(it.second));
			}

			if (it.second->flags.get(FLAG_IS_GROUP) && it.second->flags.get(DataNode::FLAG_TOUCHED))
			{
				it.second->OnChanged(dpm);
			}
		}
	}



	dpm.model.ItemsDeleted(wxDataViewItem(parent), items_del);
	subnodes.resize(n_new);
	for (size_t i = 0; i<n_new; i++)
	{
		subnodes[i] = hmap.get(i).second.second;
	}
	dpm.model.ItemsAdded(wxDataViewItem(parent), items_add);

	for (auto it = items_del.begin(); it != items_del.end(); ++it)
	{
		delete (DataNode*)(*it).GetID();
	}
}



DataNodeCreator& DataNodeCreator::current()
{
	static DataNodeCreator gInstance;
	return gInstance;
}

DataNodeSymbol* DataNodeCreator::Create(DataNode* n, CallableSymbol* p)
{
	if (!p) return NULL;
	indexer_map<ObjectInfo*, data_node_ctor>& hmap(current().hmap);
	indexer_map<ObjectInfo*, data_node_ctor>::iterator it = hmap.find(&p->GetObjectInfo());
	if (it == hmap.end() || !(*it).second)
	{
		return new DataNodeSymbol(n,p);
	}
	return (*it).second(n, p);
}

DataNodeVariant* DataNodeCreator::Create(DataNode* p, const std::pair<String, Variant>& v)
{
	return new DataNodeVariant(p, v);
}


int GLToolData::OnDraging(GLTool& gt)
{
	gt.flags.add(GLParam::BTN_IS_MOVED);
	return 0;
}

int GLToolData::OnBtnDown(GLTool&)
{
	return 0;
}

int GLToolData::OnBtnDClick(GLTool&)
{
	return 0;
}

int GLToolData::OnBtnUp(GLTool& gt)
{ 
	return OnBtnCancel(gt);
}

int GLToolData::OnBtnCancel(GLTool& gt)
{
	if (gt.flags.get(GLParam::BTN_IS_DOWN))
	{
		return GLParam::FLAG_RELEASE|GLParam::FLAG_REFRESH;
	}
	return 0;
}

GLTool::GLTool()
{
	type = 0;
}

void GLTool::Cancel()
{
	HandleValue(pdata->OnBtnCancel(*this));
}

void GLTool::HandleValue(int ret)
{

	if (ret & GLParam::FLAG_CAPTURE)
	{
		flags.add(GLParam::BTN_IS_DOWN);
		CaptureMouse();
	}

	if (ret & GLParam::FLAG_RELEASE)
	{
		flags.del(GLParam::BTN_IS_DOWN|GLParam::IMAGE_CACHED);
		ReleaseMouse();
	}


	if (ret & GLParam::FLAG_CACHING)
	{
		if (flags.add2(GLParam::IMAGE_CACHED))
		{
			pview->ImageUpdate();
		}
	}

	if (ret & GLParam::FLAG_REFRESH)
	{
		pview->Refresh();

	}


}

void GLTool::OnMouseEvent(wxMouseEvent& evt)
{
	if (pview)
	{
		wxSize sz=pview->GetClientSize();
		v2size.set2(sz.x, sz.y);
	}


	v2pos0.set2(evt.GetX(), evt.GetY());

	if (evt.ButtonDown())
	{
		if (flags.get(GLParam::BTN_IS_DOWN))
		{
			return;
		}

		flags.del(GLParam::IMAGE_CACHED|GLParam::BTN_IS_MOVED);

		if (!UpdateToolData())
		{
			return;
		}

		btn_id = evt.GetButton();
		v2pos1 = v2pos0;
		
		HandleValue(pdata->OnBtnDown(*this));

	}
	else if (evt.ButtonDClick())
	{
		if (!UpdateToolData())
		{
			return;
		}

		btn_id = evt.GetButton();
		HandleValue(pdata->OnBtnDClick(*this));
	}
	else if (!pdata)
	{

	}
	else if (evt.ButtonUp())
	{
		HandleValue(pdata->OnBtnUp(*this));
	}
	else if (evt.Dragging())
	{
		if (flags.get(GLParam::BTN_IS_DOWN))
		{
			v2pos2 = v2pos0;
			HandleValue(pdata->OnDraging(*this));
		}
	}

}

void GLTool::CaptureMouse()
{
	if (pview)
	{
		pview->CaptureMouse();
	}
}

void GLTool::ReleaseMouse()
{
	if (pview)
	{
		pview->ReleaseMouse();
	}
}

DataNode* GLTool::HitTest(int x, int y)
{

	if (!pview)
	{
		return NULL;
	}

	GLDC& dc(pview->dc);

	dc.SetCurrent(pview);
	dc.Reshape(pview->GetClientSize());
	dc.RenderSelect(pview->pmodel);

	DataNode* p = dc.HitTest(v2pos0[0], pview->GetClientSize().y - v2pos0[1]);

	return p;
}

bool GLTool::UpdateToolData()
{
	pdata.reset(NULL);
	for (DataNode* p = HitTest(v2pos0[0], v2pos0[1]); !pdata && p; p = p->parent)
	{
		pdata = p->GetToolData();
	}
	return pdata;
}



IMPLEMENT_OBJECT_INFO(DataNode, ObjectInfo);

EW_LEAVE
