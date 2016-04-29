
#include "ewa_base/util/symm.h"

EW_ENTER

SymbolItem::SymbolItem(const String& n) 
:name(n)
{

}

void SymbolItem::Serialize(Serializer& ar)
{
	ar & name & prop;
}

bool SymbolManager::UpdateValue()
{
	Executor ewsl;
	DState ds(ewsl,this);
	return DoUpdateValue(ds);
}

bool SymbolManager::DoUpdateValue(DState& ds)
{
	DState::LockerSM lock(ds,*this);

	for(size_t i=0;i<m_aSymbol.size();i++)
	{
		CallableSymbol* p=dynamic_cast<CallableSymbol*>(m_aSymbol.get(i).second.get());
		if(ds.test(p))
		{
			if(!p->DoUpdateValue(ds))
			{
				return false;
			}
		}
	}

	return true;
}



void SymbolManager::gp_beg(const String& s,const String& t)
{
	m_aStack.push_back(ObjState(s,t));
}

void SymbolManager::gp_end()
{
	ObjState& state(m_aStack.back());
	CallableSymbol* p=factory.CreateT<CallableSymbol>(state.type);
	if(p)
	{
		p->m_sId=state.name;
		append(p);

		TableSerializerReader ar(*this,state.value);
		p->DoTransferData(ar);
	}


	String last=m_aStack.back().name;

	m_aStack.pop_back();

	if(!m_aStack.empty())
	{
		gp_add(last);
	}

}


EW_LEAVE
