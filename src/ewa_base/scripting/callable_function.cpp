#include "ewa_base/scripting/callable_function.h"
#include "ewa_base/scripting/executor.h"

EW_ENTER

void CallableFunction::__set_helpdata(const String& s)
{ 
	m_pHelp.reset(new HelpData); 
	m_pHelp->parse(m_sName, s);
}

int CallableFunction::__getindex(Executor& ewsl,const String& index)
{
	return CallableData::__getindex(ewsl,index);
}

HelpData* CallableFunction::__get_helpdata()
{ 
	if (!m_pHelp)
	{
		m_pHelp.reset(new HelpData);

		StringBuffer<char> sb;
		if(sb.load("scripting/help/"+m_sName+".txt"))
		{
			m_pHelp->parse(m_sName, sb);
		}
	}
	return m_pHelp.get(); 
}


CallableFunction::CallableFunction()
{

}

bool CallableFunction::ToValue(String& v,int) const
{
	v.Printf("cpp_function:%s(0x%p)",GetName(),this);
	return true;
}


CallableFunction::CallableFunction(const String& s, int f) 
:flags(f)
,m_sName(s)
{

}

void CallableFunction::Serialize(Serializer& ar)
{

}

IMPLEMENT_OBJECT_INFO(CallableFunction,ObjectInfo);

EW_LEAVE
