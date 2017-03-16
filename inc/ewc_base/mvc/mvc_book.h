#ifndef __H_EW_UI_DOCBOOK__
#define __H_EW_UI_DOCBOOK__

#include "ewc_base/app/cmdproc.h"

class wxAuiNotebook;

EW_ENTER

class DLLIMPEXP_EWC_BASE MvcBook : public NonCopyable
{
public:

	friend class MvcBookImpl;
	friend class MvcView;

	static MvcBook& current();

	MvcBook(WndManager& w);

	bool IsActivated(const String& fn);
	bool Activate(const String& fn);

	bool AttachView(MvcView* v);
	bool DetachView(MvcView* v);

	bool OpenModel(MvcModel* m);

	bool Close(int d);
	bool CloseAll();

	void SaveAll();

	bool AnyDirty();

	void UpdateTitle();

	wxAuiNotebook* CreateBook(wxWindow* p);

	MvcView* GetActiveView();

	operator EvtListener*(){return m_pViewListener.get();}

	size_t GetPageCount();

protected:

	WndManager& wm;

	class ViewInfo
	{
	public:
		String title;
		String title_n;
		int index;
		bool dirty;
		TimePoint timestamp;

		ViewInfo()
		{
			index = -1;
			timestamp = Clock::now();
		}
	};

	typedef bst_map<MvcView*, ViewInfo> view_map;
	view_map m_aOpenedViews;
	DataPtrT<EvtListener> m_pViewListener;
	LitePtrT<MvcBookImpl> m_pBook;

};

EW_LEAVE

#endif
