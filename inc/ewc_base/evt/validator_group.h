
#ifndef __H_EW_UI_VALIDATOR_GROUP__
#define __H_EW_UI_VALIDATOR_GROUP__

#include "ewc_base/config.h"
#include "ewc_base/evt/validator.h"
#include "ewa_base/math/math_def.h"

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


class DLLIMPEXP_EWC_BASE ValidatorGroupBook : public ValidatorGroupEx
{
public:
	typedef ValidatorGroupEx basetype;

	ValidatorGroupBook();

	void IAddPage(wxWindow *w, const WndProperty& p);

	bool IGetBestSize(int& x, int& y);
	void ISetMinSize(int x, int y);
	void IEnableDynamicSize(bool bflag);
	void ISelPage(wxWindow* w);

	enum
	{
		FLAG_PAGE_ONLY = 1 << 0,
		FLAG_DYNAMIC_SIZE_X = 1 << 1,
		FLAG_DYNAMIC_SIZE_Y = 1 << 2,
		FLAG_FITINSIDE = 1 << 3,
		FLAG_FITLAYOUT = 1 << 4,
	};

	BitFlags flags;
	vec2i m_v2MinSize;
	LitePtrT<wxWindow> m_pSelectedPage;
};



EW_LEAVE

#endif
