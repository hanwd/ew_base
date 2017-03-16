
#include "ewa_base/util/symm.h"

EW_ENTER


TableSerializer::TableSerializer(int t,VariantTable& v):type(t),value(v)
{

}

void TableSerializer::link(const String& s,double& v)
{
	if(type==WRITER)
	{
		value[s].reset(v);
	}
	if(type==READER)
	{
		int id=value.find1(s);
		if(id>=0)
		{
			v=pl_cast<double>::g(value.get(id).second);
		}
	}
}

void TableSerializer::link(const String& s,BitFlags& f,int m)
{
	int v=f.get(m)?1:0;
	link(s,v);
	if(type==READER)
	{
		f.set(m,v!=0);
	}
}

void TableSerializer::link(const String& s,int& v)
{
	if(type==WRITER)
	{
		value[s].reset(v);
	}
	if(type==READER)
	{
		int id=value.find1(s);
		if(id>=0)
		{
			v=pl_cast<int>::g(value.get(id).second);
		}
	}
}

void TableSerializer::link(const String& s,String& v)
{
	if(type==WRITER)
	{
		value[s].reset(v);
	}
	if(type==READER)
	{
		int id=value.find1(s);
		if(id>=0)
		{
			v=pl_cast<String>::g(value.get(id).second);
		}
	}
}

void TableSerializer::link(const String& s,String& v,const String& d)
{
	if(type==WRITER)
	{
		value[s].reset(v);
	}
	if(type==READER)
	{
		int id=value.find1(s);
		if(id>=0)
		{
			v=pl_cast<String>::g(value.get(id).second);
		}
		else
		{
			v=d;
		}
	}
}
EW_LEAVE
