#ifndef __EW_SCRIPTING_VARIANT_ARRAY_H__
#define __EW_SCRIPTING_VARIANT_ARRAY_H__

#include "ewa_base/scripting/variant.h"
#include "ewa_base/scripting/callable_data.h"
#include "ewa_base/scripting/variant_op.h"


EW_ENTER

inline idx_1t::size_type idx_1t::operator()(size_type i)
{
	switch(type)
	{
	case IDX_DCOLON:
		return cdat.dval[0]+cdat.dval[1]*double(i);
	case IDX_ICOLON:
		return cdat.ival[0]+cdat.ival[1]*i;
	case IDX_INTPTR:
		return ((const int64_t*)cdat.xptr)[i];
	case IDX_DBLPTR:
		return ((const double*)cdat.xptr)[i];
	case IDX_VARPTR:
		return pl_cast<size_type>::g(((const Variant*)cdat.xptr)[i]);
	case IDX_CPXPTR:
		return pl_cast<size_type>::g(((const dcomplex*)cdat.xptr)[i]);
	default:
		return i;
	};
}

template<typename A>
class DLLIMPEXP_EWA_BASE sub_xt
{
public:

	typedef typename A::value_type value_type;
	typedef typename A::size_type size_type;

	sub_xt(A& a):arr(a){}

	A& arr;
	idx_1t idx[6];

	value_type &operator()(size_type s0,size_type s1,size_type s2,size_type s3,size_type s4,size_type s5)
	{
		return arr(idx[0](s0),idx[1](s1),idx[2](s2),idx[3](s3),idx[4](s4),idx[5](s5));
	}

	value_type &operator()(size_type s0)
	{
		return arr(idx[0](s0));
	}
};



template<typename T>
class CallableWrapT<arr_xt<T> > : public CallableDataBaseT<arr_xt<T> >
{
public:
	typedef CallableDataBaseT<arr_xt<T> > basetype;
	using basetype::value;

	typedef arr_xt<T> arr_type;

	CallableWrapT():basetype(){}
	CallableWrapT(const arr_xt<T>& v):basetype(v){}

	void __get_iterator(Executor& ewsl,int d);

	virtual int __getarray(Executor& ewsl,int pm);
	virtual int __setarray(Executor& ewsl,int pm);
	virtual int __getarray_index_range(Executor& ewsl,int pm);

	virtual bool __test_dims(arr_xt_dims& dm,int op);

	template<typename A,typename X>
	void __arrset_xt(A& sub,const X& tmp);

	template<typename A,typename X>
	void __arrset_xt(A& sub,const arr_xt<X>& tmp);

	template<typename A,typename X>
	void __arrset_1t(A& sub,const X& tmp);

	template<typename A,typename X>
	void __arrset_1t(A& sub,const arr_xt<X>& tmp);

	virtual CallableData* DoClone(ObjectCloneState&){return new CallableWrapT(value);}

	DECLARE_OBJECT_INFO(CallableWrapT,ObjectInfo);

};


class ResultTreeNode
{
public:

	String value;
	arr_1t<ResultTreeNode> children;

	int32_t index;
	int32_t depth;

	ResultTreeNode(const String& v="");

	void find(arr_1t<int32_t>& res,String* p,int d);
	void addn(int idx,String* p,int d);

	void Serialize(Serializer& ar);

protected:
	void find_all(arr_1t<int32_t>& res,String* p,int d);
};



class CallableResultGroup : public CallableWrapT<arr_xt<Variant> >
{
public:
	typedef CallableWrapT<arr_xt<Variant> > basetype;

	void addvar(Variant& var,const String& acc);

	ResultTreeNode strmap;

	int __getarray(Executor& ewsl,int pm);
	int __setarray(Executor& ewsl,int pm);

	virtual void Serialize(Serializer& ar);

	DECLARE_OBJECT_INFO(CallableResultGroup,ObjectInfo)
};


DEFINE_OBJECT_NAME_T(arr_1t,"arr_1t")
DEFINE_OBJECT_NAME_T(arr_xt,"arr_xt")


EW_LEAVE

#endif

