#include "ewa_base/scripting/callable_buffer.h"
#include "ewa_base/scripting/variant_op.h"
#include "ewa_base/scripting/callable_iterator.h"

#include "ewa_base/util/strlib.h"
EW_ENTER

template<typename T> 
typename CallableWrapT<StringBuffer<T> >::infotype 
CallableWrapT<StringBuffer<T> >::sm_info(ObjectNameT<StringBuffer<T> >::MakeName("CallableWrap"));



template<typename T>
class CallableMetatableT<StringBuffer<T> > : public CallableMetatable
{
public:

	typedef StringBuffer<T> type;
	typedef T scalar_type;

	CallableMetatableT()
	{

	}

	virtual int __fun_call(Executor& ewsl, int pm)
	{
		return 0;
	}

	DECLARE_OBJECT_CACHED_INFO(CallableMetatableT, ObjectInfo);
};

template<typename T> 
typename CallableMetatableT<StringBuffer<T> >::infotype  
CallableMetatableT<StringBuffer<T> >::sm_info(ObjectNameT<T>::MakeName("CallableMetatableT#Buffer"));

template<typename T> 
CallableMetatable* CallableWrapT<StringBuffer<T> >::GetMetaTable()
{
	return CallableMetatableT<StringBuffer<T> >::sm_info.CreateObject();
}

template class CallableWrapT<StringBuffer<char> >;
template class CallableMetatableT<StringBuffer<char> >;


EW_LEAVE
