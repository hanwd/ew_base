#include "ewa_base/scripting/callable_buffer.h"
#include "ewa_base/scripting/variant_op.h"
#include "ewa_base/scripting/callable_iterator.h"

#include "ewa_base/util/strlib.h"
EW_ENTER

template<typename T> 
typename CallableWrapT<StringBuffer<T> >::infotype 
CallableWrapT<StringBuffer<T> >::sm_info(ObjectNameT<StringBuffer<T> >::MakeName("CallableWrap"));


template<typename T>
class CallableFunctionBufferClearT : public CallableFunction
{
public:
	CallableFunctionBufferClearT():CallableFunction(){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this, pm,0);
		ewsl.ci1.nbp[StackState1::SBASE_THIS].ref<StringBuffer<T> >().clear();
		ewsl.ci0.nbx[1]=ewsl.ci1.nbp[StackState1::SBASE_THIS];
		return 1;	
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionBufferClearT, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO_T1(CallableFunctionBufferClearT, ObjectInfo);

template<typename T>
class CallableFunctionBufferWriteT : public CallableFunction
{
public:
	CallableFunctionBufferWriteT():CallableFunction(){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		StringBuffer<T>& buffer(ewsl.ci1.nbp[StackState1::SBASE_THIS].ref<StringBuffer<T> >());
		for(int i=1;i<=pm;i++)
		{
			Variant& var(ewsl.ci0.nbx[i]);
			StringBuffer<T>* p=var.ptr<StringBuffer<T> >();
			if(p)
			{
				buffer+=*p;
			}
			else
			{
				buffer+=variant_cast<String>(var);
			}
		}
		ewsl.ci0.nbx[1]=ewsl.ci1.nbp[StackState1::SBASE_THIS];
		return 1;	
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionBufferWriteT, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO_T1(CallableFunctionBufferWriteT, ObjectInfo);


template<typename T>
class CallableMetatableT<StringBuffer<T> > : public CallableMetatable
{
public:

	typedef StringBuffer<T> type;
	typedef T scalar_type;

	CallableMetatableT()
	{
		value["clear"].kptr(new CallableFunctionBufferClearT<T>());
		value["write"].kptr(new CallableFunctionBufferWriteT<T>());
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
