#ifndef __EW_SCRIPTING_VARIANT_BUFFER_H__
#define __EW_SCRIPTING_VARIANT_BUFFER_H__

#include "ewa_base/scripting/variant.h"
#include "ewa_base/scripting/callable_data.h"
#include "ewa_base/scripting/variant_op.h"


EW_ENTER



template<typename T>
class DLLIMPEXP_EWA_BASE CallableWrapT<StringBuffer<T> > : public CallableDataBaseT<StringBuffer<T> >
{
public:
	typedef CallableDataBaseT<StringBuffer<T> > basetype;
	using basetype::value;

	typedef StringBuffer<T> arr_type;

	CallableWrapT():basetype(){}
	CallableWrapT(const StringBuffer<T>& v):basetype(v){}

	virtual CallableMetatable* GetMetaTable();


	virtual CallableData* DoClone(ObjectCloneState&){return this;}

	DECLARE_OBJECT_INFO(CallableWrapT,ObjectInfo);

};


EW_LEAVE
#endif
