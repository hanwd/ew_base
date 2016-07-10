#include "ewa_base/scripting/callable_array.h"
#include "ewa_base/scripting/variant_op.h"
#include "ewa_base/scripting/callable_iterator.h"

#include "ewa_base/util/strlib.h"
EW_ENTER

template<typename T1>
typename CallableWrapT<arr_xt<T1> >::infotype CallableWrapT<arr_xt<T1> >::sm_info(ObjectNameT<arr_xt<T1> >::MakeName("CallableWrap"));


void idx_1t::_update_colon(double s1,double ds)
{

	if(s1==::floor(s1) && ds==::floor(ds))
	{
		type=IDX_ICOLON;
		cdat.ival[0]=s1;
		cdat.ival[1]=ds;
	}
	else
	{
		type=IDX_DCOLON;
		cdat.dval[0]=s1;
		cdat.dval[1]=ds;
	}

	if(ds>0.0)
	{
		imin=s1;
		imax=s1+double(size-1)*ds;
	}
	else
	{
		imin=s1+double(size-1)*ds;
		imax=s1;
	}

}

int idx_1t::update(const ColonData& d,intptr_t n1,intptr_t n2)
{

	if(n2-n1==0 && !d.flags.get(ColonData::FLAG_MID))
	{
		size=d.size();
	}
	else
	{
		size=d.size(n1,n2-1);
	}

	if(size==0)
	{
		imin=imax=n1;
		return -1;
	}

	_update_colon(d.get_beg(),d.get_mid());

	return (imin>=n1&&imax<n2)?0:1;
}

int idx_1t::update(double d,intptr_t n1,intptr_t n2)
{
	return update((int64_t)d,n1,n2);
}

template<typename T>
inline int idx_1t::_update_array(int t,const arr_xt<T>& d,intptr_t n1,intptr_t n2)
{
	type=t;
	cdat.xptr=d.data();
	size=d.size();
	if(size>0)
	{
		imin=pl_cast<intptr_t>::g(d[0]);
		imax=pl_cast<intptr_t>::g(d[size-1]);
		for(size_t i=0;i<size;i++)
		{
			intptr_t id=pl_cast<intptr_t>::g(d[i]);
			if(id>imax) imax=id;
			if(id<imin) imin=id;
		}
	}
	else
	{
		imin=imax=n1;
		return -1;
	}
	return (imin>=n1&&imax<n2)?0:1;
}

int idx_1t::update(const arr_xt<int64_t>& d,intptr_t n1,intptr_t n2)
{
	return _update_array(IDX_INTPTR,d,n1,n2);
}

int idx_1t::update(const arr_xt<double>& d,intptr_t n1,intptr_t n2)
{
	return _update_array(IDX_DBLPTR,d,n1,n2);
}

int idx_1t::update(const arr_xt<Variant>& d,intptr_t n1,intptr_t n2)
{
	return _update_array(IDX_VARPTR,d,n1,n2);
}

int idx_1t::update(const arr_xt<dcomplex>& d,intptr_t n1,intptr_t n2)
{
	return _update_array(IDX_CPXPTR,d,n1,n2);
}

int idx_1t::update(int64_t d,intptr_t n1,intptr_t n2)
{
	type=IDX_ICOLON;
	size=1;
	cdat.ival[0]=d;
	imin=imax=d;
	return (imin>=n1&&imax<n2)?0:1;
}

int idx_1t::update(const String& d,intptr_t n1,intptr_t n2)
{
	return update(pl_cast<int64_t>::g(d),n1,n2);
}

int idx_1t::update(const dcomplex& d,intptr_t n1,intptr_t n2)
{
	if(d.imag()!=0.0) return -1;
	return update((int64_t)d.real(),n1,n2);
}

int idx_1t::update(Variant& d,intptr_t n1,intptr_t n2)
{
	int k=d.type();

	if(k==type_flag<int64_t>::value)
	{
		return update(variant_handler<int64_t>::raw(d),n1,n2);
	}
	else if(k==type_flag<double>::value)
	{
		return update(variant_handler<double>::raw(d),n1,n2);
	}
	else if (d.kptr())
	{
		return d.kptr()->__update_idx(*this,n1,n2);
	}
	else if (k == type_flag<bool>::value)
	{
		int64_t v = variant_handler<bool>::raw(d);
		return update(v,n1,n2);
	}
	else
	{
		return -1;
	}


}

