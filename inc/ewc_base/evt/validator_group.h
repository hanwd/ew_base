
#ifndef __H_EW_UI_VALIDATOR_GROUP__
#define __H_EW_UI_VALIDATOR_GROUP__

#include "ewc_base/config.h"
#include "ewc_base/evt/validator.h"


EW_ENTER


class DLLIMPEXP_EWC_BASE ValidatorGroup : public ObjectGroupT<Validator,Validator>
{
public:
	typedef ObjectGroupT<Validator,Validator> basetype;

	ValidatorGroup();
	~ValidatorGroup();

	virtual bool WndExecute(IWndParam& cmd);
	bool OnWndEvent(IWndParam&,int);
};



class DLLIMPEXP_EWC_BASE ValidatorGroupEx : public ValidatorGroup
{
public:
	typedef ValidatorGroup basetype;

	ValidatorGroupEx();
	virtual bool WndExecute(IWndParam& cmd);

	int32_t pageid;
};


EW_LEAVE

#endif
