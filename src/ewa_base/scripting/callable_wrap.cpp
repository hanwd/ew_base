#include "ewa_base/scripting/callable_wrap.h"
#include "ewa_base/scripting/executor.h"
#include "ewa_base/scripting/callable_table.h"
#include "ewa_base/scripting/callable_class.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/scripting/variant_ggvar.h"

EW_ENTER

CallableMetatable* CallableWrap::GetMetaTable()
{
	return CallableData::sm_meta[m_nType];
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
	if(pm>=1) ewsl.kerror("invalid dim");

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


int CallableWrapT<TimePoint>::__getindex(Executor& ewsl,const String& index)
{
	TimeDetail td(value);

	if(index=="year")
	{
		(*ewsl.ci1.nsp).reset(td.GetYear());
	}
	else if(index=="month")
	{
		(*ewsl.ci1.nsp).reset(td.GetMonth());		
	}
	else if(index=="day")
	{
		(*ewsl.ci1.nsp).reset(td.GetDay());
	}
	else if(index=="hour")
	{
		(*ewsl.ci1.nsp).reset(td.GetHour());		
	}
	else if(index=="minute")
	{
		(*ewsl.ci1.nsp).reset(td.GetMinute());		
	}
	else if(index=="second")
	{
		(*ewsl.ci1.nsp).reset(td.GetSecond());		
	}
	else if(index=="millisecond")
	{
		(*ewsl.ci1.nsp).reset(td.GetMilliSecond());		
	}
	else
	{
		return basetype::__getindex(ewsl,index);
	}

	return CallableData::STACK_BALANCED;
}

int CallableWrapT<dcomplex>::__getindex(Executor& ewsl,const String& index)
{
	if(index=="real")
	{
		(*ewsl.ci1.nsp).reset(value.real());
	}
	else if(index=="imag")
	{
		(*ewsl.ci1.nsp).reset(value.imag());
	}
	else
	{
		return basetype::__getindex(ewsl,index);
	}

	return CallableData::STACK_BALANCED;
}

int CallableWrapT<dcomplex>::__setindex(Executor& ewsl,const String& index)
{
	if(index=="real")
	{
		value.real(variant_cast<double>(ewsl.ci1.nsp[-1]));
		ewsl.ci1.nsp-=2;
	}
	else if(index=="imag")
	{
		value.imag(variant_cast<double>(ewsl.ci1.nsp[-1]));
		ewsl.ci1.nsp-=2;
	}
	else
	{
		return basetype::__setindex(ewsl,index);
	}

	return CallableData::STACK_BALANCED;
}


IMPLEMENT_OBJECT_INFO(CallableWrapT<bool>,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableWrapT<int64_t>,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableWrapT<double>,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableWrapT<String>,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableWrapT<dcomplex>,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableWrapT<ColonData>,ObjectInfo);

EW_LEAVE
