#include "ewc_base/evt/validator_group.h"
#include "ewc_base/app/data_defs.h"

EW_ENTER


ValidatorGroup::ValidatorGroup()
{

}

ValidatorGroup::~ValidatorGroup()
{

}

bool ValidatorGroup::OnWndEvent(IWndParam& cmd,int phase)
{
	for(size_t i=0;i<size();i++)
	{
		(*this)[i]->OnWndEvent(cmd,phase);
	}
	return true;
}

bool ValidatorGroup::WndExecute(IWndParam& cmd)
{
	bool flag=true;
	for(size_t i=0;i<size();i++)
	{
		Validator* v=(*this)[i].get();
		if(!v->WndExecute(cmd))
		{
			return false;
		}
	}
	return true;
}



ValidatorGroupEx::ValidatorGroupEx()
{
	pageid=-1;
}

bool ValidatorGroupEx::WndExecute(IWndParam& cmd)
{
	if(pageid<0||cmd.action==IDefs::ACTION_WINDOW_INIT)
	{
		return basetype::WndExecute(cmd);
	}
		
	return (*this)[pageid]->WndExecute(cmd);

}


EW_LEAVE
