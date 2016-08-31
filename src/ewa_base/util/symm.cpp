
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

bool SymbolManager::DoUpdateValue(DState& dp,const String& name)
{
	CallableSymbol* p=get_item_t<CallableSymbol>(name);
	if(!p) return true;

	DState::LockerSM lock(dp,*this);
	return p->DoUpdateValue(dp);
}

bool SymbolManager::DoUpdateValue(DState& dp)
{
	DState::LockerSM lock(dp,*this);

	for(size_t i=0;i<m_aSymbol.size();i++)
	{
		CallableSymbol* p=dynamic_cast<CallableSymbol*>(m_aSymbol.get(i).second.get());
		if(!dp.test(p)) continue;

		if(!p->DoUpdateValue(dp))
		{
			return false;
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
