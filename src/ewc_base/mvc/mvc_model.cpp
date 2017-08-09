#include "ewc_base/mvc/mvc_model.h"
#include "ewc_base/mvc/mvc_view.h"
#include "ewc_base/wnd/wnd_manager.h"
#include "ewc_base/wnd/wnd_updator.h"

EW_ENTER

MvcView* MvcModel::CreateView()
{
	return NULL;
}

bool MvcModel::DoLoad(const String&)
{
	return false;
}

bool MvcModel::DoSave(const String&)
{
	return false;
}

void MvcModel::OnDocumentUpdated()
{
	WndManager::current().wup.gp_add("CmdProc");
	OnRefreshView();
}

void MvcModel::OnRefreshView()
{
	for (auto it = m_aViews.begin(); it != m_aViews.end(); ++it)
	{
		(*it)->Refresh();
	}
}

void MvcModel::ActivateModel()
{

}

void MvcModel::AttachView(MvcView* v)
{
	m_aViews.insert(v);
}

void MvcModel::DetachView(MvcView* v)
{
	m_aViews.erase(v);
}



class CmdProcStkModel : public CmdProcStk
{
public:
	MvcModelStk& Target;
	CmdProcStkModel(MvcModelStk& tar) :Target(tar){}

	virtual CmdProcLockerObject* CreateLockerObject()
	{
		return Target.CreateLockerObject();
	}

	virtual bool DoLoad(const String& fp)
	{
		return Target.DoLoad(fp);
	}

	virtual bool DoSave(const String& fp)
	{
		return Target.DoSave(fp);
	}

	bool add_cmd(DataPtrT<CmdBase> c)
	{

		DataPtrT<CmdProcLockerObject> locker(CreateLockerObject());
		if (!c->Redo())
		{
			return false;
		}

		if (!locker->test())
		{
			c->Undo();
			return false;
		}

		aCommands.resize(nIndex);
		aCommands.push_back(c);
		++nIndex;

		return true;
	}
};

MvcModelStk::MvcModelStk()
{
	m_refData.reset(new CmdProcStkModel(*this));
	SetData(m_refData.get());

	fn = m_refData->fn;


}

void MvcModelInfo::OnInit()
{

}




EW_LEAVE

