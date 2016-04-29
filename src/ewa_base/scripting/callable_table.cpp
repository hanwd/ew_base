#include "ewa_base/scripting/callable_table.h"
#include "ewa_base/scripting/callable_class.h"
#include "ewa_base/scripting/callable_iterator.h"
#include "ewa_base/scripting/executor.h"

EW_ENTER


int CallableTableOperators::__do_setindex(Executor& ewsl,VariantTable& tb,const String& si)
{
	--ewsl.ci1.nsp;
	ewsl.popq(tb[si]);
	return CallableData::STACK_BALANCED;
}

int CallableTableOperators::__do_getindex(Executor& ewsl,VariantTable& tb,const String& si)
{
	int id=tb.find1(si);
	if(id<0)
	{
		ewsl.kerror("invalid index");
	}
	else
	{
		(*ewsl.ci1.nsp)=tb.get(id).second;
	}
	return CallableData::STACK_BALANCED;
}

int CallableTableOperators::__do_getarray(Executor& ewsl,VariantTable& tb,int pm)
{
	if(pm<1) ewsl.kerror("invalid param");

	Variant& var(ewsl.ci0.nbx[1]);
	int64_t* nid=var.ptr<int64_t>();
	if(nid)
	{
		if(size_t(*nid)>=tb.size()) ewsl.kerror("invalid index");
		ewsl.ci0.nbx[1]=tb.get(*nid).second;
		return 1;
	}
	
	String* sid=var.ptr<String>();
	if(sid)
	{
		int id=tb.find1(*sid);
		if(id>=0)
		{
			ewsl.ci0.nbx[1]=tb.get(id).second;
		}
		else
		{
			ewsl.kerror("invalid index");
		}

		return 1;
	}

	ewsl.kerror("invalid param");
	return 1;
}

int CallableTableOperators::__do_setarray(Executor& ewsl,VariantTable& tb,int pm)
{
	if(pm<1) ewsl.kerror("invalid param");

	Variant& var(ewsl.ci0.nbx[1]);
	ewsl.ci1.nsp=ewsl.ci0.nbx+0;

	int64_t* nid=var.ptr<int64_t>();
	if(nid)
	{
		if(size_t(*nid)>=tb.size()) ewsl.kerror("invalid index");
		ewsl.popq(tb.get(*nid).second);
		return STACK_BALANCED;
	}
	
	String* sid=var.ptr<String>();
	if(sid)
	{
		ewsl.popq(tb[*sid]);
		return STACK_BALANCED;
	}

	ewsl.kerror("invalid param");
	return 0;
}

void CallableTableOperators::__do_get_iterator(Executor& ewsl,VariantTable& tb,int nd)
{
	CallableData* pd=ewsl.ci1.nsp[0].kptr();
	if(nd==1)
	{
		ewsl.ci1.nsp[0].kptr(new CallableDataIterator1T<VariantTable::const_iterator>(pd,tb.begin(),tb.end()));
	}
	else if(nd==2)
	{
		ewsl.ci1.nsp[0].kptr(new CallableDataIterator2T<VariantTable::const_iterator>(pd,tb.begin(),tb.end()));
	}
	else
	{
		pd->CallableData::__get_iterator(ewsl,nd);
	}
}



int CallableTableProxy::__getindex(Executor& ewsl,const String& si)
{
	if(flags.get(FLAG_SET_THIS)) ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr(this);
	return CallableTableOperators::__do_getindex(ewsl,value,si);
}

int CallableTableProxy::__setindex(Executor& ewsl,const String& si)
{
	if(flags.get(FLAG_SET_THIS)) ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr(this);
	return CallableTableOperators::__do_setindex(ewsl,value,si);
}

int CallableTableProxy::__getarray(Executor& ewsl,int pm)
{
	if(flags.get(FLAG_READONLY)) ewsl.kerror("table is readonly");
	return CallableTableOperators::__do_getarray(ewsl,value,pm);
}

int CallableTableProxy::__setarray(Executor& ewsl,int pm)
{
	if(flags.get(FLAG_READONLY)) ewsl.kerror("table is readonly");
	return CallableTableOperators::__do_setarray(ewsl,value,pm);
}

void CallableTableProxy::__get_iterator(Executor& ewsl,int nd)
{
	CallableTableOperators::__do_get_iterator(ewsl,value,nd);
}

bool VariantTable::operator==(const  VariantTable& v2) const
{
	const VariantTable& v1(*this);
	if(v1.size()!=v2.size())
	{
		return false;
	}

	for(size_t i=0;i<v1.size();i++)
	{
		if(v1.get(i)!=v2.get(i))
		{
			return false;
		}
	}
	return true;
}

bool VariantTable::operator!=(const  VariantTable& v2) const
{
	return !((*this)==v2);
}


int CallableTableRo::__setindex(Executor& ewsl,const String&)
{
	ewsl.kerror("readonly");
	return -1;
}
int CallableTableRo::__setarray(Executor& ewsl,int pm)
{
	ewsl.kerror("readonly");
	return -1;
}

int CallableTableRo::__getindex(Executor& ewsl,const String& si)
{
	return CallableTableOperators::__do_getindex(ewsl,value,si);
}

int CallableTableRo::__getarray(Executor& ewsl,int pm)
{
	return CallableTableOperators::__do_getarray(ewsl,value,pm);
}

IMPLEMENT_OBJECT_INFO(CallableWrapT<VariantTable>,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableTableEx,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableTableRo,ObjectInfo);

EW_LEAVE
