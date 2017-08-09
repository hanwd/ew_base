#include "ewc_base/data/data_node.h"
#include "ewc_base/data/data_model.h"
#include "ewc_base/wnd/wnd_glcontext.h"
#include "ewc_base/mvc/mvc_view.h"
#include "ewc_base/wnd/impl_wx/iwnd_modelview.h"

EW_ENTER


DataNode::DataNode(DataNode* p,const String& n):parent(p),name(n)
{
	if (!parent)
	{
		depth = 0;
	}
	else if (parent->flags.get(FLAG_TMP_NODE))
	{
		depth = parent->depth;
	}
	else
	{
		depth = parent->depth + 1;
	}

	if (depth == 1)
	{
		parent = NULL;
	}
}

DataNode::~DataNode()
{
	subnodes.clear_and_destroy();
	if (ptool)
	{
		EW_ASSERT(ptool->pnode == this);
		ptool->pnode.reset(NULL);
	}
}

DataNode* DataNode::GetRoot()
{
	DataNode* p = this;
	for (; p->parent; p = p->parent);
	return p;
}

DataModel* DataNode::GetModel()
{
	return GetRoot()->DoGetModel();
}

void DataNode::DoRender(GLDC& dc)
{
	dc.EnterGroup();
	for (size_t i = 0; i < subnodes.size(); i++)
	{
		dc.RenderNode(subnodes[i]);
	}
	dc.LeaveGroup();
}

void DataNode::DoUpdateAttribute(GLDC& dc)
{

}

void DataNode::OnItemMenu(wxWindow*)
{

}


void DataNode::OnGroupMenu(wxWindow*)
{

}

bool DataNode::OnActivate()
{
	return true;
}

void DataNode::OnToggle(bool flag)
{
	flags.set(FLAG_EXPANDED, !flag);
}

void DataNode::OnSelected(bool flag)
{
	flags.set(FLAG_SELECTED, flag);
}

DObject* DataNode::GetItem()
{ 
	return NULL; 
}

DObject* DataNode::GetRealItem()
{
	return GetItem();
}

bool DataNode::AllowMultiSelection()
{
	return true; 
}


DataNode* DataNode::GetParent()
{
	return parent;
}

DataNode* DataNode::GetRealParent()
{
	for (DataNode* p = parent; p; p = p->parent)
	{
		if (!p->flags.get(FLAG_TMP_NODE)) return p;
	}
	return NULL;
}


int DataNode::GetDepth()
{
	return depth;
}


DataPtrT<GLToolData> DataNode::GetToolData()
{
	return NULL;
}

void DataNode::OnChanged(DataChangedParam& dpm)
{
	if (!flags.get(DataNode::FLAG_TOUCHED))
	{
		TouchNode(dpm, 0);
	}
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

void DataNodeVariant::TouchNode(DataChangedParam&, unsigned)
{
	if (!flags.get(DataNode::FLAG_TOUCHED))
	{
		flags.add(DataNode::FLAG_TOUCHED);

		CallableWrapT<VariantTable>* ptable_node = dynamic_cast<CallableWrapT<VariantTable>*>(value.kptr());
		if (!ptable_node) return;
		VariantTable& table(ptable_node->value);
		subnodes.clear_and_destroy();
		for (size_t i = 0; i < table.size(); i++)
		{
			DataNodeVariant* pv = NCreatorRegister::Create(this, table.get(i));
			subnodes.push_back(pv);
		}
	}
}

void DataNodeVariant::OnChanged(DataChangedParam& dpm)
{
	if (!flags.get(DataNode::FLAG_IS_GROUP))
	{
		return;
	}

	if (!flags.get(DataNode::FLAG_TOUCHED))
	{
		TouchNode(dpm, 0);
		return;
	}


	VariantTable& table(value.ref<VariantTable>());

	DataNodeArray& oldnodes(this->subnodes);

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
			newnodes[i] = new DataNodeVariant(this, table.get(i));
			items_add.Add(wxDataViewItem(newnodes[i]));
		}
	}

	DataNode* parent = (this == dpm.model.GetRootNode()) ? NULL : this;

	if (!items_del.empty() && this->flags.get(FLAG_ND_OPENED))
	{
		if (!dpm.model.ItemsDeleted(wxDataViewItem(parent), items_del))
		{
			System::DebugBreak();
		}
	}

	oldnodes.swap(newnodes);

	if (!items_add.empty() && this->flags.get(FLAG_ND_OPENED))
	{
		if (!dpm.model.ItemsAdded(wxDataViewItem(parent), items_add))
		{
			System::DebugBreak();
		}
	}

	if (!items_del.empty())
	{
		dpm.model.GetSelector().clear();
		dpm.model.GetSelector().UpdateSelection();
	}

	for (auto it = items_del.begin(); it != items_del.end(); ++it)
	{
		delete (DataNode*)(*it).GetID();
	}


}


