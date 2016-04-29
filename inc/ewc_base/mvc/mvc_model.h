#ifndef __H_EW_UI_DOCMODEL__
#define __H_EW_UI_DOCMODEL__

#include "ewc_base/app/cmdproc.h"
#include "ewc_base/app/data_defs.h"

EW_ENTER


class DLLIMPEXP_EWC_BASE MvcModelInfo : public ObjectInfo
{
public:

	typedef ObjectInfo basetype;

	enum
	{
		INFO_OPENWITH=1<<0,
		INFO_NEWMODEL=1<<1,
	};

	MvcModelInfo(const String& name=""):basetype(name)
	{
		flags.clr(INFO_OPENWITH|INFO_NEWMODEL);
	}

	MvcModelInfo& OpenWith(bool f)
	{
		flags.set(INFO_OPENWITH,f);
		return *this;
	}

	bool OpenWith() const
	{
		return flags.get(INFO_OPENWITH);
	}

	MvcModelInfo& NewModel(bool f)
	{
		flags.set(INFO_NEWMODEL,f);
		return *this;
	}

	bool NewModel() const
	{
		return flags.get(INFO_NEWMODEL);
	}


	virtual void OnInit();

	BitFlags flags;
	String m_sExtentions;


};

class DLLIMPEXP_EWC_BASE MvcModel : public CmdProcHolderT<CmdProc>
{
public:

	virtual MvcView* CreateView();

	virtual bool DoLoad(const String& file);
	virtual bool DoSave(const String& file);

	virtual void OnDocumentUpdated();
	virtual void OnRefreshView();


	virtual void AttachView(MvcView* v);
	virtual void DetachView(MvcView* v);

	virtual void ActivateModel();

	IFileNameHolder fn;

protected:
	bst_set<MvcView*> m_aSetViews;
};

template<typename V,typename B=MvcModel>
class DLLIMPEXP_EWC_BASE MvcModelT : public B
{
public:
	virtual MvcView* CreateView()
	{
		return new V(*this);
	}
};

EW_LEAVE

#endif
