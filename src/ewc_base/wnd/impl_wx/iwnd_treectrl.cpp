
#include "ewc_base/wnd/impl_wx/iwnd_treectrl.h"
#include "ewc_base/app/res_manager.h"

EW_ENTER


int ObjectNodeInfo::GetImage(ObjectNode& nd)
{
	return nd.flags.get(ObjectNode::FLAG_EXPANDED)? img2:img1;
}

ObjectNodeInfo::ObjectNodeInfo(const String& s):ObjectInfo(s),img1(-1),img2(-1)
{
	img1=img2=ObjectImageList::current().get("File");
}

ObjectNodeGroupInfo::ObjectNodeGroupInfo(const String& s):ObjectNodeInfo(s)
{
	img1=ObjectImageList::current().get("Folder");
	img2=ObjectImageList::current().get("Folder_Open");
}

ObjectNode::ObjectNode(const String& f):m_sName(f)
{

}

int ObjectNode::GetImage()
{
	return GetObjectInfo().GetImage(*this);
}

String ObjectNode::GetContextMenuName()
{
	return "";
}

bool ObjectNode::OnTreeGroupEvent(enumTreeEventType,ITreeEventData& d)
{
	return false;
}

bool ObjectNode::OnTreeEvent(enumTreeEventType t,ITreeEventData& d)
{
	switch(t)
	{
	case TREE_ATTACH:
		d.window->SetItemImage(this->GetId(),GetImage());
		return true;
	case TREE_CONTEXT_MENU:
		{
			EvtGroup* g=EvtManager::current()[GetContextMenuName()].cast_group();
			if(!g) break;
			wxMenu* menu=g->CreateMenu();
			d.window->PopupMenu(menu);
			delete menu;
		}
		return true;
	case TREE_DRAG_END:
		return false;
	}
	return true;
}


ObjectNodeGroup::ObjectNodeGroup(const String& s):basetype(s)
{

}

bool ObjectNodeGroup::OnTreeGroupEvent(enumTreeEventType,ITreeEventData& d)
{
	return false;
}

bool ObjectNodeGroup::OnTreeEvent(enumTreeEventType t,ITreeEventData& d)
{

	switch(t)
	{
	case TREE_EXPANDED:
		flags.add(FLAG_EXPANDED);
		d.window->SetItemImage(this->GetId(),GetImage());
		return true;
	case TREE_COLLAPSED:
		flags.del(FLAG_EXPANDED);
		d.window->SetItemImage(this->GetId(),GetImage());
		return true;
	case TREE_DRAG_END:
		{
			if(!GetId().IsOk())
			{
				System::LogError("invalid nd_grp");
				return false;
			}

			DataPtrT<ObjectNode> nd_src=d.window->ExGetItemNode(d.item_src);

			ObjectNode* nd_tmp=d.window->ExGetItemNode(d.window->GetItemParent(d.item_src));
			if(!nd_tmp) return false;
			ObjectNodeGroup* nd_parent=nd_tmp->cast_group();
			if(!nd_parent) return false;


			nd_parent->remove(nd_src.get());			
			d.window->Delete(d.item_src);
			d.window->ExAppendNode(nd_src.get(),this);

		}
		return true;
	default:
		return basetype::OnTreeEvent(t,d);		
	}

}

ObjectNode* ObjectNodeGroup::FindNode(const String& s,arr_1t<ObjectNode*>& path)
{
	for(size_t i=0;i<size();i++)
	{
		ObjectNode* p=(*this)[i]->FindNode(s,path);
		if(p)
		{
			path.push_back(this);
			return p;
		}
	}
	return NULL;
}


ITreeEventData IWnd_treectrl::m_tData;

bool ITreeEventData::update(IWnd_treectrl* w,ObjectNode* p)
{
	window=w;
	pnode=p;
	return p!=NULL;
}

bool ITreeEventData::update(IWnd_treectrl* w,wxTreeEvent& evt)
{
	return update(w,w->ExGetItemNode(evt.GetItem()));
}

class ObjectTreeData : public wxTreeItemData
{
public:
	DataPtrT<ObjectNode> m_pNode;
	ITreeEventData m_tData;

