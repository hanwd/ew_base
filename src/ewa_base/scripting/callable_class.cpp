#include "ewa_base/scripting/callable_class.h"
#include "ewa_base/scripting/callable_array.h"
#include "ewa_base/scripting/parser_nodes.h"
#include "ewa_base/scripting/executor.h"
#include "ewa_base/util/strlib.h"

EW_ENTER


HelpData* CallableMetatable::__get_helpdata()
{ 
	if (!m_pHelp)
	{
		m_pHelp.reset(new HelpData);

		StringBuffer<char> sb;
		if(sb.load("scripting/help/"+m_sClassName+".txt"))
		{
			m_pHelp->parse(m_sClassName, sb);
		}
	}
	return m_pHelp.get(); 
}

int CallableMetatable::__fun_call(Executor& ewsl,int pm)
{
	ewsl.check_pmc(this,pm,0);
	DataPtrT<CallableClass> p=new CallableClass(this);
	ewsl.push(p);
	return 1;
}


void CallableClass::reset(CallableMetatable* p)
{
	metax.reset(p);

	if (metax)
	{
		size_t n = metax->table_self.size();
		value.resize(n);

		for (size_t i = 0; i<n; i++)
		{
			value[i] = metax->table_self.get(i).second;
		}
	}
	else
	{
		value.clear();
	}

}

int CallableClass::__getindex(Executor& ewsl,const String& si)
{
	ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr(this);
	{
		VariantTable& tb(metax->table_self);
		int id=tb.find1(si);
		if(id>=0)
		{
			(*ewsl.ci1.nsp)=value[id];
			return STACK_BALANCED;
		}
	}
	{
		VariantTable& tb(metax->table_meta);
		int id=tb.find1(si);
		if(id>=0)
		{
			(*ewsl.ci1.nsp)=tb.get(id).second;
			return STACK_BALANCED;
		}
	}

	ewsl.kerror(String::Format("getindex FAILED! Class has no index:%s",si));
	return INVALID_CALL;
}

int CallableClass::__setindex(Executor& ewsl,const String& si)
{

	{
		VariantTable& tb(metax->table_self);
		int id=tb.find1(si);
		if(id>=0)
		{
			--ewsl.ci1.nsp;
			ewsl.popq(value[id]);
			return 0;
		}
	}
	{
		VariantTable& tb(metax->table_meta);
		int id=tb.find1(si);
		if(id>=0)
		{
			--ewsl.ci1.nsp;
			ewsl.popq(tb.get(id).second);
			return 0;
		}
	}

	ewsl.kerror(String::Format("setindex FAILED! Class has no index:%s",si));
	return INVALID_CALL;
}

void CallableClass::Serialize(Serializer& ar)
{
	ar & metax;
	ar & value;
}

int CallableMetatable::__setindex(Executor& ewsl,const String&)
{
	ewsl.kerror("readonly");
	return -1;
}

int CallableMetatable::__setarray(Executor& ewsl,int pm)
{
	ewsl.kerror("readonly");
	return -1;
}


IMPLEMENT_OBJECT_INFO(CallableModule,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableMetatable,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableClass,ObjectInfo);

EW_LEAVE