template<typename T>
int CallableWrapT<arr_xt<T> >::__getarray(Executor& ewsl,int pm)
{
	if(pm==0)
	{
		ewsl.push(this);
	}
	else if(pm==1)
	{
		size_t sz=value.size();
		Variant* _bp=ewsl.ci1.nsp;

		idx_1t id0;
		if(id0.update(_bp[0],0,sz)!=0)
		{
			ewsl.kerror("invalid array index");
		}

		if(id0.size==1)
		{
			_bp[0].reset(value[id0(0)]);
		}
		else
		{
			arr_type tmp;
			tmp.resize(id0.size);

			for(size_t k0=0;k0<id0.size;k0++)
			{
				tmp[k0]=value[id0(k0)];
			}
			_bp[0].reset(tmp);
		}
	}
	else if(pm<=6)
	{

		sub_xt<arr_type> sub(value);
		arr_xt_dims xtd=value.size_ptr();

		Variant* _bp=ewsl.ci1.nsp-pm+1;

		int flag=0;
		for(int i=0;i<pm;i++)
		{
			flag|=sub.idx[i].update(_bp[i],0,xtd[i]);
			xtd[i]=sub.idx[i].size;
		}
		for(int i=pm;i<6;i++)
		{
			sub.idx[i].size=xtd[i];
		}

		if(flag!=0)
		{
			ewsl.kerror("invalid array index");
		}

		if(xtd.size()==1)
		{
			_bp[0].reset(sub(0,0,0,0,0,0));
		}
		else
		{
			arr_type tmp;
			tmp.resize(xtd);

			for(size_t k5=0;k5<sub.idx[5].size;k5++)
			for(size_t k4=0;k4<sub.idx[4].size;k4++)
			for(size_t k3=0;k3<sub.idx[3].size;k3++)
			for(size_t k2=0;k2<sub.idx[2].size;k2++)
			for(size_t k1=0;k1<sub.idx[1].size;k1++)
			for(size_t k0=0;k0<sub.idx[0].size;k0++)
			{
				tmp(k0,k1,k2,k3,k4,k5)=sub(k0,k1,k2,k3,k4,k5);
			}
			_bp[0].reset(tmp);
		}
	}
	else
	{
		ewsl.kerror("too many dims");
	}

	return 1;

}

template<typename T>
template<typename A,typename X>
void CallableWrapT<arr_xt<T> >::__arrset_xt(A& sub,const X& tmp)
{
	T y=pl_cast<T>::g(tmp);
	for(size_t k5=0;k5<sub.idx[5].size;k5++)
	for(size_t k4=0;k4<sub.idx[4].size;k4++)
	for(size_t k3=0;k3<sub.idx[3].size;k3++)
	for(size_t k2=0;k2<sub.idx[2].size;k2++)
	for(size_t k1=0;k1<sub.idx[1].size;k1++)
	for(size_t k0=0;k0<sub.idx[0].size;k0++)
	{
		sub(k0,k1,k2,k3,k4,k5)=y;
	}
}

template<typename T>
template<typename A,typename X>
void CallableWrapT<arr_xt<T> >::__arrset_xt(A& sub,const arr_xt<X>& tmp)
{
	for(int i=0;i<6;i++)
	{
		if(sub.idx[i].size!=tmp.size(i))
		{
			Exception::XInvalidIndex();
		}
	}

	for(size_t k5=0;k5<sub.idx[5].size;k5++)
	for(size_t k4=0;k4<sub.idx[4].size;k4++)
	for(size_t k3=0;k3<sub.idx[3].size;k3++)
	for(size_t k2=0;k2<sub.idx[2].size;k2++)
	for(size_t k1=0;k1<sub.idx[1].size;k1++)
	for(size_t k0=0;k0<sub.idx[0].size;k0++)
	{
		sub(k0,k1,k2,k3,k4,k5)=pl_cast<T>::g(tmp(k0,k1,k2,k3,k4,k5));
	}
}

template<typename T>
template<typename A,typename X>
void CallableWrapT<arr_xt<T> >::__arrset_1t(A& sub,const X& tmp)
{
	T y=pl_cast<T>::g(tmp);
	for(size_t k0=0;k0<sub.idx[0].size;k0++)
	{
		sub(k0)=y;
	}
}

