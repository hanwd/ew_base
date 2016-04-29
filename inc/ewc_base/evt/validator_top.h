
#ifndef __H_EW_UI_VALIDATOR_TOP__
#define __H_EW_UI_VALIDATOR_TOP__

#include "ewc_base/config.h"
#include "ewc_base/evt/validator_group.h"


EW_ENTER
	
class DLLIMPEXP_EWC_BASE ValidatorTop : public ValidatorGroup
{
public:

	
	IWindowPtr pWindow;
	ValidatorTop(IWindowPtr w);
	~ValidatorTop();

	virtual bool DoSetLabel(const String& v);
	virtual bool DoGetLabel(String& v);

	bool Show(bool f);
	int ShowModal();
	void EndModal(int h);	

	bool IsShown();
	void Close();

	virtual bool mu_set(const String&){return false;}

	virtual void OnChildWindow(IWindowPtr w,int a);

	void SetModel(WndModel* pwm);
	WndModel* GetModel();

protected:
	DataPtrT<WndModel> m_pModel;

};

EW_LEAVE
#endif