DataNodeSymbol::DataNodeSymbol(DataNode* n, DObject* p) :DataNode(n, p->m_sId), value(p)
{

}

const String& DataNodeSymbol::GetObjectName() const
{
	return value->GetObjectName();
}

void DataNodeSymbol::OnItemMenu(wxWindow* w)
{
	String menu_name = "property.menu." + GetObjectName();
	EvtBase* pevt=WndManager::current().evtmgr.get(menu_name);
	if (pevt)
	{
		pevt->PopupMenu(w);
	}
}

DObject* DataNodeSymbol::GetItem()
{
	return value.get();
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

void DataNode::TouchNode(DataChangedParam&,unsigned)
{

}


void DataNodeSymbol::TouchNode(DataChangedParam& dpm,unsigned depth)
{
	if (flags.get(DataNode::FLAG_TOUCHED))
	{
		return;
	}

	flags.add(DataNode::FLAG_TOUCHED);

	EW_ASSERT(subnodes.empty());

	if (!value->DoGetChildren(dpm.state))
	{
		flags.del(DataNode::FLAG_IS_GROUP);
		return;
	}

	flags.add(DataNode::FLAG_IS_GROUP);

	for (size_t i = 0; i<dpm.state.size(); i++)
	{
		DataNode* pv = NCreatorRegister::Create(this, dpm.state[i].get());
		if (!pv) continue;
		subnodes.push_back(pv);
	}
	
	if (depth==0)
	{
		return;
	}

	for (size_t i = 0; i < subnodes.size(); i++)
	{
		subnodes[i]->TouchNode(dpm,depth - 1);
	}

}

void DataNodeSymbol::OnChanged(DataChangedParam& dpm)
{

	if (!flags.get(DataNode::FLAG_TOUCHED))
	{		
		TouchNode(dpm,0);
		return;
	}

	if (!flags.get(DataNode::FLAG_IS_GROUP))
	{
		return;
	}

	if (!value->DoGetChildren(dpm.state))
	{
		flags.del(DataNode::FLAG_IS_GROUP);
	}


	size_t n_new = dpm.state.size();
	size_t n_old = subnodes.size();

	bool changed = n_new != n_old;

	if (n_new == n_old)
	{

		for (size_t i = n_new;;)
		{
			if (i-- == 0)
			{
				break;
			}

			if (subnodes[i]->GetItem() != dpm.state[i].get())
			{
				changed = true;
				break;
			}
		}
	}

	if (!changed)
	{
		for (size_t i = 0; i < n_old; i++)
		{
			subnodes[i]->OnChanged(dpm);
		}
		return;
	}

	class nodeinfo
	{
	public:
		nodeinfo() :item(NULL), node(NULL), next(NULL){}

		DObject* item;
		DataNode* node;
		nodeinfo* next;
	};


	wxDataViewItemArray items_add;
	wxDataViewItemArray items_del;

	arr_1t<nodeinfo> nodeinfos(n_new);
	indexer_map<DObject*, nodeinfo*> hmap;	

	for (size_t i = 0; i<n_new; i++)
	{
		auto& ni(nodeinfos[i]);
		ni.item = dpm.state[i].get();
		auto& pinfo(hmap[ni.item]);
		ni.next = pinfo;
		pinfo = &ni;
	}

	for (size_t i = 0; i<n_old; i++)
	{
		DataNode* node = subnodes[i];
		DObject* item = node->GetItem();

		bool g1 = node->flags.get(FLAG_IS_GROUP);

		int idx = hmap.find1(item);
		if (idx < 0 || g1 != item->DoGetChildren(DChildrenState()))
		{
			items_del.Add(wxDataViewItem(node));
		}
		else
		{
			auto& pinfo(hmap.get(idx).second);
			if (pinfo)
			{
				pinfo->node = node;
				pinfo = pinfo->next;
			}
			else
			{
				items_del.Add(wxDataViewItem(node));
			}
		}
	}

	for (size_t i = 0; i<n_new; i++)
	{

		auto& ni(nodeinfos[i]);

		if (!ni.node)
		{
			if (ni.node = NCreatorRegister::Create(this, ni.item))
			{
				items_add.push_back(wxDataViewItem(ni.node));
			}
		}
		else
		{
			if (ni.node->UpdateLabel())
			{
				dpm.model.ItemChanged(wxDataViewItem(ni.node));
			}
		}
	}


	DataNode* real_parent = flags.get(DataNode::FLAG_TMP_NODE)?GetRealParent():this;

	if (real_parent->flags.get(FLAG_ND_OPENED))
	{
		dpm.model.ItemsDeleted(wxDataViewItem(real_parent), items_del);
	}

	subnodes.resize(n_new);
	for (size_t i = 0; i<n_new; i++)
	{
		subnodes[i] = nodeinfos[i].node;
	}

	if (real_parent->flags.get(FLAG_ND_OPENED))
	{
		dpm.model.ItemsAdded(wxDataViewItem(real_parent), items_add);
	}

	if (!items_del.empty())
	{
		dpm.model.GetSelector().clear();
		dpm.model.GetSelector().UpdateSelection();
	}

	for (auto it = items_del.begin(); it != items_del.end(); ++it)
	{
		DataNode* pn = (DataNode*)(*it).GetID();
		delete pn;
	}

	for (size_t i = 0; i < subnodes.size(); i++)
	{
		subnodes[i]->OnChanged(dpm);
	}

}



NCreatorRegister& NCreatorRegister::current()
{
	static NCreatorRegister gInstance;
	return gInstance;
}

DataNodeSymbol* NCreatorRegister::Create(DataNode* n, DObject* p)
{
	if (!p) return NULL;
	auto& hmap(current().hmap);
	auto it = hmap.find(&p->GetObjectInfo());
	if (it == hmap.end() || !(*it).second)
	{
		return new DataNodeSymbol(n,p);
	}
	return (*it).second(n, p);
}

DataNodeVariant* NCreatorRegister::Create(DataNode* p, const std::pair<String, Variant>& v)
{
	return new DataNodeVariant(p, v);
}



int GLToolData::OnMoving(GLTool&)
{
	return 0;
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

int GLToolData::OnWheel(GLTool&)
{
	return 0;
}

GLTool::GLTool()
{
	type = 0;
}

GLTool::~GLTool()
{
	DoSetNode(NULL);
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
		pview->PendingRefresh();
	}

}

void GLTool::HandleSelection(int k, int f)
{
	if (!pview) return;
	auto& selctor(pview->pmodel->GetSelector());

	if (f & 1)
	{
		if (pnode)
		{
			selctor.AddSelection(pnode.get());
			selctor.UpdateSelection();
		}
	}
	else
	{
		selctor.SetSelection(pnode.get());
		selctor.UpdateSelection();

	}

	if (k == 3)
	{
		DataModel::ms_pActive = pview->pmodel;
		selctor.OnItemMenu(pview);
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

		UpdateToolData(evt.GetButton());
		

		if (pdata)
		{
			v2pos2=v2pos1 = v2pos0;		
			HandleValue(pdata->OnBtnDown(*this));
		}
		else
		{
			HandleSelection(evt.GetButton(), evt.ControlDown()?1:0);
		}

		return;

	}
	else if (evt.ButtonDClick())
	{
		UpdateToolData(evt.GetButton());
		HandleSelection(evt.GetButton(), 2);

		if (pdata)
		{
			HandleValue(pdata->OnBtnDClick(*this));
		}		

		return;
	}

	if (!pdata)
	{
		return;
	}


	if (evt.ButtonUp())
	{
		if (!flags.get(GLParam::BTN_IS_MOVED))
		{
			HandleSelection(evt.GetButton(), evt.ControlDown() ? 1 : 0);
		}
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
	else if (int wr = evt.GetWheelRotation())
	{
		wheel = double(wr) / double(evt.GetWheelDelta());
		HandleValue(pdata->OnWheel(*this));
	}
	else if (evt.Moving())
	{
		HandleValue(pdata->OnMoving(*this));
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

	DataNode* p = dc.HitTest(x, pview->GetClientSize().y - y);
	while (p && p->flags.get(DataNode::FLAG_TMP_NODE))
	{
		p = p->parent;
	}

	if (functor)
	{
		functor(p,btn_id);
	}

	if (p)
	{
		this_logger().LogMessage("node %p,%s clicked",p,p->name);
	}

	return p;
}


void GLTool::DoSetNode(DataNode* node)
{
	if (pnode == node) return;
	if (pnode) pnode->ptool = NULL;
	pnode = node;
	if (pnode) pnode->ptool = this;
}


bool GLTool::UpdateToolData(int k)
{
	btn_id = k;
	pdata.reset(NULL);

	DoSetNode(HitTest(v2pos0[0], v2pos0[1]));

	for (DataNode* p = pnode; !pdata && p; p = p->parent)
	{
		pdata = p->GetToolData();
	}
	return pdata;
}



IMPLEMENT_OBJECT_INFO(DataNode, ObjectInfo);

EW_LEAVE