template<typename T>
template<typename A,typename X>
void CallableWrapT<arr_xt<T> >::__arrset_1t(A& sub,const arr_xt<X>& tmp)
{
	if(sub.idx[0].size!=tmp.size(0))
	{
		Exception::XInvalidIndex();
	}

	for(size_t k0=0;k0<sub.idx[0].size;k0++)
	{
		sub(k0)=pl_cast<T>::g(tmp(k0));
	}
}


template<typename T>
int CallableWrapT<arr_xt<T> >::__setarray(Executor& ewsl,int pm)
{

	sub_xt<arr_type> sub(value);
	arr_xt_dims xtd=value.size_ptr();

	Variant* _bp=ewsl.ci0.nbx+1;

	int flag=0;

	if(pm==1)
	{
		flag|=sub.idx[0].update(_bp[0],0,value.size());
		if(flag==1)
		{
			//需要增大数组，扩大并强制转化为一维行向量
			value.reshape(1,sub.idx[0].imax+1);
			flag=0;
		}

		Variant& val(ewsl.ci0.nbx[0]);

		switch(val.type())
		{
		case type_flag<int64_t>::value:
			__arrset_1t(sub,variant_handler<int64_t>::raw(val));
			break;
		case type_flag<double>::value:
			__arrset_1t(sub,variant_handler<double>::raw(val));
			break;
		case type_flag<dcomplex>::value:
			__arrset_1t(sub,variant_handler<dcomplex>::raw(val));
			break;
		case type_flag<arr_xt<int64_t> >::value:
			__arrset_1t(sub,variant_handler<arr_xt<int64_t> >::raw(val));
			break;
		case type_flag<arr_xt<double> >::value:
			__arrset_1t(sub,variant_handler<arr_xt<double> >::raw(val));
			break;
		case type_flag<arr_xt<dcomplex> >::value:
			__arrset_1t(sub,variant_handler<arr_xt<dcomplex> >::raw(val));
			break;
		case type_flag<arr_xt<Variant> >::value:
			__arrset_1t(sub,variant_handler<arr_xt<Variant> >::raw(val));
			break;
		default:
			if(tl::is_same_type<T,Variant>::value)
			{
				__arrset_1t(sub,val);
				break;
			}
			ewsl.kerror("invalid param");
		}

		ewsl.ci1.nsp=ewsl.ci0.nbx-1;
		return CallableData::STACK_BALANCED;

	}

	for(int i=0;i<pm;i++)
	{
		flag|=sub.idx[i].update(_bp[i],0,xtd[i]);
		xtd[i]=sub.idx[i].size;
	}

	if(flag==0){}
	else if(flag==1)
	{
		// 自动增大数组
		arr_xt_dims kkk=value.size_ptr();
		for(int i=0;i<pm;i++)
		{
			if(sub.idx[i].imin<0)
			{
				ewsl.kerror("negative array index");
			}

			kkk[i]=std::max<size_t>(kkk[i],sub.idx[i].imax+1);
		}
		value.resize(kkk);

	}
	else
	{
		ewsl.kerror("invalid array index");
	}

	for(int i=pm;i<6;i++)
	{
		sub.idx[i].size=xtd[i];
	}

	Variant& val(ewsl.ci0.nbx[0]);

	switch(val.type())
	{
	case type_flag<int64_t>::value:
		__arrset_xt(sub,variant_handler<int64_t>::raw(val));
		break;
	case type_flag<double>::value:
		__arrset_xt(sub,variant_handler<double>::raw(val));
		break;
	case type_flag<dcomplex>::value:
		__arrset_xt(sub,variant_handler<dcomplex>::raw(val));
		break;
	case type_flag<arr_xt<int64_t> >::value:
		__arrset_xt(sub,variant_handler<arr_xt<int64_t> >::raw(val));
		break;
	case type_flag<arr_xt<double> >::value:
		__arrset_xt(sub,variant_handler<arr_xt<double> >::raw(val));
		break;
	case type_flag<arr_xt<dcomplex> >::value:
		__arrset_xt(sub,variant_handler<arr_xt<dcomplex> >::raw(val));
		break;
	case type_flag<arr_xt<Variant> >::value:
		__arrset_xt(sub,variant_handler<arr_xt<Variant> >::raw(val));
		break;
	default:
		if(tl::is_same_type<T,Variant>::value)
		{
			__arrset_xt(sub,val);
			break;
		}
		ewsl.kerror("invalid param");
	}

	ewsl.ci1.nsp=ewsl.ci0.nbx-1;
	return CallableData::STACK_BALANCED;
}


