#include "ewa_base/scripting/callable_wrap.h"
#include "ewa_base/scripting/executor.h"
#include "ewa_base/scripting/callable_table.h"
#include "ewa_base/scripting/callable_class.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/scripting/variant_ggvar.h"

EW_ENTER

CallableMetatable* CallableWrap::GetMetaTable()
{
	return CG_GGVar::current().sm_meta[m_nType].get();
}

int CallableWrapT<String>::__getarray(Executor& ewsl,int pm)
{
	if(pm!=1)
	{
		ewsl.kerror("invalid dim");
	}

	StringBuffer<wchar_t> rhs(value);

	idx_1t idx;
	int flag=idx.update(ewsl.ci1.nsp[0],0,rhs.size());
	if(flag!=0)
	{
		ewsl.kerror("invalid index");
	}

	StringBuffer<wchar_t> lhs;lhs.resize(idx.size);

	for(size_t i=0;i<idx.size;i++)
	{
		lhs[i]=rhs[idx(i)];
	}

	ewsl.ci1.nsp[0].reset(String(lhs));
	return STACK_BALANCED;
}

int CallableWrapT<String>::__setarray(Executor& ewsl,int pm)
{
	if(pm!=1)
	{
		ewsl.kerror("invalid dim");
	}

	idx_1t idx;
	int flag=idx.update(ewsl.ci1.nsp[0],0,value.size());

	StringBuffer<wchar_t> lhs(value);

	if(flag==0){}
	else if(flag==1)
	{
		lhs.resize(idx.imax+1);
	}
	else
	{
		ewsl.kerror("invalid index");
	}

	String* _p1=ewsl.ci0.nbx[0].ptr<String>();
	if(!_p1)
	{
		ewsl.kerror("invalid value");
	}

	StringBuffer<wchar_t> rhs(*_p1);

	if(rhs.size()==1)
	{
		for(size_t i=0;i<idx.size;i++)
		{
			lhs[idx(i)]=rhs[0];
		}
	}
	else if(idx.size==rhs.size())
	{
		for(size_t i=0;i<idx.size;i++)
		{
			lhs[idx(i)]=rhs[i];
		}
	}
	else
	{
		ewsl.kerror("invalid size");
	}

	value=lhs;

	ewsl.ci1.nsp=ewsl.ci0.nbx-1;
	return STACK_BALANCED;
}


int CallableWrapT<String>::__getarray_index_range(Executor& ewsl,int pm)
{
	if(unsigned(pm)>=1) ewsl.kerror("invalid dim");

	StringBuffer<wchar_t> rhs(value);
	ewsl.push(0);
	ewsl.push((int64_t)(rhs.size()-1));

	return 2;
}

bool CallableWrapT<String>::__test_dims(arr_xt_dims& dm, int op)
{
	if (op == 2)
	{
		StringBuffer<wchar_t> rhs(value);
		dm[0] = rhs.size();
		return true;
	}
	return false;
}


IMPLEMENT_OBJECT_INFO(CallableWrapT<bool>,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableWrapT<int64_t>,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableWrapT<double>,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableWrapT<String>,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableWrapT<dcomplex>,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableWrapT<ColonData>,ObjectInfo);

EW_LEAVE