	ObjectTreeData(ObjectNode* t,IWnd_treectrl* w=NULL):m_pNode(t),m_tData(w)
	{
		m_pNode->m_pItemData.reset(this);
		m_pNode->OnTreeEvent(ObjectNode::TREE_NODE_APPEND,m_tData);
	}

	~ObjectTreeData()
	{
		m_pNode->m_pItemData.reset(NULL);
		m_pNode->OnTreeEvent(ObjectNode::TREE_NODE_REMOVE,m_tData);
	}

};



void IWnd_treectrl::OnItemDragBegin(wxTreeEvent& evt)
{
	m_tData.item_src=evt.GetItem();
	if(m_tData.item_src==GetRootItem()) return;

	if(m_tData.update(this,evt) && m_tData.pnode->OnTreeEvent(ObjectNode::TREE_DRAG_BEGIN,m_tData))
	{
		evt.Allow();
	}
}

void IWnd_treectrl::OnItemDragEnd(wxTreeEvent& evt)
{

	m_tData.item_dst=evt.GetItem();
	if(!m_tData.item_dst.IsOk()) return;

	for(wxTreeItemId item=m_tData.item_dst;item.IsOk();item=this->GetItemParent(item))
	{
		if(item==m_tData.item_src)	return;
	}

	if(m_tData.update(this,evt))
	{
		m_tData.pnode->OnTreeEvent(ObjectNode::TREE_DRAG_END,m_tData);
	}
}

IWnd_treectrl::IWnd_treectrl(wxWindow* p,const WndPropertyEx& h)
	:wxTreeCtrl(p,h.id(),h,h)
{
		
	ExSetRoot(NULL);

	this->Connect(wxEVT_TREE_ITEM_ACTIVATED,wxTreeEventHandler(IWnd_treectrl::OnItemActivate));
	this->Connect(wxEVT_TREE_ITEM_MENU,wxTreeEventHandler(IWnd_treectrl::OnItemContextMenu));
	this->Connect(wxEVT_TREE_ITEM_EXPANDED,wxTreeEventHandler(IWnd_treectrl::OnItemExpand));
	this->Connect(wxEVT_TREE_ITEM_COLLAPSED,wxTreeEventHandler(IWnd_treectrl::OnItemCollapsed));

	this->Connect(wxEVT_TREE_SEL_CHANGED,wxTreeEventHandler(IWnd_treectrl::OnItemSelChanged));

	this->Connect(wxEVT_TREE_BEGIN_DRAG,wxTreeEventHandler(IWnd_treectrl::OnItemDragBegin));
	this->Connect(wxEVT_TREE_END_DRAG,wxTreeEventHandler(IWnd_treectrl::OnItemDragEnd));

	wxImageList *img_list=ObjectImageList::current().get_imglist(16);
	SetImageList(img_list);


}

ObjectNode* IWnd_treectrl::ExGetCurrentNode()
{
	return ExGetItemNode(this->GetSelection());
}

ObjectNodeGroup* IWnd_treectrl:: ExGetItemParent(wxTreeItemId id)
{
	if(!id.IsOk()) return NULL;
	ObjectNode* pd=ExGetItemNode(GetItemParent(id));
	if(!pd) return NULL;
	return pd->cast_group();
}

ObjectNodeGroup* IWnd_treectrl::ExGetItemParent(ObjectNode* nd)
{
	if(!nd) return NULL;
	return ExGetItemParent(nd->GetId());
}

ObjectNodeGroup* IWnd_treectrl::ExGetCurrentFolder()
{
	wxTreeItemId item=this->GetSelection();
	while(item.IsOk())
	{
		ObjectNode* nd=ExGetItemNode(item);
		if(!nd) return m_pRoot.get();
		ObjectNodeGroup* gp=nd->cast_group();
		if(gp) return gp;
		item=GetItemParent(item);
	}
	return m_pRoot.get();
}

ObjectNode* IWnd_treectrl::ExGetItemNode(wxTreeItemId id)
{
	ObjectTreeData *p=(ObjectTreeData*)this->GetItemData(id);
	return p?p->m_pNode.get():NULL;
}