template<typename T>
void CallableWrapT<arr_xt<T> >::__get_iterator(Executor& ewsl,int d)
{
	if(d==1)
	{
		ewsl.ci1.nsp[0].kptr(new CallableDataIterator1T<typename arr_type::const_iterator>(this,value.begin(),value.end()));
	}
	else if(d==2||d==-2)
	{
		ewsl.ci1.nsp[0].kptr(new CallableDataIteratorPT<typename arr_type::const_iterator>(this,value.begin(),value.end()));
	}
	else
	{
		CallableData::__get_iterator(ewsl,d);
	}
}




template<typename T>
int CallableWrapT<arr_xt<T> >::__getarray_index_range(Executor& ewsl,int pm)
{
	if(pm<0)
	{
		int64_t sz=((int64_t)value.size())-1;
		ewsl.push(0);
		ewsl.push(sz);
		return 2;
	}
	else if(pm<6)
	{
		int64_t sz=((int64_t)value.size(pm))-1;
		ewsl.push(0);
		ewsl.push(sz);
	}
	else
	{
		ewsl.kerror("invalid dim");
	}
	return 2;
}

template<typename T>
bool CallableWrapT<arr_xt<T> >::__test_dims(arr_xt_dims& dm,int op)
{
	if(op==0)
	{
		dm=value.size_ptr();
		return true;
	}
	else if(op==1)
	{
		value.resize(dm);
		return true;
	}
	else if (op==2)
	{
		dm[0] = value.size();
		return true;
	}
	else
	{
		return false;
	}
}


void ResultTreeNode::Serialize(Serializer& ar)
{
	ar & value & index & depth;
	ar & children;
}

ResultTreeNode::ResultTreeNode(const String& v):value(v)
{
	index=-1;
}

void ResultTreeNode::find(arr_1t<int32_t>& res,String* p,int d)
{
	for(size_t i=0;i<children.size();i++)
	{
		children[i].find_all(res,p,d);
	}
}

void ResultTreeNode::find_all(arr_1t<int32_t>& res,String* p,int d)
{
	if(*p==value)
	{
		if(d<=1)
		{
			res.push_back(index);
		}
		else
		{
			find(res,p+1,d-1);

		}
	}
	else
	{
		find(res,p,d);
	}
}

void ResultTreeNode::addn(int idx,String* p,int d)
{
	if(d==0) return;

	for(size_t i=0;i<children.size();i++)
	{
		if(children[i].value==*p)
		{
			children[i].index=-1;
			children[i].addn(idx,p+1,d-1);
			if(d>children[i].depth)
			{
				children[i].depth=d;
			}
			return;
		}
	}

	children.push_back(ResultTreeNode(*p));
	children.back().index=idx;
	children.back().depth=d;
	children.back().addn(idx,p+1,d-1);

}

void CallableResultGroup::Serialize(Serializer& ar)
{
	basetype::Serialize(ar);
	ar & strmap;
}

int CallableResultGroup::__getarray(Executor& ewsl,int pm)
{

	Variant* _bp=ewsl.ci1.nsp-pm+1;
	String* sptr=_bp[0].ptr<String>();

	if(!sptr||pm!=1)
	{
		return basetype::__getarray(ewsl,pm);
	}

	arr_1t<String> arr= string_split(*sptr,".");
	if(arr.empty())
	{
		ewsl.kerror("invalid param");
	}

	arr_1t<int32_t> idx;
	strmap.find(idx,arr.data(),arr.size());

	if(idx.size()!=1)
	{
		ewsl.kerror("invalid param");
	}

	int n=idx[0];

	if(n<0||n>(int)value.size())
	{
		ewsl.kerror("invalid param");
	}

	_bp[0]=value[n];
	return 1;

}

void CallableResultGroup::addvar(Variant& var,const String& acc)
{
	arr_1t<String> arr=string_split(acc,".");
	int32_t idx=value.size();
	value.push_back(var);
	value.reshape(value.size());

	strmap.addn(idx,arr.data(),arr.size());
}

int CallableResultGroup::__setarray(Executor& ewsl,int)
{
	ewsl.kerror("readonly");
	return 0;
}


IMPLEMENT_OBJECT_INFO(CallableResultGroup,ObjectInfo)

template class CallableWrapT<arr_xt<int64_t> >;
template class CallableWrapT<arr_xt<double> >;
template class CallableWrapT<arr_xt<dcomplex> >;
template class CallableWrapT<arr_xt<Variant> >;


EW_LEAVE
