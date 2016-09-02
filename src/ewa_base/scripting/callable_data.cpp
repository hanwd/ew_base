#include "ewa_base/scripting/callable_data.h"
#include "ewa_base/scripting/callable_class.h"
#include "ewa_base/scripting/stackstate.h"
#include "ewa_base/scripting/executor.h"
#include "ewa_base/util/strlib.h"

EW_ENTER

CallableMetatable* CallableData::sm_meta[16];

int CallableData::__getindex(Executor& ewsl,const String& si)
{

	CallableMetatable* metax=GetMetaTable();
	if(metax)
	{
		ewsl.ci1.nbp[StackState1::SBASE_META].reset(this);

		VariantTable& tb(metax->table_meta);
		int id=tb.find1(si);
		if(id>=0)
		{
			(*ewsl.ci1.nsp)=tb.get(id).second;
			return 1;
		}	
	}

	ewsl.kerror(String::Format("getindex FAILED! Class has no index:%s",si));
	return INVALID_CALL;
}

int CallableData::__getarray_index_range(Executor& ewsl,int)
{
	ewsl.kerror("not array");
	return INVALID_CALL;
}

int CallableData::__setindex(Executor& ewsl,const String& si)
{
	ewsl.kerror(String("invalid setindex:")+si);
	return INVALID_CALL;
} 

int CallableData::__getarray(Executor& ewsl,int)
{
	ewsl.kerror("invalid getarray");
	return INVALID_CALL;
}

int CallableData::__setarray(Executor& ewsl,int)
{
	ewsl.kerror("invalid setarray");
	return INVALID_CALL;
}

int CallableData::__fun_call(Executor& ewsl,int)
{
	ewsl.kerror("invalid fun_call");
	return INVALID_CALL;
}

int CallableData::__new_item(Executor& ewsl,int)
{
	ewsl.kerror("invalid new_item");
	return INVALID_CALL;
}

void CallableData::__get_iterator(Executor& ewsl,int)
{
	ewsl.kerror("invalid get_iterator");
}


bool CallableData::ToValue(String& v,int) const
{
	v.Printf("object:0x%p",this);
	return true;
}

bool CallableData::ToValue(int64_t&) const
{
	return false;
}

bool CallableData::ToValue(double&) const
{
	return false;
}

bool CallableData::ToValue(bool&) const
{
	int64_t v(0);
	if(!ToValue(v)) return false;
	return v!=0;
}

bool CallableData::ToValue(int32_t& v) const
{
	int64_t tmp;if(!ToValue(tmp)) return false;
	v=tmp;
	return true;
}

bool CallableData::ToValue(float& v) const
{
	double tmp;if(!ToValue(tmp)) return false;
	v=tmp;
	return true;
}

bool CallableData::ToValue(uint32_t& v) const
{
	int64_t tmp;if(!ToValue(tmp)) return false;
	v=tmp;
	return true;
}

bool CallableData::ToValue(uint64_t& v) const
{
	int64_t tmp;if(!ToValue(tmp)) return false;
	v=tmp;
	return true;
}

//IMPLEMENT_OBJECT_INFO(CallableData,ObjectInfo);

EW_LEAVE
