#ifndef __H_EW_UI_DATA_NODE__
#define __H_EW_UI_DATA_NODE__

#include "ewc_base/data/data_array.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE DataModel;

class DLLIMPEXP_EWC_BASE DataChangedParam
{
public:
	DataChangedParam(const DataModel& m) :model(const_cast<DataModel&>(m))
	{

	}

	DataModel& model;

};

class DLLIMPEXP_EWC_BASE DataSizeParam
{
public:

	enum
	{
		ACTION_CALC_MINSIZE,
		ACTION_SET_REALSIZE,
	};

	DataSizeParam()
	{

	}

	int action;

	box3d b3bbox;
	box3d b3axis;

	mat4d m4;

};



class GLDC;

class DLLIMPEXP_EWC_BASE DataNode : public Object
{
public:

	enum
	{
		FLAG_TOUCHED	=1<<0,
		FLAG_IS_GROUP	=1<<1,
	};

	DataNode(DataNode* p = NULL, const String& n = "");
	~DataNode();


	virtual bool UpdateLabel(){ return false; }

	virtual void OnChanged(DataChangedParam&);

	virtual void DoRender(GLDC& dc);

	DataNode* parent;
	String name;
	String label;
	DataNodeArray subnodes;
	BitFlags flags;
	int depth;

	DECLARE_OBJECT_INFO(DataNode, ObjectInfo);
};

class DLLIMPEXP_EWC_BASE DataNodeSymbol : public DataNode
{
public:
	DataNodeSymbol(DataNode* p, CallableSymbol* s);

	DataPtrT<CallableSymbol> value;

	bool UpdateLabel();
	virtual void OnChanged(DataChangedParam&);

	const String& GetObjectName() const;

protected:

	void _TouchNode(unsigned depth=0);
};


class DLLIMPEXP_EWC_BASE DataNodeVariant : public DataNode
{
public:

	DataNodeVariant(DataNode* p, const std::pair<String, Variant>& v);

	Variant value;

	bool UpdateLabel();
	void OnChanged(DataChangedParam&);

};

template<typename T>
class DLLIMPEXP_EWC_BASE DataNodeSymbolT;

class DLLIMPEXP_EWC_BASE DataNodeCreator : public NonCopyableAndNonNewable
{
public:

	static DataNodeCreator& current();

	static DataNodeSymbol* Create(DataNode* n, CallableSymbol* p);

	static DataNodeVariant* Create(DataNode* p, const std::pair<String, Variant>& v);

	template<typename T>
	static void Register()
	{
		current().hmap[&T::sm_info] = &_DoCreateDataNode<T>;
	}

private:

	template<typename T>
	static DataNodeSymbol* _DoCreateDataNode(DataNode* n, CallableSymbol* p)
	{
		return new DataNodeSymbolT<T>(n, p);
	}

	typedef DataNodeSymbol* (*data_node_ctor)(DataNode*, CallableSymbol*);
	indexer_map<ObjectInfo*, data_node_ctor> hmap;
};



EW_LEAVE
#endif