void IWnd_treectrl::OnItemContextMenu(wxTreeEvent& evt)
{
	if(m_tData.update(this,evt))
	{
		this->SelectItem(evt.GetItem());
		m_tData.pnode->OnTreeEvent(ObjectNode::TREE_CONTEXT_MENU,m_tData);
	}
}

void IWnd_treectrl::OnItemActivate(wxTreeEvent& evt)
{
	if(m_tData.update(this,evt))
	{
		m_tData.pnode->OnTreeEvent(ObjectNode::TREE_ACTIVATE,m_tData);
	}
}

void IWnd_treectrl::OnItemSelChanged(wxTreeEvent& evt)
{
	if(m_tData.update(this,evt))
	{
		m_tData.pnode->OnTreeEvent(ObjectNode::TREE_SELECTED,m_tData);
	}
}

void IWnd_treectrl::OnItemExpand(wxTreeEvent& evt)
{
	if(m_tData.update(this,evt))
	{
		m_tData.pnode->OnTreeEvent(ObjectNode::TREE_EXPANDED,m_tData);
	}
}

void IWnd_treectrl::OnItemCollapsed(wxTreeEvent& evt)
{
	if(m_tData.update(this,evt))
	{
		m_tData.pnode->OnTreeEvent(ObjectNode::TREE_COLLAPSED,m_tData);
	}
}

IWnd_treectrl::~IWnd_treectrl()
{

}

void IWnd_treectrl::ExRemoveNode(ObjectNode* p,ObjectNodeGroup* g)
{
	if(!p) return;
	if(!g) g=m_pRoot.get();

	this->Delete(p->GetId());
	g->remove(p);

}

void IWnd_treectrl::ExAppendNodeChildren(ObjectNodeGroup* g)
{
	if(!g) return;

	for(size_t i=0;i<g->size();++i)
	{
		ObjectNode* p=(*g)[i].get();
		ExDoAppendNode(p,g);
	}
}

void IWnd_treectrl::ExAppendNode(ObjectNode* p,ObjectNodeGroup* g)
{
	if(!p) return;
	if(!g) g=m_pRoot.get();

	g->append(p);
	ExDoAppendNode(p,g);

	if(g->size()==1&&g->flags.get(ObjectNode::FLAG_EXPANDED))
	{
		this->Expand(g->GetId());
	}
}

ObjectNodeGroup* IWnd_treectrl::ExGetRoot()
{
	return m_pRoot.get();
}

void IWnd_treectrl::ExSetRoot(ObjectNodeGroup* g)
{
	if(m_pRoot) DeleteAllItems();
	if(!g)
	{
		g=new ObjectNodeGroup("Root");
		g->flags.add(ObjectNode::FLAG_EXPANDED);
	}

	m_pRoot.reset(g);
	ExDoAppendNode(g,NULL);
}

void IWnd_treectrl::ExUpdate()
{
	ExUpdate(m_pRoot.get());
}

void IWnd_treectrl::ExUpdate(ObjectNodeGroup* g)
{
	if(!g) return;
	wxTreeItemId root=g->GetId();
	if(!root.IsOk())
	{
		return;
	}

	this->DeleteChildren(root);
	ExAppendNodeChildren(g);

}

void IWnd_treectrl::ExDoAppendNode(ObjectNode* p,ObjectNodeGroup* g)
{
	wxString name(p->GetLabel().c_str());
	ObjectTreeData* data=new ObjectTreeData(p,this);

	int img1=-1;
	int img2=-1;

	if(g)
	{
		this->AppendItem(g->GetId(),name,img1,img2,data);
	}
	else
	{
		this->AddRoot(name,img1,img2,data);
	}

	ITreeEventData d(this);
	p->OnTreeEvent(ObjectNode::TREE_ATTACH,d);

	ObjectNodeGroup* x=p->cast_group();
	if(!x) return;

	ExAppendNodeChildren(x);
	if(x->flags.get(ObjectNode::FLAG_EXPANDED))
	{
		this->Expand(x->GetId());
	}

}

IMPLEMENT_OBJECT_INFO(ObjectNode,ObjectNodeInfo)
IMPLEMENT_OBJECT_INFO(ObjectNodeGroup,ObjectNodeGroupInfo)

IMPLEMENT_WX_CONTROL(IWnd_treectrl)

EW_LEAVE
