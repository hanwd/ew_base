#ifndef __H_EW_UI_DATA_NODE__
#define __H_EW_UI_DATA_NODE__

#include "ewc_base/data/data_array.h"
#include "ewa_base/domdata/dobject.h"

EW_ENTER

class DObject;
class DataNode;

class DataNodeSymbol;
class NCreatorRegister;



#define IMPLEMENT_DATANODE_SYMBOL(T) \
	template<> DataNodeSymbol* NCreatorRegister_do_create_node<T>(DataNode* n, DObject* p){return NCreatorRegister::nd_create<T>(n, p);}\
	template<> void NCreatorRegister_do_init<T>(NCreatorRegister& o){o.reg0(T::sm_info, &NCreatorRegister_do_create_node<T>);}

#define IMPLEMENT_DATANODE_SYMBOL_2(T,N) \
	template<> void NCreatorRegister_do_init<T>(NCreatorRegister& o){o.reg0(T::sm_info, &NCreatorRegister_do_create_node<N>);}

template<typename T>
void NCreatorRegister_do_init(NCreatorRegister&);

template<typename T>
DataNodeSymbol* NCreatorRegister_do_create_node(DataNode* n, DObject* p);


class DLLIMPEXP_EWC_BASE DataModel;

class DLLIMPEXP_EWC_BASE DataChangedParam
{
public:
	DataChangedParam(const DataModel& m) :model(const_cast<DataModel&>(m))
	{

	}

	DataModel& model;
	DChildrenState state;
};

class DLLIMPEXP_EWC_BASE GLDC;
class DLLIMPEXP_EWC_BASE GLTool;
class DLLIMPEXP_EWC_BASE DataNode;
class DLLIMPEXP_EWC_BASE IWnd_modelview;


class DLLIMPEXP_EWC_BASE GLParam
{
public:

	enum
	{
		FLAG_CAPTURE	=1<<0,
		FLAG_RELEASE	=1<<1,
		FLAG_REFRESH	=1<<2,
		FLAG_CACHING	=1<<3,
	};

	enum
	{
		BTN_IS_DOWN		= 1 << 0,
		BTN_IS_MOVED	= 1 << 1,
		IMAGE_CACHED	= 1 << 2,
	};
};

class DLLIMPEXP_EWC_BASE GLToolData : public ObjectData
{
public:

	virtual int OnDraging(GLTool&);
	virtual int OnBtnDown(GLTool&);

	virtual int OnBtnDClick(GLTool&);

	virtual int OnBtnUp(GLTool&);
	virtual int OnBtnCancel(GLTool&);

	virtual int OnWheel(GLTool&);

	virtual int OnMoving(GLTool&);
};


class DLLIMPEXP_EWC_BASE GLTool : public Object
{
public:


	int type;
	int btn_id;

	double wheel;

	vec2i v2size;
	vec2i v2pos0,v2pos1, v2pos2;

	LitePtrT<IWnd_modelview> pview;
	DataPtrT<GLToolData> pdata;
	LitePtrT<DataNode> pnode;

	BitFlags flags;


	void CaptureMouse();
	void ReleaseMouse();

	DataNode* HitTest(int x, int y);


	void OnMouseEvent(wxMouseEvent& evt);

	void Cancel();

	GLTool();
	~GLTool();

	Functor<void(DataNode*,int)> functor;

protected:

	void DoSetNode(DataNode* node);

	void HandleSelection(int k, int f);

	bool UpdateToolData(int k);

	void HandleValue(int ret);

};

;

class DLLIMPEXP_EWC_BASE DataNode : public Object
{
public:

	enum
	{
		FLAG_TOUCHED	=1<<0,
		FLAG_ATTRIBUTED	=1<<1,
		FLAG_IS_GROUP	=1<<2,
		FLAG_SELECTED	=1<<3,
		FLAG_EXPANDED	=1<<4,
		FLAG_TMP_NODE	=1<<5,
		FLAG_ND_OPENED	=1<<6,
		FLAG_MAX		=1<<8,
	};

	DataNode(DataNode* p = NULL, const String& n = "");
	~DataNode();


	virtual bool UpdateLabel(){ return false; }

	virtual void OnChanged(DataChangedParam&);

	virtual void TouchNode(DataChangedParam&, unsigned);

	virtual void DoRender(GLDC& dc);

	virtual void DoUpdateAttribute(GLDC& dc);

	virtual void OnItemMenu(wxWindow*);

	virtual void OnGroupMenu(wxWindow*);

	virtual bool OnActivate();

	virtual void OnSelected(bool);

	virtual void OnToggle(bool);

	virtual DObject* GetItem();

	virtual DObject* GetRealItem();

	virtual bool AllowMultiSelection();

	virtual DataPtrT<GLToolData> GetToolData();

	DataModel* GetModel();

	DataNode* GetRoot();

	DataNode* GetParent();
	DataNode* GetRealParent();

	int GetDepth();

	DataNode* parent;
	String name;
	String label;
	DataNodeArray subnodes;
	BitFlags flags;
	int depth;

	LitePtrT<GLTool> ptool;

	DECLARE_OBJECT_INFO(DataNode, ObjectInfo);

protected:
	virtual DataModel* DoGetModel(){ return NULL; }
};

class DLLIMPEXP_EWC_BASE DataNodeSymbol : public DataNode
{
public:
	DataNodeSymbol(DataNode* n, DObject* p);

	DataPtrT<DObject> value;

	bool UpdateLabel();

	virtual void OnChanged(DataChangedParam&);

	const String& GetObjectName() const;

	void TouchNode(DataChangedParam&,unsigned);

	void OnItemMenu(wxWindow* w);

	virtual DObject* GetItem();

};


class DLLIMPEXP_EWC_BASE DataNodeVariant : public DataNode
{
public:

	DataNodeVariant(DataNode* p, const std::pair<String, Variant>& v);

	Variant value;

	bool UpdateLabel();
	void OnChanged(DataChangedParam&);

	void TouchNode(DataChangedParam&, unsigned);

};

template<typename T>
class DLLIMPEXP_EWC_BASE DataNodeSymbolT;


class DLLIMPEXP_EWC_BASE NCreatorRegister : public NonCopyableAndNonNewable
{
public:

	template<typename T>
	static void init(NCreatorRegister& o)
	{
		NCreatorRegister_do_init<T>(o);
	}

	template<typename T>
	static void init()
	{
		NCreatorRegister_do_init<T>(current());
	}


	static NCreatorRegister& current();

	static DataNodeSymbol* Create(DataNode* n, DObject* p);

	static DataNodeVariant* Create(DataNode* p, const std::pair<String, Variant>& v);

	template<typename T>
	static DataNodeSymbol* nd_create(DataNode* n, DObject* p){ return new DataNodeSymbolT<T>(n, p); }

	template<typename T>
	static void reg1()
	{
		reg0(T::sm_info, &NCreatorRegister::nd_create<T>);
	}

	template<typename T1,typename T2>
	static void reg2()
	{
		reg0(T1::sm_info, &NCreatorRegister::nd_create<T2>);
	}

	template<typename F>
	static void reg0(ObjectInfo& info, const F& func)
	{
		current().hmap[&info] = func;
	}

private:

	typedef DataNodeSymbol* (*data_node_ctor)(DataNode*, DObject*);
	indexer_map<ObjectInfo*, data_node_ctor> hmap;
};



EW_LEAVE
#endif

