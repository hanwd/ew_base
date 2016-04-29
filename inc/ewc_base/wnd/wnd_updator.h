#ifndef __H_EW_UI_WINUPDATOR__
#define __H_EW_UI_WINUPDATOR__

#include "ewc_base/config.h"
#include "ewc_base/evt/evt_base.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE WndUpdator
{
public:
	WndUpdator(WndManager& app_);

	void lock();
	void unlock();

	void gp_add(const String& s);
	void gp_add(const String& s,int);
	void mu_set(const String& s);
	void tb_set(const String& s);
	void sb_set(const String& s);

	void update();

	operator bool();

	static WndUpdator& current();

protected:

	WndManager& wm;

	void tb_add(const String& s,int f=0);
	indexer_set<String> m_aUpdate;
	String m_sMenubar;
	String m_sStatbar;

	int m_bUpdate;
	int m_nLocked;

	IUpdParam upd;

};

EW_LEAVE
#endif
