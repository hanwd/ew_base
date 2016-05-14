#include "ewc_base/mvc/mvc_model.h"

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

}

void MvcModel::OnRefreshView()
{

}

void MvcModel::ActivateModel()
{

}

void MvcModel::AttachView(MvcView* v)
{
	m_aSetViews.insert(v);
}

void MvcModel::DetachView(MvcView* v)
{
	m_aSetViews.erase(v);
}


void MvcModelInfo::OnInit()
{

}

EW_LEAVE

