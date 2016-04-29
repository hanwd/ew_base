#ifndef __UI_IWND_TREECTRL__
#define __UI_IWND_TREECTRL__

#include "ewc_base/wnd/impl_wx/window.h"
#include "wx/treectrl.h"


EW_ENTER


class ITreeEventData;

class ObjectNode;
class ObjectNodeGroup;

class IWnd_treectrl;

class DLLIMPEXP_EWC_BASE ObjectNodeInfo : public ObjectInfo
{
public:

	ObjectNodeInfo(const String& s);

	int GetImage(ObjectNode& nd);

	int img1;
	int img2;

	static wxImageList* GetImageList(size_t sz);
};

class DLLIMPEXP_EWC_BASE ObjectNodeGroupInfo : public ObjectNodeInfo
{
public:
	ObjectNodeGroupInfo(const String& s);
};


class DLLIMPEXP_EWC_BASE ObjectNode : public ObjectData
{
public:

	enum
	{
		FLAG_EXPANDED=1<<0,
	};

	enum enumTreeEventType
	{
		TREE_ATTACH,
		TREE_EXPANDED,
		TREE_COLLAPSED,
		TREE_ACTIVATE,
		TREE_SELECTED,
		TREE_CONTEXT_MENU,
		TREE_NODE_APPEND,
		TREE_NODE_REMOVE,

		TREE_DRAG_BEGIN,
		TREE_DRAG_END,
	};

	ObjectNode(const String& f="");

	int GetImage();


	virtual ObjectNodeGroup* cast_group(){return NULL;}

	ObjectNode* FindNodeEx(const String& s)
	{
		arr_1t<ObjectNode*> p;
		return FindNode(s,p);
	}

	virtual ObjectNode* FindNode(const String& s,arr_1t<ObjectNode*>&)
	{
		if(m_sName==s) return this;
		return NULL;
	}

	virtual String GetLabel(){return m_sName;}

	wxTreeItemId GetId(){return m_pItemData?m_pItemData->GetId():wxTreeItemId();}

	bool OnTreeEventEx(enumTreeEventType t);

	virtual bool OnTreeEvent(enumTreeEventType t,ITreeEventData&);
	virtual bool OnTreeGroupEvent(enumTreeEventType t,ITreeEventData&);


	virtual String GetContextMenuName();


	String m_sName;
	LitePtrT<wxTreeItemData> m_pItemData;
	BitFlags flags;

	DECLARE_OBJECT_INFO(ObjectNode,ObjectNodeInfo)

};


class DLLIMPEXP_EWC_BASE ObjectNodeGroup : public ObjectGroupT<ObjectNode,ObjectNode>
{
public:

	typedef ObjectGroupT<ObjectNode,ObjectNode> basetype;

	ObjectNodeGroup(const String& s="");

	virtual ObjectNodeGroup* cast_group(){return this;}

	virtual bool OnTreeEvent(enumTreeEventType t,ITreeEventData&);
	virtual bool OnTreeGroupEvent(enumTreeEventType t,ITreeEventData&);

	virtual ObjectNode* FindNode(const String& s,arr_1t<ObjectNode*>& path);

	DECLARE_OBJECT_INFO(ObjectNodeGroup,ObjectNodeGroupInfo)
};



class DLLIMPEXP_EWC_BASE ITreeEventData
{
public:
	ITreeEventData(IWnd_treectrl* p=NULL):window(p){}

	IWnd_treectrl* window;
	LitePtrT<ObjectNode> pnode;

	wxTreeItemId item_src;
	wxTreeItemId item_dst;

	bool update(IWnd_treectrl* w,ObjectNode* p);
	bool update(IWnd_treectrl* w,wxTreeEvent& evt);



};

class DLLIMPEXP_EWC_BASE IWnd_treectrl : public wxTreeCtrl
{
public:

	IWnd_treectrl(wxWindow* w,const WndPropertyEx& h);
	~IWnd_treectrl();

	void ExRemoveNode(ObjectNode* p,ObjectNodeGroup* g=NULL);
	void ExAppendNodeChildren(ObjectNodeGroup* g);
	void ExAppendNode(ObjectNode* p,ObjectNodeGroup* g=NULL);

	ObjectNodeGroup* ExGetRoot();
	void ExSetRoot(ObjectNodeGroup* g);

	void ExUpdate();
	void ExUpdate(ObjectNodeGroup* g);

	ObjectNode* ExGetItemNode(wxTreeItemId id);
	ObjectNode* ExGetCurrentNode();
	ObjectNodeGroup* ExGetCurrentFolder();

	ObjectNodeGroup* ExGetItemParent(wxTreeItemId);
	ObjectNodeGroup* ExGetItemParent(ObjectNode*);

	static ITreeEventData m_tData;

protected:
	void ExDoAppendNode(ObjectNode* p,ObjectNodeGroup* g);

	void OnItemActivate(wxTreeEvent& evt);
	void OnItemSelChanged(wxTreeEvent& evt);
	void OnItemContextMenu(wxTreeEvent& evt);
	void OnItemExpand(wxTreeEvent& evt);
	void OnItemCollapsed(wxTreeEvent& evt);

	void OnItemDragBegin(wxTreeEvent& evt);
	void OnItemDragEnd(wxTreeEvent& evt);

	DataPtrT<ObjectNodeGroup> m_pRoot;
};


inline bool ObjectNode::OnTreeEventEx(enumTreeEventType t)
{
	ITreeEventData dat;
	return OnTreeEvent(t,dat);
}

EW_LEAVE

#endif
